#pragma once

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

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
}

