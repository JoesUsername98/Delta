#pragma once

#include <Delta++Market/yield_curve.h>

#include <string>
#include <vector>

namespace DPP
{
    struct AHInput
    {
        double spot{};
        YieldCurve curve;
        std::vector<double> expiries;                // T (years), strictly increasing
        std::vector<double> dividendYields;          // q(T) per expiry (same size as expiries) or empty => 0
        std::vector<std::vector<double>> strikes;    // per-expiry K grid, sorted ascending
        std::vector<std::vector<double>> callPrices; // per-expiry call mids matching strikes
    };

    /// Forward-measure call prices / local variance on the FD strike grid from Andreasen–Huge bootstrap.
    /// Used for gap filling (paper step 2) between market expiries. Single source of truth for w; σ is derived.
    struct AhForwardSurfaceData
    {
        YieldCurve curve;
        std::vector<double> kGrid;
        /// Undiscounted forward call c = C/DF on kGrid per pillar expiry.
        std::vector<std::vector<double>> cFwd;
        /// Piecewise local variance w on kGrid per pillar (same layout as bootstrap).
        std::vector<std::vector<double>> localVariance;
    };
}

/// Dupire forward FD helpers shared by bootstrap and `AHInterpolator` gap queries.
namespace DPP::AhDupireFd
{
    constexpr double kSigmaMin = 0.01;
    constexpr double kSigmaMax = 1.5;
    constexpr double kVarMin = kSigmaMin * kSigmaMin;
    constexpr double kVarMax = kSigmaMax * kSigmaMax;
    constexpr double kDupireEps = 1e-14;
    constexpr int kMinGridPoints = 32;

    void dupireXOperatorTimes(const std::vector<double>& K, const std::vector<double>& c, std::vector<double>& out);

    bool implicitStep(const std::vector<double>& K,
                      const std::vector<double>& cOld,
                      const std::vector<double>& cBdry,
                      double dT,
                      const std::vector<double>& w,
                      std::vector<double>& cNew);

    std::vector<double> buildLogStrikeGrid(const std::vector<double>& expiries,
                                           const std::vector<std::vector<double>>& strikes);
}
