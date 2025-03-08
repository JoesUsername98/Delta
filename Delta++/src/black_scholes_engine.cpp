#include <math.h>

#include "black_scholes_engine.h"

namespace DPP
{
    double BlackScholesEngine::cumDensity(double z) const
    {
        double p = 0.3275911;
        double a1 = 0.254829592;
        double a2 = -0.284496736;
        double a3 = 1.421413741;
        double a4 = -1.453152027;
        double a5 = 1.061405429;

        int sign;
        if (z < 0.0)
            sign = -1;
        else
            sign = 1;

        double x = abs(z) / sqrt(2.0);
        double t = 1.0 / (1.0 + p * x);
        double erf = 1.0 - (((((a5 * t + a4) * t) + a3)
            * t + a2) * t + a1) * t * exp(-x * x);
        return 0.5 * (1.0 + sign * erf);
    }

    double BlackScholesEngine::probDensity( double z ) const
    {
        const double inv_sqrt_2pi = 0.3989422804014337; // 1 / sqrt(2 * pi)
        return inv_sqrt_2pi * exp( -0.5 * z * z );
    }

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
        return cumDensity( getD1() ) * m_mkt.m_underlyingPrice  - cumDensity( getD2() ) * m_trd.m_strike * exp( -m_mkt.m_interestRate * m_trd.m_maturity );
    }

    double BlackScholesEngine::putPrice() const
    {
        return cumDensity( -getD2() ) * m_trd.m_strike * exp( -m_mkt.m_interestRate * m_trd.m_maturity ) - cumDensity( -getD1() ) * m_mkt.m_underlyingPrice ;
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
            m_results.emplace( calc.m_calc, cumDensity( getD1() ) );
            break;
        case OptionPayoffType::Put:
            m_results.emplace( calc.m_calc, cumDensity( getD1() ) - 1. );
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
            m_results.emplace( calc.m_calc, m_trd.m_strike * m_trd.m_maturity * exp( -m_mkt.m_interestRate * m_trd.m_maturity ) * cumDensity( getD2() ) );
            break;
        case OptionPayoffType::Put:
            m_results.emplace( calc.m_calc, -m_trd.m_strike * m_trd.m_maturity * exp( -m_mkt.m_interestRate * m_trd.m_maturity ) * cumDensity( -getD2() ) );
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
            m_results.emplace( calc.m_calc, m_mkt.m_underlyingPrice * probDensity( getD1() ) * sqrt( m_trd.m_maturity ) );
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
            m_results.emplace( calc.m_calc, probDensity( getD1() ) / ( m_mkt.m_underlyingPrice * m_mkt.m_vol * sqrt( m_trd.m_maturity ) ) );
            break;
        default:
            m_errors.emplace( calc.m_calc, "Only Call and Put are supported PayoffTypes" );
        }
    }
}