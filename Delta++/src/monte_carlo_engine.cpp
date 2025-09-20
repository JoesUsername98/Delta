#include <random>
#include <algorithm>
#include <numeric>
#include <ranges>

#include "monte_carlo_engine.h"


using namespace std::string_literals;
namespace DPP
{
    void MonteCarloEngine::updatePrice( double& S, double dW, double dt ) const
    {
        // exact scheme
        // S = S * std::exp( ( m_mkt.m_interestRate - 0.5 * m_mkt.m_vol * m_mkt.m_vol ) * dt + m_mkt.m_vol * dW );
        // Euler scheme
        // S = S * ( 1 + m_mkt.m_interestRate * dt + m_mkt.m_vol * dW );
        // Milstein scheme
        S = S * (1 + m_mkt.m_interestRate * dt + m_mkt.m_vol * dW + 0.5 * m_mkt.m_vol * m_mkt.m_vol * (dW * dW - dt));
    }
    void MonteCarloEngine::calcPV( const CalcData& calc )
    {
        const auto dt =  m_trd.m_maturity / calc.m_steps;
        std::seed_seq seed{ 42 };
        std::mt19937_64 rng{ seed };
        std::normal_distribution<double> norm{ 0., 1. };
        std::vector<double> normal_numbers( calc.m_sims *calc.m_steps, 0 );
        std::generate( normal_numbers.begin(), normal_numbers.end(), [&]() { return norm( rng ); });

        std::vector<double> sims( calc.m_sims *calc.m_steps, 0 );
        for( size_t simIdx = 0; simIdx <calc.m_sims; ++simIdx )
        {
            double S = m_mkt.m_underlyingPrice;
            for( size_t stepIdx = 0; stepIdx <calc.m_steps; ++stepIdx )
            {
                const auto idx = simIdx *calc.m_steps + stepIdx;
                const double dW = std::sqrt( dt ) * normal_numbers[ idx ];
				updatePrice(S, dW, dt);
                sims[idx] =  S;
            }
        }

        const auto& maturityPrice = sims | std::views::drop( calc.m_steps - 1 ) |  std::views::stride( calc.m_steps );
        std::vector<double> payoff( maturityPrice.begin(), maturityPrice.end() );
        
        switch( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call: 
            std::for_each( payoff.begin(), payoff.end(),[ strike = m_trd.m_strike ]( double &val ) { val = std::max(val - strike, 0.0); } );
			break;
        case OptionPayoffType::Put:
            std::for_each( payoff.begin(), payoff.end(),[ strike = m_trd.m_strike ]( double &val ) { val = std::max(strike - val, 0.0); } );
			break;
        default:
            m_errors.emplace( calc.m_calc, "Unsupported option payoff type" );
			return;
		}

        const double sum = std::accumulate( payoff.begin(), payoff.end(), 0.0 );
        const double avgPayoff = sum / static_cast<double>( calc.m_sims );
        const double pv = std::exp( -m_mkt.m_interestRate * m_trd.m_maturity ) * avgPayoff;
        
        m_results.emplace( calc.m_calc , pv );
    }

    void MonteCarloEngine::calcDelta( const CalcData& calc )
    {
        CalcData pvOnly = calc;
        pvOnly.m_calc = Calculation::PV;

        MarketData bumpUp = m_mkt.bumpUnderlying( 1.005 );
        MonteCarloEngine upCalc ( bumpUp, m_trd, pvOnly );
        upCalc.run();

        if( !upCalc.m_errors.empty() ) 
        {
            for( const auto& [ upCalc , err ] : upCalc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bumpDown = m_mkt.bumpUnderlying( .995 );
        MonteCarloEngine downCalc ( bumpDown, m_trd, pvOnly );
        downCalc.run();
        if( !downCalc.m_errors.empty() ) 
        {
            for( const auto& [ downCalc , err ] : downCalc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        const double pvUp = upCalc.m_results.at( Calculation::PV );
        const double pvDown = downCalc.m_results.at( Calculation::PV );
        m_results.emplace( calc.m_calc, pvUp - pvDown );
    }

    void MonteCarloEngine::calcRho( const CalcData& calc )
    {
        CalcData pvOnly = calc;
        pvOnly.m_calc = Calculation::PV;

        MarketData bumpUp = m_mkt.bumpInterestRate( 0.005 );
        MonteCarloEngine upCalc ( bumpUp, m_trd, pvOnly );
        upCalc.run();

        if( !upCalc.m_errors.empty() ) 
        {
            for( const auto& [ upCalc , err ] : upCalc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bumpDown = m_mkt.bumpInterestRate (-0.005);
        MonteCarloEngine downCalc ( bumpDown, m_trd, pvOnly );
        downCalc.run();
        if( !downCalc.m_errors.empty() ) 
        {
            for( const auto& [ downCalc , err ] : downCalc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        const double pvUp = upCalc.m_results.at( Calculation::PV );
        const double pvDown = downCalc.m_results.at( Calculation::PV );
        m_results.emplace( calc.m_calc, 100. * ( pvUp - pvDown ) );
    }

    void MonteCarloEngine::calcVega( const CalcData& calc )
    {
        CalcData pvOnly = calc;
        pvOnly.m_calc = Calculation::PV;

        MarketData bumpUp = m_mkt.bumpVol( 0.005 );
        MonteCarloEngine upCalc ( bumpUp, m_trd, pvOnly );
        upCalc.run();

        if( !upCalc.m_errors.empty() ) 
        {
            for( const auto& [ upCalc , err ] : upCalc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bumpDown = m_mkt.bumpVol( -0.005 );
        MonteCarloEngine downCalc ( bumpDown, m_trd, pvOnly );
        downCalc.run();
        if( !downCalc.m_errors.empty() ) 
        {
            for( const auto& [ downCalc , err ] : downCalc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        const double pvUp = upCalc.m_results.at( Calculation::PV );
        const double pvDown = downCalc.m_results.at( Calculation::PV );
        m_results.emplace( calc.m_calc, (pvUp - pvDown)*100 );
    }

    void MonteCarloEngine::calcGamma( const CalcData& calc )
    {
        CalcData deltaOnly = calc;
        deltaOnly.m_calc = Calculation::Delta;

        MarketData bumpUp = m_mkt.bumpUnderlying( 1.005 );
        MonteCarloEngine upCalc ( bumpUp, m_trd, deltaOnly );
        upCalc.run();

        if( !upCalc.m_errors.empty() ) 
        {
            for( const auto& [ upCalc , err ] : upCalc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bumpDown = m_mkt.bumpUnderlying( .995 );
        MonteCarloEngine downCalc ( bumpDown, m_trd, deltaOnly );
        downCalc.run();
        if( !downCalc.m_errors.empty() ) 
        {
            for( const auto& [ downCalc , err ] : downCalc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        const double deltaUp = upCalc.m_results.at( Calculation::Delta );
        const double deltaDown = downCalc.m_results.at( Calculation::Delta );
        m_results.emplace( calc.m_calc, deltaUp - deltaDown );
    }
}