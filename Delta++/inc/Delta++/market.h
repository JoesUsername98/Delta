#pragma once

#include <cstddef>
#include <optional>

#include <Delta++Market/andreasen_huge_interpolator.h>
#include <Delta++Market/dividend_yield_curve.h>
#include <Delta++Market/yield_curve.h>

namespace DPP
{
    struct MarketData
    {
        double m_vol{};
        double m_underlyingPrice{};
        DPP::YieldCurve m_yieldCurve;
        /// Default flat zero dividend yield (matches legacy pricers before q was wired).
        DPP::DividendYieldCurve m_dividendYieldCurve = DividendYieldCurve::flat(0.0);
        /// When set, path engines use `AHInterpolator::localVol(T,K)` (Monte Carlo: at path `(t,S)`).
        std::optional<DPP::AHInterpolator> m_localVolSurface{};

        MarketData copy() const
        {
            return MarketData{.m_vol = m_vol,
                             .m_underlyingPrice = m_underlyingPrice,
                             .m_yieldCurve = m_yieldCurve,
                             .m_dividendYieldCurve = m_dividendYieldCurve,
                             .m_localVolSurface = m_localVolSurface};
        }

        double zeroRate(double t) const { return m_yieldCurve.zeroRate(t); }

        double discount(double t) const { return m_yieldCurve.discount(t); }

        double dividendYield(double t) const { return m_dividendYieldCurve.q(t); }

        /// Dupire local vol when `m_localVolSurface` is set; otherwise constant `m_vol`.
        double localVolAt(double T, double K) const
        {
            if (m_localVolSurface.has_value())
                return m_localVolSurface->localVol(T, K);
            return m_vol;
        }

        bool hasLocalVolSurface() const { return m_localVolSurface.has_value(); }

        MarketData bumpVol(double bump) const
        {
            MarketData bumpee = copy();
            bumpee.m_vol += bump;
            return bumpee;
        }

        MarketData bumpUnderlying(double bump) const
        {
            MarketData bumpee = copy();
            bumpee.m_underlyingPrice *= bump;
            return bumpee;
        }

        MarketData bumpYieldCurveParallel(double bump) const
        {
            MarketData bumpee = copy();
            bumpee.m_yieldCurve = bumpee.m_yieldCurve.parallelShift(bump);
            return bumpee;
        }

        MarketData bumpYieldCurveKeyRate(std::size_t knotIdx, double bump) const
        {
            MarketData bumpee = copy();
            bumpee.m_yieldCurve = bumpee.m_yieldCurve.keyRateBump(knotIdx, bump);
            return bumpee;
        }
    };
}
