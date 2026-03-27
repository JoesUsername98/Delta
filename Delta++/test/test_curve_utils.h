#pragma once

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include <Delta++Market/yield_curve.h>

namespace DPPTest
{
    inline DPP::YieldCurve makeFlatCurve(double flatZeroRate)
    {
        const double ratePct = (std::exp(flatZeroRate) - 1.0) * 100.0;
        std::vector<DPP::RateQuote> quotes = {
            {.tenor = 1.0, .rate = ratePct},
            {.tenor = 30.0, .rate = ratePct},
        };
        auto yc = DPP::YieldCurve::build(quotes);
        EXPECT_TRUE(yc.has_value()) << yc.error();
        return yc.value();
    }
}

