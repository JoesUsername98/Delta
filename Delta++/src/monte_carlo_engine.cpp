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

    CalculationResult MonteCarloEngine::calcPV( const CalcData& calc )
    {
        const auto dt =  m_trd.m_maturity / static_cast<double>( calc.m_steps );
		std::vector<double> sims = m_scheme->simPaths(m_mkt, calc, dt);

        const double pv = m_exercise->price(m_trd, m_mkt, calc, sims, dt, *m_payoff);
        return pv;
    }

    CalculationResult MonteCarloEngine::calcDelta( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

        std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected{ aggErr };

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        MonteCarloEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();
        
		aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
		if ( !aggErr.empty() ) 
			return std::unexpected{ aggErr };

        const double pv_up = up_calc.m_results.at( Calculation::PV ).value();
        const double pv_down = down_calc.m_results.at( Calculation::PV ).value();
        return pv_up - pv_down;
    }

    CalculationResult MonteCarloEngine::calcRho( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpInterestRate( 0.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

		std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected{ aggErr };

        MarketData bump_down = m_mkt.bumpInterestRate (-0.005);
        MonteCarloEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();

        aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected{ aggErr };

        const double pv_up = up_calc.m_results.at( Calculation::PV ).value();
        const double pv_down = down_calc.m_results.at( Calculation::PV ).value();
        return 100. * ( pv_up - pv_down );
    }

    CalculationResult MonteCarloEngine::calcVega( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpVol( 0.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

		std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() )
            return std::unexpected{ aggErr };

        MarketData bump_down = m_mkt.bumpVol( -0.005 );
        MonteCarloEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();

        aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
        if (!aggErr.empty())
            return std::unexpected{ aggErr };

        const double pv_up = up_calc.m_results.at( Calculation::PV ).value();
        const double pv_down = down_calc.m_results.at( Calculation::PV ).value();
        return ( pv_up - pv_down) * 100. ;
    }

    CalculationResult MonteCarloEngine::calcGamma( const CalcData& calc )
    {
        CalcData delta_only = calc;
        delta_only.m_calc = Calculation::Delta;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, delta_only );
        up_calc.run();

		std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() )
            return std::unexpected{ aggErr };

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        MonteCarloEngine down_calc ( bump_down, m_trd, delta_only );
        down_calc.run();

        aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
        if ( !aggErr.empty() )
            return std::unexpected{ aggErr };

        const double delta_up = up_calc.m_results.at( Calculation::Delta ).value();
        const double delta_down = down_calc.m_results.at( Calculation::Delta ).value();
        return delta_up - delta_down;
    }
}