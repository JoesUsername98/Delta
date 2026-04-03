#pragma once

#include <Delta++Market/yield_curve.h>

#include <expected>
#include <optional>
#include <string>
#include <vector>

namespace DPP
{
    /// Forward-measure call prices / local variance on the FD strike grid from Andreasen–Huge bootstrap.
    /// Used for gap filling (paper step 2) between market expiries.
    struct AhForwardSurfaceData
    {
        YieldCurve curve;
        std::vector<double> kGrid;
        /// Undiscounted forward call c = C/DF on kGrid per pillar expiry.
        std::vector<std::vector<double>> cFwd;
        /// Piecewise local variance w on kGrid per pillar (same layout as bootstrap).
        std::vector<std::vector<double>> localVariance;
    };

    // Irregular knot surface: per-expiry 1D interpolation in strike,
    // then linear interpolation in time between adjacent expiries (unless AH forward data is present).
    class LocalVolSurface
    {
    public:
        double localVol(double T, double K) const;
        double callPrice(double T, double K) const;

        const std::vector<double>& expiries() const { return m_expiries; }
        const std::vector<std::vector<double>>& strikes() const { return m_strikes; }
        const std::vector<std::vector<double>>& callPrices() const { return m_callPrices; }
        const std::vector<std::vector<double>>& localVols() const { return m_localVols; }

        /// Optional `ahForward` enables Dupire gap filling (paper step 2) for T strictly between pillars.
        static std::expected<LocalVolSurface, std::string>
        build(std::vector<double> expiries,
              std::vector<std::vector<double>> strikes,
              std::vector<std::vector<double>> callPrices,
              std::vector<std::vector<double>> localVols,
              std::optional<AhForwardSurfaceData> ahForward = std::nullopt);

    private:
        LocalVolSurface() = default;

        double callPriceLegacyBetween(double T, double K, size_t i0, size_t i1) const;
        double callPriceGapBetween(double T, double K, size_t i0, size_t i1) const;
        double localVolLegacyBetween(double T, double K, size_t i0, size_t i1) const;
        double localVolGapBetween(double K, size_t i0) const;

        std::vector<double> m_expiries;
        std::vector<std::vector<double>> m_strikes;
        std::vector<std::vector<double>> m_callPrices;
        std::vector<std::vector<double>> m_localVols;
        std::optional<AhForwardSurfaceData> m_ahForward;
    };
}
