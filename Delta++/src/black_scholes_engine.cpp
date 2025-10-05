#include <Delta++Math/distributions.h>

#include "Delta++/black_scholes_engine.h"

namespace DPP
{
    double BlackScholesEngine::getD1() const
    {
        return ( log( m_mkt.m_underlyingPrice / m_trd.m_strike ) + ( m_mkt.m_interestRate + m_mkt.m_vol * m_mkt.m_vol * .5 ) * m_trd.m_maturity ) /
            ( m_mkt.m_vol * sqrt( m_trd.m_maturity ) );
    }

    double BlackScholesEngine::getD2() const
    {
        return getD1() - ( m_mkt.m_vol * sqrt( m_trd.m_maturity ) );
    }

    double BlackScholesEngine::callPrice() const
    {
        return DPPMath::cumDensity( getD1() ) * m_mkt.m_underlyingPrice  -  DPPMath::cumDensity( getD2() ) * m_trd.m_strike * exp( -m_mkt.m_interestRate * m_trd.m_maturity );
    }

    double BlackScholesEngine::putPrice() const
    {
        return  DPPMath::cumDensity( -getD2() ) * m_trd.m_strike * exp( -m_mkt.m_interestRate * m_trd.m_maturity ) -  DPPMath::cumDensity( -getD1() ) * m_mkt.m_underlyingPrice ;
    }

    void BlackScholesEngine::calcPV(const CalcData& calc)
    {
        if (m_trd.m_optionExerciseType != OptionExerciseType::European)
        {
            m_errors.emplace(calc.m_calc, "BlackScholes can only handle European Exercise");
            return;
        }

        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
            m_results.emplace( calc.m_calc, callPrice() );
            break;
        case OptionPayoffType::Put:
            m_results.emplace( calc.m_calc, putPrice() );
            break;
        default:
            m_errors.emplace( calc.m_calc, "Only Call and Put are supported PayoffTypes");
        }
    }

    void BlackScholesEngine::calcDelta(const CalcData& calc)
    {
        if ( m_trd.m_optionExerciseType != OptionExerciseType::European )
        {
            m_errors.emplace(calc.m_calc, "BlackScholes can only handle European Exercise");
            return;
        }

        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
            m_results.emplace( calc.m_calc,  DPPMath::cumDensity( getD1() ) );
            break;
        case OptionPayoffType::Put:
            m_results.emplace( calc.m_calc,  DPPMath::cumDensity( getD1() ) - 1. );
            break;
        default:
            m_errors.emplace( calc.m_calc, "Only Call and Put are supported PayoffTypes" );
        }
    }

    void BlackScholesEngine::calcRho(const CalcData& calc)
    {
        if ( m_trd.m_optionExerciseType != OptionExerciseType::European )
        {
            m_errors.emplace( calc.m_calc, "BlackScholes can only handle European Exercise" );
            return;
        }

        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
            m_results.emplace( calc.m_calc, m_trd.m_strike * m_trd.m_maturity * exp( -m_mkt.m_interestRate * m_trd.m_maturity ) *  DPPMath::cumDensity( getD2() ) );
            break;
        case OptionPayoffType::Put:
            m_results.emplace( calc.m_calc, -m_trd.m_strike * m_trd.m_maturity * exp( -m_mkt.m_interestRate * m_trd.m_maturity ) *  DPPMath::cumDensity( -getD2() ) );
            break;
        default:
            m_errors.emplace( calc.m_calc, "Only Call and Put are supported PayoffTypes" );
        }
    }

    void BlackScholesEngine::calcVega(const CalcData& calc)
    {
        if ( m_trd.m_optionExerciseType != OptionExerciseType::European )
        {
            m_errors.emplace( calc.m_calc, "BlackScholes can only handle European Exercise" );
            return;
        }
        
        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
        case OptionPayoffType::Put:
            m_results.emplace( calc.m_calc, m_mkt.m_underlyingPrice *  DPPMath::probDensity( getD1() ) * sqrt( m_trd.m_maturity ) );
            break;
        default:
            m_errors.emplace( calc.m_calc, "Only Call and Put are supported PayoffTypes" );
        }
    }

    void BlackScholesEngine::calcGamma(const CalcData& calc)
    {
        if ( m_trd.m_optionExerciseType != OptionExerciseType::European )
        {
            m_errors.emplace( calc.m_calc, "BlackScholes can only handle European Exercise" );
            return;
        }

        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
        case OptionPayoffType::Put:
            m_results.emplace( calc.m_calc,  DPPMath::probDensity( getD1() ) / ( m_mkt.m_underlyingPrice * m_mkt.m_vol * sqrt( m_trd.m_maturity ) ) );
            break;
        default:
            m_errors.emplace( calc.m_calc, "Only Call and Put are supported PayoffTypes" );
        }
    }
}