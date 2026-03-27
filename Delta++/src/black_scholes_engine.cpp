#include <Delta++Math/distributions.h>

#include "Delta++/black_scholes_engine.h"

namespace DPP
{
    double BlackScholesEngine::getD1() const
    {
        const double r = m_mkt.zeroRate(m_trd.m_maturity);
        return ( log( m_mkt.m_underlyingPrice / m_trd.m_strike ) + ( r + m_mkt.m_vol * m_mkt.m_vol * .5 ) * m_trd.m_maturity ) /
            ( m_mkt.m_vol * sqrt( m_trd.m_maturity ) );
    }

    double BlackScholesEngine::getD2() const
    {
        return getD1() - ( m_mkt.m_vol * sqrt( m_trd.m_maturity ) );
    }

    double BlackScholesEngine::callPrice() const
    {
        const double df = m_mkt.discount(m_trd.m_maturity);
        return DPPMath::cumDensity( getD1() ) * m_mkt.m_underlyingPrice  -  DPPMath::cumDensity( getD2() ) * m_trd.m_strike * df;
    }

    double BlackScholesEngine::putPrice() const
    {
        const double df = m_mkt.discount(m_trd.m_maturity);
        return  DPPMath::cumDensity( -getD2() ) * m_trd.m_strike * df -  DPPMath::cumDensity( -getD1() ) * m_mkt.m_underlyingPrice ;
    }

    CalculationResult BlackScholesEngine::calcPV(const CalcData& calc) const
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

    CalculationResult BlackScholesEngine::calcDelta(const CalcData& calc) const
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

    CalculationResult BlackScholesEngine::calcRho(const CalcData& calc) const
    {
        // Key-rate rho: one entry per curve knot (fallback: single parallel-style entry when no curve).
        constexpr double bump = 0.005;

        if (!m_mkt.m_yieldCurve.has_value())
        {
            const auto up = m_mkt.bumpInterestRate(bump);
            const auto down = m_mkt.bumpInterestRate(-bump);
            BlackScholesEngine up_calc(up, m_trd, calc);
            BlackScholesEngine down_calc(down, m_trd, calc);
            const auto pv_up = scalarOrError(up_calc.calcPV(calc));
            const auto pv_down = scalarOrError(down_calc.calcPV(calc));
            if (!pv_up.has_value())
                return std::unexpected(pv_up.error());
            if (!pv_down.has_value())
                return std::unexpected(pv_down.error());

            CurveRho rho;
            rho.push_back({m_trd.m_maturity, 100. * (pv_up.value() - pv_down.value())});
            return rho;
        }

        const auto& tenors = m_mkt.m_yieldCurve->tenors();
        const double T = m_trd.m_maturity;
        CurveRho rho;
        rho.reserve(tenors.size());
        for (size_t i = 0; i < tenors.size(); ++i)
        {
            if (tenors[i] > T)
                break;
            const auto up = m_mkt.bumpYieldCurveKeyRate(i, bump);
            const auto down = m_mkt.bumpYieldCurveKeyRate(i, -bump);
            BlackScholesEngine up_calc(up, m_trd, calc);
            BlackScholesEngine down_calc(down, m_trd, calc);
            const auto pv_up = scalarOrError(up_calc.calcPV(calc));
            const auto pv_down = scalarOrError(down_calc.calcPV(calc));
            if (!pv_up.has_value())
                return std::unexpected(pv_up.error());
            if (!pv_down.has_value())
                return std::unexpected(pv_down.error());
            rho.push_back({tenors[i], 100. * (pv_up.value() - pv_down.value())});
        }
        return rho;
    }

    CalculationResult BlackScholesEngine::calcRhoParallel(const CalcData& calc) const
    {
        constexpr double bump = 0.005;
        const auto up = m_mkt.bumpYieldCurveParallel(bump);
        const auto down = m_mkt.bumpYieldCurveParallel(-bump);
        BlackScholesEngine up_calc(up, m_trd, calc);
        BlackScholesEngine down_calc(down, m_trd, calc);
        const auto pv_up = scalarOrError(up_calc.calcPV(calc));
        const auto pv_down = scalarOrError(down_calc.calcPV(calc));
        if (!pv_up.has_value())
            return std::unexpected(pv_up.error());
        if (!pv_down.has_value())
            return std::unexpected(pv_down.error());
        return 100. * (pv_up.value() - pv_down.value());
    }

    CalculationResult BlackScholesEngine::calcVega(const CalcData& calc) const
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

    CalculationResult BlackScholesEngine::calcGamma(const CalcData& calc) const
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