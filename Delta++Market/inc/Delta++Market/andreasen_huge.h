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

/// Dupire forward FD helpers used by `bootstrapAndreasenHuge` and gap-filled `LocalVolSurface` queries.
namespace DPP::AhDupireFd
{
    constexpr double kSigmaMin = 0.01;
    constexpr double kSigmaMax = 1.5;
    constexpr double kVarMin = kSigmaMin * kSigmaMin;
    constexpr double kVarMax = kSigmaMax * kSigmaMax;
    constexpr double kDupireEps = 1e-14;
    constexpr int kMinGridPoints = 32;

    void laplacianTimes(const std::vector<double>& K, const std::vector<double>& c, std::vector<double>& out);

    bool implicitStep(const std::vector<double>& K,
                      const std::vector<double>& cOld,
                      const std::vector<double>& cBdry,
                      double dT,
                      const std::vector<double>& w,
                      std::vector<double>& cNew);

    std::vector<double> buildLogStrikeGrid(const std::vector<double>& expiries,
                                           const std::vector<std::vector<double>>& strikes);
}

