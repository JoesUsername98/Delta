#include <random>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <future>
#include <thread>

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
        S = S * ( 1 + m_mkt.m_interestRate * dt + m_mkt.m_vol * dW + 0.5 * m_mkt.m_vol * m_mkt.m_vol * ( dW * dW - dt ) );
    }
    void MonteCarloEngine::calcPV( const CalcData& calc )
    {
        const auto dt =  m_trd.m_maturity / static_cast<double>( calc.m_steps );
        const auto sqrt_dt =  std::sqrt( dt );
        std::vector<double> sims( calc.m_sims * calc.m_steps, m_mkt.m_underlyingPrice );
        auto worker = [&](size_t start_sim, size_t end_sim, size_t thread_id) {
            static thread_local std::seed_seq seq{ 42 + thread_id };
            static thread_local std::mt19937_64 rng{ seq };
            static thread_local std::normal_distribution<double> norm{ 0., 1. };

            for ( size_t sim_idx = start_sim; sim_idx < end_sim; ++sim_idx ) 
            {
                double S = m_mkt.m_underlyingPrice;
                for ( size_t step_index = 1; step_index < calc.m_steps; ++step_index) 
                {
                    const auto idx = sim_idx * calc.m_steps + step_index;
                    const double dW = sqrt_dt * norm( rng );
                    updatePrice( S, dW, dt );
                    sims[ idx ] = S;
                }
            }
        };

        const size_t n_threads = std::thread::hardware_concurrency();
        const size_t sims_per_thread = calc.m_sims / n_threads;
        std::vector<std::thread> threads;
        threads.reserve( n_threads );
        size_t start = 0;
        for ( size_t t = 0; t < n_threads; ++t )
        {
            size_t end = ( t == n_threads - 1 ) ? calc.m_sims : start + sims_per_thread;
            threads.emplace_back( std::thread( worker, start, end, t ) );
            start = std::min( end, calc.m_sims );
        }
		for( auto& t : threads) { t.join(); } ;

        const auto maturity_prices = sims
            | std::views::drop( calc.m_steps - 1 )
            | std::views::stride( calc.m_steps );

        double sum = 0.0;
        switch (m_trd.m_optionPayoffType)
        {
        case OptionPayoffType::Call:
            sum = std::ranges::fold_left(
                maturity_prices
                | std::views::transform( [strike = m_trd.m_strike](double val) { return std::max( val - strike, 0.0 ); }), 0.0, std::plus{} );
            break;

        case OptionPayoffType::Put:
            sum = std::ranges::fold_left(
                maturity_prices
                | std::views::transform( [strike = m_trd.m_strike](double val) { return std::max( strike - val, 0.0 ); }), 0.0, std::plus{} );
            break;

        default:
            m_errors.emplace( calc.m_calc, "Unsupported option payoff type" );
            return;
        }

        const double mean_payoff = sum / std::ranges::distance( maturity_prices );
        const double pv = std::exp( -m_mkt.m_interestRate * m_trd.m_maturity ) * mean_payoff;

        m_results.emplace( calc.m_calc, pv );
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