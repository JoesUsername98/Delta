#include "Delta++/binomial_engine.h"

using namespace std::string_literals;
namespace DPP
{
    void BinomialEngine::calcPV( const CalcData& calc )
    {
        TriMatrixBuilder build_result = 
        TriMatrixBuilder::create( calc.m_steps, m_trd.m_maturity / calc.m_steps )
        .withUnderlyingValueAndVolatility( m_mkt.m_underlyingPrice, m_mkt.m_vol )
        .withInterestRate( m_mkt.m_interestRate )
        .withPayoff( m_trd.m_optionPayoffType, m_trd.m_strike )
        .withRiskNuetralProb()
        .withPremium( m_trd.m_optionExerciseType );
        
        if( build_result.m_hasError )
            m_errors.emplace( calc.m_calc , build_result.getErrorMsg() );
        else
            m_results.emplace( calc.m_calc , build_result.build().getMatrix()[ 0 ].m_data.m_optionValue );
    }

    void BinomialEngine::calcDelta( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        BinomialEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

        if( !up_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : up_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        BinomialEngine down_calc ( bump_down, m_trd, pv_only );
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

    void BinomialEngine::calcRho( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpInterestRate( 0.005 );
        BinomialEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

        if( !up_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : up_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bump_down = m_mkt.bumpInterestRate (-0.005);
        BinomialEngine down_calc ( bump_down, m_trd, pv_only );
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

    void BinomialEngine::calcVega( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpVol( 0.005 );
        BinomialEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

        if( !up_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : up_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bump_down = m_mkt.bumpVol( -0.005 );
        BinomialEngine down_calc ( bump_down, m_trd, pv_only );
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

    void BinomialEngine::calcGamma( const CalcData& calc )
    {
        CalcData delta_only = calc;
        delta_only.m_calc = Calculation::Delta;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        BinomialEngine up_calc ( bump_up, m_trd, delta_only );
        up_calc.run();

        if( !up_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : up_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        BinomialEngine down_calc ( bump_down, m_trd, delta_only );
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