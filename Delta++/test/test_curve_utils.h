#pragma once

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include <Delta++/market.h>
#include <Delta++Market/yield_curve.h>

namespace DPPTest
{
    inline DPP::YieldCurve makeFlatCurve(double flatZeroRate)
    {
        // Use expm1 for better numerical stability when converting z -> quoted annual rate.
        const double ratePct = std::expm1(flatZeroRate) * 100.0;
        std::vector<DPP::RateQuote> quotes = {
            {.tenor = 1.0, .rate = ratePct},
            {.tenor = 30.0, .rate = ratePct},
        };
        auto yc = DPP::YieldCurve::build(quotes);
        EXPECT_TRUE(yc.has_value()) << yc.error();
        return yc.value();
    }

    /// Flat Black–Scholes σ via `AHInterpolator` stub (`MarketData::withFlatConstantVol`).
    inline DPP::MarketData makeFlatMarket(const double spot, const double sigma, const DPP::YieldCurve& yieldCurve)
    {
        auto mkt = DPP::MarketData::withFlatConstantVol(spot, sigma, yieldCurve);
        EXPECT_TRUE(mkt.has_value()) << mkt.error();
        return std::move(mkt.value());
    }

    inline DPP::MarketData makeFlatMarket(const double spot, const double sigma, const double flatZeroRate)
    {
        return makeFlatMarket(spot, sigma, makeFlatCurve(flatZeroRate));
    }
}

