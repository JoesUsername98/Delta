#include "Delta++/black_scholes_engine.h"

#include <Delta++BlackScholes/black_scholes.h>
#include <Delta++Math/distributions.h>

#include <algorithm>

namespace DPP
{
    namespace
    {
        double qAtMaturity(const MarketData& mkt, const TradeData& trd)
        {
            return mkt.dividendYield(trd.m_maturity);
        }

        double sigmaAtStrike(const MarketData& mkt, const TradeData& trd)
        {
            return mkt.localVolAt(trd.m_maturity, trd.m_strike);
        }
    }

    double BlackScholesEngine::getD1() const
    {
        const double r = m_mkt.zeroRate(m_trd.m_maturity);
        const double q = qAtMaturity(m_mkt, m_trd);
        return BlackScholes::d1(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, q,
                               sigmaAtStrike(m_mkt, m_trd));
    }

    double BlackScholesEngine::getD2() const
    {
        const double r = m_mkt.zeroRate(m_trd.m_maturity);
        const double q = qAtMaturity(m_mkt, m_trd);
        return BlackScholes::d2(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, q,
                               sigmaAtStrike(m_mkt, m_trd));
    }

    double BlackScholesEngine::callPrice() const
    {
        const double S = m_mkt.m_underlyingPrice;
        const double K = m_trd.m_strike;
        const double T = m_trd.m_maturity;
        const double r = m_mkt.zeroRate(T);
        const double q = qAtMaturity(m_mkt, m_trd);
        const double sigma = sigmaAtStrike(m_mkt, m_trd);
        const double df = m_mkt.discount(T);
        if (T <= 0.0)
            return std::max(0.0, S - K);
        if (sigma <= 0.0)
        {
            const double fwd = S * std::exp((r - q) * T);
            return df * std::max(0.0, fwd - K);
        }
        const double d1v = BlackScholes::d1(S, K, T, r, q, sigma);
        const double d2v = BlackScholes::d2(S, K, T, r, q, sigma);
        return S * std::exp(-q * T) * DPPMath::cumDensity(d1v) - K * df * DPPMath::cumDensity(d2v);
    }

    double BlackScholesEngine::putPrice() const
    {
        const double S = m_mkt.m_underlyingPrice;
        const double K = m_trd.m_strike;
        const double T = m_trd.m_maturity;
        const double r = m_mkt.zeroRate(T);
        const double q = qAtMaturity(m_mkt, m_trd);
        const double sigma = sigmaAtStrike(m_mkt, m_trd);
        const double df = m_mkt.discount(T);
        if (T <= 0.0)
            return std::max(0.0, K - S);
        if (sigma <= 0.0)
        {
            const double fwd = S * std::exp((r - q) * T);
            return df * std::max(0.0, K - fwd);
        }
        const double d1v = BlackScholes::d1(S, K, T, r, q, sigma);
        const double d2v = BlackScholes::d2(S, K, T, r, q, sigma);
        return K * df * DPPMath::cumDensity(-d2v) - S * std::exp(-q * T) * DPPMath::cumDensity(-d1v);
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
        const double q = qAtMaturity(m_mkt, m_trd);
        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
            return BlackScholes::callDelta(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, q,
                                           sigmaAtStrike(m_mkt, m_trd));
        case OptionPayoffType::Put:
            return BlackScholes::putDelta(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, q,
                                           sigmaAtStrike(m_mkt, m_trd));
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
        if (!m_mkt.isEssentiallyConstantVolSurface())
            return std::unexpected(
                "Analytical vega requires constant σ(T,K) (e.g. MarketData::withFlatConstantVol); general local vol "
                "surfaces are not supported.");

        const double r = m_mkt.zeroRate(m_trd.m_maturity);
        const double q = qAtMaturity(m_mkt, m_trd);
        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
        case OptionPayoffType::Put:
            return BlackScholes::vega(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, q,
                                      sigmaAtStrike(m_mkt, m_trd));
        default:
            return std::unexpected("Only Call and Put are supported PayoffTypes" );
        }
    }

    CalculationResult BlackScholesEngine::calcGamma(const CalcData& calc) const
    {
        if (!m_mkt.isEssentiallyConstantVolSurface())
            return std::unexpected(
                "Analytical gamma requires constant σ(T,K) (e.g. MarketData::withFlatConstantVol); general local vol "
                "surfaces are not supported.");

        const double r = m_mkt.zeroRate(m_trd.m_maturity);
        const double q = qAtMaturity(m_mkt, m_trd);
        switch ( m_trd.m_optionPayoffType )
        {
        case OptionPayoffType::Call:
        case OptionPayoffType::Put:
            return BlackScholes::gamma(m_mkt.m_underlyingPrice, m_trd.m_strike, m_trd.m_maturity, r, q,
                                       sigmaAtStrike(m_mkt, m_trd));
        default:
            return std::unexpected("Only Call and Put are supported PayoffTypes" );
        }
    }
}