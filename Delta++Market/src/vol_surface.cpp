#include "Delta++Market/vol_surface.h"
#include <algorithm>
#include <set>
#include <cmath>

// TODO: THIS IS BUT A PLACE HOLDER!
namespace DPP
{
    // Helper: parse "YYYY-MM-DD" expiry string to fractional years from today
    static double expiryToYears(const std::string& /*expiry*/)
    {
        // Placeholder: in production, compute (expiry - today) / 365.25
        return 0.0;
    }

    std::expected<VolSurface, std::string>
    VolSurface::build(const std::vector<OptionQuote>& quotes)
    {
        if (quotes.empty())
            return std::unexpected("No option quotes provided");

        // Collect unique sorted expiries and strikes
        std::set<double> expirySet, strikeSet;
        for (const auto& q : quotes)
        {
            if (q.impliedVol.has_value())
            {
                expirySet.insert(expiryToYears(q.expiry));
                strikeSet.insert(q.strike);
            }
        }

        if (expirySet.empty() || strikeSet.empty())
            return std::unexpected("No valid implied vol data in quotes");

        VolSurface surf;
        surf.m_expiries.assign(expirySet.begin(), expirySet.end());
        surf.m_strikes.assign(strikeSet.begin(), strikeSet.end());
        surf.m_numStrikes = surf.m_strikes.size();
        surf.m_vols.resize(surf.m_expiries.size() * surf.m_numStrikes, 0.0);

        // Fill grid (nearest match for now)
        for (const auto& q : quotes)
        {
            if (!q.impliedVol.has_value()) continue;
            double ey = expiryToYears(q.expiry);
            auto eit = std::lower_bound(surf.m_expiries.begin(), surf.m_expiries.end(), ey);
            auto sit = std::lower_bound(surf.m_strikes.begin(), surf.m_strikes.end(), q.strike);
            if (eit != surf.m_expiries.end() && sit != surf.m_strikes.end())
            {
                size_t ei = static_cast<size_t>(eit - surf.m_expiries.begin());
                size_t si = static_cast<size_t>(sit - surf.m_strikes.begin());
                surf.m_vols[ei * surf.m_numStrikes + si] = q.impliedVol.value();
            }
        }

        return surf;
    }

    double VolSurface::vol(double expiryYears, double strike) const
    {
        if (m_expiries.empty() || m_strikes.empty()) return 0.0;

        // Nearest-neighbour lookup (placeholder for proper 2D interpolation)
        auto eit = std::lower_bound(m_expiries.begin(), m_expiries.end(), expiryYears);
        if (eit == m_expiries.end()) --eit;
        auto sit = std::lower_bound(m_strikes.begin(), m_strikes.end(), strike);
        if (sit == m_strikes.end()) --sit;

        size_t ei = static_cast<size_t>(eit - m_expiries.begin());
        size_t si = static_cast<size_t>(sit - m_strikes.begin());
        return m_vols[ei * m_numStrikes + si];
    }
}
