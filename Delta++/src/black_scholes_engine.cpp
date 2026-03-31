#include "Delta++/black_scholes_engine.h"

#include <Delta++BlackScholes/black_scholes.h>

namespace DPP
{
    double BlackScholesEngine::getD1() const
    {
        const double r = m_mkt.zeroRate(m_trd.m_maturity);
        return BlackScholes::d1(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, 0.0, m_mkt.m_vol);
    }

    double BlackScholesEngine::getD2() const
    {
        const double r = m_mkt.zeroRate(m_trd.m_maturity);
        return BlackScholes::d2(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, 0.0, m_mkt.m_vol);
    }

    double BlackScholesEngine::callPrice() const
    {
        const double r = m_mkt.zeroRate(m_trd.m_maturity);
        const double df = m_mkt.discount(m_trd.m_maturity);
        return BlackScholes::callPriceDf(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, df, m_mkt.m_vol);
    }

    double BlackScholesEngine::putPrice() const
    {
        const double r = m_mkt.zeroRate(m_trd.m_maturity);
        const double df = m_mkt.discount(m_trd.m_maturity);
        return BlackScholes::putPriceDf(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, df, m_mkt.m_vol);
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

        const double r = m_mkt.zeroRate(m_trd.m_maturity);
        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
            return BlackScholes::callDelta(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, 0.0, m_mkt.m_vol);
        case OptionPayoffType::Put:
            return BlackScholes::putDelta(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, 0.0, m_mkt.m_vol);
        default:
            return std::unexpected("Only Call and Put are supported PayoffTypes");
        }
    }

    CalculationResult BlackScholesEngine::calcRho(const CalcData& calc) const
    {
        // Key-rate rho: one entry per curve knot (fallback: single parallel-style entry when no curve).
        constexpr double bump = 0.005;

        const auto& tenors = m_mkt.m_yieldCurve.tenors();
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
        const double r = m_mkt.zeroRate(m_trd.m_maturity);
        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
        case OptionPayoffType::Put:
            return BlackScholes::vega(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, 0.0, m_mkt.m_vol);
        default:
            return std::unexpected("Only Call and Put are supported PayoffTypes" );
        }
    }

    CalculationResult BlackScholesEngine::calcGamma(const CalcData& calc) const
    {
        const double r = m_mkt.zeroRate(m_trd.m_maturity);
        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
        case OptionPayoffType::Put:
            return BlackScholes::gamma(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, 0.0, m_mkt.m_vol);
        default:
            return std::unexpected("Only Call and Put are supported PayoffTypes" );
        }
    }
}