#include "Delta++/binomial_engine.h"

using namespace std::string_literals;
namespace DPP
{
    void BinomialEngine::calcPV( const CalcData& calc )
    {
        TriMatrixBuilder buildResult = 
        TriMatrixBuilder::create( calc.m_steps, m_trd.m_maturity / calc.m_steps )
        .withUnderlyingValueAndVolatility( m_mkt.m_underlyingPrice, m_mkt.m_vol )
        .withInterestRate( m_mkt.m_interestRate )
        .withPayoff( m_trd.m_optionPayoffType, m_trd.m_strike )
        .withRiskNuetralProb()
        .withPremium( m_trd.m_optionExerciseType );
        
        if( buildResult.m_hasError )
            m_errors.emplace( calc.m_calc , buildResult.getErrorMsg() );
        else
            m_results.emplace( calc.m_calc , buildResult.build().getMatrix()[ 0 ].m_data.m_optionValue );
    }

    void BinomialEngine::calcDelta( const CalcData& calc )
    {
        CalcData pvOnly = calc;
        pvOnly.m_calc = Calculation::PV;

        MarketData bumpUp = m_mkt.bumpUnderlying( 1.005 );
        BinomialEngine upCalc ( bumpUp, m_trd, pvOnly );
        upCalc.run();

        if( !upCalc.m_errors.empty() ) 
        {
            for( const auto& [ upCalc , err ] : upCalc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bumpDown = m_mkt.bumpUnderlying( .995 );
        BinomialEngine downCalc ( bumpDown, m_trd, pvOnly );
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

    void BinomialEngine::calcRho( const CalcData& calc )
    {
        CalcData pvOnly = calc;
        pvOnly.m_calc = Calculation::PV;

        MarketData bumpUp = m_mkt.bumpInterestRate( 0.005 );
        BinomialEngine upCalc ( bumpUp, m_trd, pvOnly );
        upCalc.run();

        if( !upCalc.m_errors.empty() ) 
        {
            for( const auto& [ upCalc , err ] : upCalc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bumpDown = m_mkt.bumpInterestRate (-0.005);
        BinomialEngine downCalc ( bumpDown, m_trd, pvOnly );
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

    void BinomialEngine::calcVega( const CalcData& calc )
    {
        CalcData pvOnly = calc;
        pvOnly.m_calc = Calculation::PV;

        MarketData bumpUp = m_mkt.bumpVol( 0.005 );
        BinomialEngine upCalc ( bumpUp, m_trd, pvOnly );
        upCalc.run();

        if( !upCalc.m_errors.empty() ) 
        {
            for( const auto& [ upCalc , err ] : upCalc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bumpDown = m_mkt.bumpVol( -0.005 );
        BinomialEngine downCalc ( bumpDown, m_trd, pvOnly );
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

    void BinomialEngine::calcGamma( const CalcData& calc )
    {
        CalcData deltaOnly = calc;
        deltaOnly.m_calc = Calculation::Delta;

        MarketData bumpUp = m_mkt.bumpUnderlying( 1.005 );
        BinomialEngine upCalc ( bumpUp, m_trd, deltaOnly );
        upCalc.run();

        if( !upCalc.m_errors.empty() ) 
        {
            for( const auto& [ upCalc , err ] : upCalc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bumpDown = m_mkt.bumpUnderlying( .995 );
        BinomialEngine downCalc ( bumpDown, m_trd, deltaOnly );
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