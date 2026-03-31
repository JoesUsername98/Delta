#pragma once

#include <Delta++Market/local_vol_surface.h>
#include <Delta++Market/yield_curve.h>

#include <expected>
#include <string>
#include <vector>

namespace DPP
{
    struct AHInput
    {
        double spot{};
        YieldCurve curve;
        std::vector<double> expiries;                    // T (years), strictly increasing
        std::vector<double> dividendYields;              // q(T) per expiry (same size as expiries) or empty => 0
        std::vector<std::vector<double>> strikes;        // per-expiry K grid, sorted ascending
        std::vector<std::vector<double>> callPrices;     // per-expiry call mids matching strikes
    };

    std::expected<LocalVolSurface, std::string> bootstrapAndreasenHuge(const AHInput& in);
}

