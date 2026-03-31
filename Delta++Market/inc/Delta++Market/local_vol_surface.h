#pragma once

#include <Delta++Solver/interpolation.h>

#include <vector>
#include <expected>
#include <string>

namespace DPP
{
    // Irregular knot surface: per-expiry 1D interpolation in strike,
    // then linear interpolation in time between adjacent expiries.
    class LocalVolSurface
    {
    public:
        double localVol(double T, double K) const;
        double callPrice(double T, double K) const;

        const std::vector<double>& expiries() const { return m_expiries; }
        const std::vector<std::vector<double>>& strikes() const { return m_strikes; }
        const std::vector<std::vector<double>>& callPrices() const { return m_callPrices; }
        const std::vector<std::vector<double>>& localVols() const { return m_localVols; }

        static std::expected<LocalVolSurface, std::string>
        build(std::vector<double> expiries,
              std::vector<std::vector<double>> strikes,
              std::vector<std::vector<double>> callPrices,
              std::vector<std::vector<double>> localVols);

    private:
        LocalVolSurface() = default;

        std::vector<double> m_expiries;
        std::vector<std::vector<double>> m_strikes;
        std::vector<std::vector<double>> m_callPrices;
        std::vector<std::vector<double>> m_localVols;
    };
}

