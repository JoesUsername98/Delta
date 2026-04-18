#pragma once

#include <cmath>
#include <cstddef>
#include <expected>
#include <optional>
#include <string>

#include <Delta++Market/andreasen_huge_interpolator.h>
#include <Delta++Market/dividend_yield_curve.h>
#include <Delta++Market/yield_curve.h>

namespace DPP
{
    struct MarketData
    {
        double m_underlyingPrice{};
        DPP::YieldCurve m_yieldCurve;
        /// Default flat zero dividend yield (matches legacy pricers before q was wired).
        DPP::DividendYieldCurve m_dividendYieldCurve = DividendYieldCurve::flat(0.0);
        /// Volatility for path engines and `localVolAt` (flat Black use `withFlatConstantVol` / `buildFlatConstant`).
        std::optional<DPP::AHInterpolator> m_localVolSurface{};

        /// Constant σ everywhere (AH stub). Prefer this over a separate scalar vol field.
        [[nodiscard]] static std::expected<MarketData, std::string>
        withFlatConstantVol(double underlyingPrice, double sigma, const YieldCurve& yieldCurve,
                            DividendYieldCurve dividendCurve = DividendYieldCurve::flat(0.0));

        MarketData copy() const
        {
            return MarketData{.m_underlyingPrice = m_underlyingPrice,
                             .m_yieldCurve = m_yieldCurve,
                             .m_dividendYieldCurve = m_dividendYieldCurve,
                             .m_localVolSurface = m_localVolSurface};
        }

        double zeroRate(double t) const { return m_yieldCurve.zeroRate(t); }

        double discount(double t) const { return m_yieldCurve.discount(t); }

        double dividendYield(double t) const { return m_dividendYieldCurve.q(t); }

        /// Dupire local vol from `m_localVolSurface` (throws `std::bad_optional_access` if unset).
        double localVolAt(double T, double K) const { return m_localVolSurface.value().localVol(T, K); }

        bool hasLocalVolSurface() const { return m_localVolSurface.has_value(); }

        /// True if σ(T,K) is (numerically) constant — allows analytical Black–Scholes greeks and `bumpVol` bumps.
        bool isEssentiallyConstantVolSurface(double absTol = 1e-8) const;

        /// Additive σ bump for flat constant surfaces only; rebuilds the AH flat stub. Misleading if the smile is not flat.
        MarketData bumpVol(double bump) const;

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

    inline std::expected<MarketData, std::string> MarketData::withFlatConstantVol(const double underlyingPrice,
                                                                                  const double sigma,
                                                                                  const YieldCurve& yieldCurve,
                                                                                  DividendYieldCurve dividendCurve)
    {
        auto surf = AHInterpolator::buildFlatConstant(sigma, yieldCurve);
        if (!surf)
            return std::unexpected(surf.error());
        return MarketData{.m_underlyingPrice = underlyingPrice,
                          .m_yieldCurve = yieldCurve,
                          .m_dividendYieldCurve = std::move(dividendCurve),
                          .m_localVolSurface = std::move(*surf)};
    }

    inline bool MarketData::isEssentiallyConstantVolSurface(const double absTol) const
    {
        if (!hasLocalVolSurface())
            return false;
        const auto& lv = *m_localVolSurface;
        const double S = m_underlyingPrice;
        const double v0 = lv.localVol(0.2, S);
        return std::abs(lv.localVol(0.2, 0.5 * S) - v0) < absTol && std::abs(lv.localVol(0.8, 1.5 * S) - v0) < absTol;
    }

    inline MarketData MarketData::bumpVol(const double bump) const
    {
        MarketData out = copy();
        if (!out.m_localVolSurface.has_value())
            return out;
        const double sigma0 = out.m_localVolSurface->localVol(0.25, m_underlyingPrice);
        auto surf = AHInterpolator::buildFlatConstant(sigma0 + bump, m_yieldCurve);
        if (surf)
            out.m_localVolSurface = std::move(*surf);
        return out;
    }
}
