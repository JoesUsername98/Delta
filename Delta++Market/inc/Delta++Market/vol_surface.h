#pragma once

#include <vector>
#include <string>
#include <expected>
#include "quotes.h"

namespace DPP
{
    class VolSurface
    {
    public:
        static std::expected<VolSurface, std::string>
        build(const std::vector<OptionQuote>& quotes);

        // Query implied vol by expiry (in years) and strike
        double vol(double expiryYears, double strike) const;

        const std::vector<double>& expiries() const { return m_expiries; }
        const std::vector<double>& strikes() const  { return m_strikes; }

    private:
        VolSurface() = default;

        std::vector<double> m_expiries;
        std::vector<double> m_strikes;
        // Flat grid of implied vols [expiry_idx * num_strikes + strike_idx]
        std::vector<double> m_vols;
        size_t m_numStrikes = 0;
    };
}
