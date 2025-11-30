#include "Delta++/tri_matrix_builder.h"
#include "Delta++/binomial_engine.h"

using namespace std::string_literals;
namespace DPP
{
    CalculationResult BinomialEngine::calcPV( const CalcData& calc )
    {
        TriMatrixBuilder build_result = 
        TriMatrixBuilder::create( calc.m_steps, m_trd.m_maturity / calc.m_steps )
        .withUnderlyingValueAndVolatility( m_mkt.m_underlyingPrice, m_mkt.m_vol )
        .withInterestRate( m_mkt.m_interestRate )
        .withPayoff( m_trd.m_optionPayoffType, m_trd.m_strike )
        .withRiskNuetralProb()
        .withPremium( m_trd.m_optionExerciseType );
        
        if( build_result.m_hasError )
            return std::unexpected( build_result.getErrorMsg() );
        else
            return build_result.build().getMatrix()[ 0 ].m_data.m_optionValue;
    }

    CalculationResult BinomialEngine::calcDelta( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        BinomialEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

		std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected( aggErr );

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        BinomialEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();

        aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected(aggErr);

        const double pv_up = up_calc.m_results.at( Calculation::PV ).value();
        const double pv_down = down_calc.m_results.at( Calculation::PV ).value();
        return pv_up - pv_down;
    }

    CalculationResult BinomialEngine::calcRho( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpInterestRate( 0.005 );
        BinomialEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

		std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected( aggErr );

        MarketData bump_down = m_mkt.bumpInterestRate (-0.005);
        BinomialEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();

        aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected( aggErr );

        const double pv_up = up_calc.m_results.at( Calculation::PV ).value();
        const double pv_down = down_calc.m_results.at( Calculation::PV ).value();
        return 100. * ( pv_up - pv_down );
    }

    CalculationResult BinomialEngine::calcVega( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpVol( 0.005 );
        BinomialEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

		std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected( aggErr );

        MarketData bump_down = m_mkt.bumpVol( -0.005 );
        BinomialEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();

        aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected( aggErr );

        const double pv_up = up_calc.m_results.at( Calculation::PV ).value();
        const double pv_down = down_calc.m_results.at( Calculation::PV ).value();
        return ( pv_up - pv_down ) * 100;
    }

    CalculationResult BinomialEngine::calcGamma( const CalcData& calc )
    {
        CalcData delta_only = calc;
        delta_only.m_calc = Calculation::Delta;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        BinomialEngine up_calc ( bump_up, m_trd, delta_only );
        up_calc.run();

		std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected( aggErr );

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        BinomialEngine down_calc ( bump_down, m_trd, delta_only );
        down_calc.run();

        aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected( aggErr );

        const double delta_up = up_calc.m_results.at( Calculation::Delta ).value();
        const double delta_down = down_calc.m_results.at( Calculation::Delta ).value();
        return  delta_up - delta_down;
    }
}