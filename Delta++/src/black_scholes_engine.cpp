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

    CalculationResult BlackScholesEngine::calcPV(const CalcData& calc)
    {
        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
            return callPrice();
        case OptionPayoffType::Put:
            return putPrice();
        default:
            return std::unexpected("Only Call and Put are supported PayoffTypes");
        }
    }

    CalculationResult BlackScholesEngine::calcDelta(const CalcData& calc)
    {
        if ( m_trd.m_optionExerciseType != OptionExerciseType::European )
            return std::unexpected("BlackScholes can only handle European Exercise");

        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
            return DPPMath::cumDensity( getD1() );
        case OptionPayoffType::Put:
            return DPPMath::cumDensity( getD1() ) - 1.;
        default:
            return std::unexpected("Only Call and Put are supported PayoffTypes");
        }
    }

    CalculationResult BlackScholesEngine::calcRho(const CalcData& calc)
    {
        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
            return m_trd.m_strike * m_trd.m_maturity * exp( -m_mkt.m_interestRate * m_trd.m_maturity ) *  DPPMath::cumDensity( getD2() );
        case OptionPayoffType::Put:
            return -m_trd.m_strike * m_trd.m_maturity * exp( -m_mkt.m_interestRate * m_trd.m_maturity ) *  DPPMath::cumDensity( -getD2() );
        default:
            return std::unexpected("Only Call and Put are supported PayoffTypes" );
        }
    }

    CalculationResult BlackScholesEngine::calcVega(const CalcData& calc)
    {
        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
        case OptionPayoffType::Put:
            return m_mkt.m_underlyingPrice *  DPPMath::probDensity( getD1() ) * sqrt( m_trd.m_maturity );
        default:
            return std::unexpected("Only Call and Put are supported PayoffTypes" );
        }
    }

    CalculationResult BlackScholesEngine::calcGamma(const CalcData& calc)
    {
        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
        case OptionPayoffType::Put:
            return DPPMath::probDensity( getD1() ) / ( m_mkt.m_underlyingPrice * m_mkt.m_vol * sqrt( m_trd.m_maturity ) );
        default:
            return std::unexpected("Only Call and Put are supported PayoffTypes" );
        }
    }
}