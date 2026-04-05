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

        const std::vector<double>& expiries() const { return m_expiries; }
        const std::vector<std::vector<double>>& strikes() const { return m_strikes; }
        const std::vector<std::vector<double>>& callPrices() const { return m_callPrices; }

        /// σ(K) on each expiry’s quoted strikes, projected from `localVariance` on the FD grid (for UI / export).
        std::vector<std::vector<double>> projectLocalVolsAtQuotedStrikes() const;

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

        std::vector<double> m_expiries;
        std::vector<std::vector<double>> m_strikes;
        std::vector<std::vector<double>> m_callPrices;
        AhForwardSurfaceData m_ahForward;
    };
}
