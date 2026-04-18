#pragma once

#include <Delta++Market/andreasen_huge_common.h>

#include <expected>
#include <string>
#include <vector>

namespace DPP
{
    /// Post-bootstrap evaluator: pillar splines in strike for prices; Dupire continuation between pillars.
    class AHInterpolator
    {
    public:
        double localVol(double T, double K) const;
        double callPrice(double T, double K) const;

        /// Calendar-time routing only (same as `localVol(T,·)`). Hoist once per MC timestep; then use `localVolOnPillar`.
        /// Fails if expiries are empty (cannot route pillars).
        std::expected<size_t, std::string> localVolPillarIndexForTime(double T) const;
        /// σ(K) on a fixed pillar slice (matches `localVol(T,K)` when T maps to `pillarIdx`).
        double localVolOnPillar(size_t pillarIdx, double K) const;

        const std::vector<double>& expiries() const { return m_expiries; }
        const std::vector<std::vector<double>>& strikes() const { return m_strikes; }
        const std::vector<std::vector<double>>& callPrices() const { return m_callPrices; }

        static std::expected<AHInterpolator, std::string>
        build(std::vector<double> expiries,
              std::vector<std::vector<double>> strikes,
              std::vector<std::vector<double>> callPrices,
              AhForwardSurfaceData ahForward);

    private:
        AHInterpolator(std::vector<double> expiries,
                       std::vector<std::vector<double>> strikes,
                       std::vector<std::vector<double>> callPrices,
                       AhForwardSurfaceData ahForward);

        double callPriceGapBetween(double T, double K, size_t i0, size_t i1) const;
        /// σ(K) from pillar slice `localVariance[pillarIdx]` on kGrid (linear in K to match bootstrap projection).
        double localVolFromVarianceSlice(size_t pillarIdx, double K) const;

        // Fast per-pillar cached sigma grids to avoid allocations and repeated conversions from variance
        double interpSigK(size_t pillarIdx, double K) const;

        std::vector<double> m_expiries;
        std::vector<std::vector<double>> m_strikes;
        std::vector<std::vector<double>> m_callPrices;
        AhForwardSurfaceData m_ahForward;
        std::vector<std::vector<double>> m_sigK; // cached sigma per pillar (aligned with m_ahForward.kGrid)
    };
}
