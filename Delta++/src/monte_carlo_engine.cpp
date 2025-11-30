#include <random>

#include "Delta++/monte_carlo_engine.h"

using namespace std::string_literals;
namespace DPP
{
    MonteCarloEngine::MonteCarloEngine(const MarketData& mkt, const TradeData& trd, const CalcData& calc)
		: AbstractEngine(mkt, trd, calc)
    {
        initStrategies();
    }

    MonteCarloEngine::MonteCarloEngine(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc)
        : AbstractEngine(mkt, trd, calc)
    {
        initStrategies();
    }

    void MonteCarloEngine::initStrategies()
    {
        switch (m_calcs.front().m_pathSchemeType)
        {
        case PathSchemeType::Exact:
            m_scheme = std::make_unique<ExactScheme>();
            break;
        case PathSchemeType::Euler:
            m_scheme = std::make_unique<EulerScheme>();
            break;
        case PathSchemeType::Milstein:
            m_scheme = std::make_unique<MilsteinScheme>();
            break;
        }

        switch (m_trd.m_optionPayoffType)
        {
        case OptionPayoffType::Call:
            m_payoff = std::make_unique<MCCallPayoff>(m_trd.m_strike);
            break;
        case OptionPayoffType::Put:
            m_payoff = std::make_unique<MCPutPayoff>(m_trd.m_strike);
            break;
        }

        switch (m_trd.m_optionExerciseType)
        {
        case OptionExerciseType::European:
            m_exercise = std::make_unique<MCEuropeanExercise>();
            break;
        case OptionExerciseType::American:
            m_exercise = std::make_unique<MCAmericanExercise>();
            break;
        
        }
    }

    void MonteCarloEngine::calcPV( const CalcData& calc )
    {
        const auto dt =  m_trd.m_maturity / static_cast<double>( calc.m_steps );
		std::vector<double> sims = m_scheme->simPaths(m_mkt, calc, dt);

        const double pv = m_exercise->price(m_trd, m_mkt, calc, sims, dt, *m_payoff);
        m_results.emplace(calc.m_calc, pv);
    }

    void MonteCarloEngine::calcDelta( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

        if( !up_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : up_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        MonteCarloEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();
        if( !down_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : down_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        const double pv_up = up_calc.m_results.at( Calculation::PV );
        const double pv_down = down_calc.m_results.at( Calculation::PV );
        m_results.emplace( calc.m_calc, pv_up - pv_down );
    }

    void MonteCarloEngine::calcRho( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpInterestRate( 0.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

        if( !up_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : up_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bump_down = m_mkt.bumpInterestRate (-0.005);
        MonteCarloEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();
        if( !down_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : down_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        const double pv_up = up_calc.m_results.at( Calculation::PV );
        const double pv_down = down_calc.m_results.at( Calculation::PV );
        m_results.emplace( calc.m_calc, 100. * ( pv_up - pv_down ) );
    }

    void MonteCarloEngine::calcVega( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpVol( 0.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

        if( !up_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : up_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bump_down = m_mkt.bumpVol( -0.005 );
        MonteCarloEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();
        if( !down_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : down_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        const double pv_up = up_calc.m_results.at( Calculation::PV );
        const double pv_down = down_calc.m_results.at( Calculation::PV );
        m_results.emplace( calc.m_calc, (pv_up - pv_down)*100 );
    }

    void MonteCarloEngine::calcGamma( const CalcData& calc )
    {
        CalcData delta_only = calc;
        delta_only.m_calc = Calculation::Delta;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, delta_only );
        up_calc.run();

        if( !up_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : up_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        MonteCarloEngine down_calc ( bump_down, m_trd, delta_only );
        down_calc.run();
        if( !down_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : down_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        const double delta_up = up_calc.m_results.at( Calculation::Delta );
        const double delta_down = down_calc.m_results.at( Calculation::Delta );
        m_results.emplace( calc.m_calc, delta_up - delta_down );
    }
}