#include <Delta++Market/local_vol_surface.h>

#include <algorithm>
#include <cmath>

namespace
{
    double lerp(double a, double b, double t) { return a + (b - a) * t; }

    double interpStrikeSpline(const std::vector<double>& Ks, const std::vector<double>& Vs, double K)
    {
        if (Ks.size() < 2)
            return Vs.empty() ? 0.0 : Vs.front();
        DPP::CubicSplineInterpolator s(Ks, Vs);
        return s(K);
    }
}

namespace DPP
{
    std::expected<LocalVolSurface, std::string>
    LocalVolSurface::build(std::vector<double> expiries,
                           std::vector<std::vector<double>> strikes,
                           std::vector<std::vector<double>> callPrices,
                           std::vector<std::vector<double>> localVols)
    {
        if (expiries.size() < 2)
            return std::unexpected("Need >=2 expiries");
        if (strikes.size() != expiries.size() || callPrices.size() != expiries.size() || localVols.size() != expiries.size())
            return std::unexpected("Surface arrays must match expiries size");

        for (size_t i = 0; i < expiries.size(); ++i)
        {
            if (strikes[i].size() < 3)
                return std::unexpected("Need >=3 strikes per expiry for stable local vol");
            if (callPrices[i].size() != strikes[i].size() || localVols[i].size() != strikes[i].size())
                return std::unexpected("Per-expiry strike/value sizes must match");
        }

        LocalVolSurface s;
        s.m_expiries = std::move(expiries);
        s.m_strikes = std::move(strikes);
        s.m_callPrices = std::move(callPrices);
        s.m_localVols = std::move(localVols);
        return s;
    }

    double LocalVolSurface::callPrice(const double T, const double K) const
    {
        if (m_expiries.empty())
            return 0.0;
        if (T <= m_expiries.front())
            return interpStrikeSpline(m_strikes.front(), m_callPrices.front(), K);
        if (T >= m_expiries.back())
            return interpStrikeSpline(m_strikes.back(), m_callPrices.back(), K);

        const auto it = std::upper_bound(m_expiries.begin(), m_expiries.end(), T);
        const size_t i1 = static_cast<size_t>(it - m_expiries.begin());
        const size_t i0 = i1 - 1;

        const double c0 = interpStrikeSpline(m_strikes[i0], m_callPrices[i0], K);
        const double c1 = interpStrikeSpline(m_strikes[i1], m_callPrices[i1], K);
        const double t0 = m_expiries[i0];
        const double t1 = m_expiries[i1];
        const double w = std::clamp((T - t0) / (t1 - t0), 0.0, 1.0);
        return lerp(c0, c1, w);
    }

    double LocalVolSurface::localVol(const double T, const double K) const
    {
        if (m_expiries.empty())
            return 0.0;
        if (T <= m_expiries.front())
            return interpStrikeSpline(m_strikes.front(), m_localVols.front(), K);
        if (T >= m_expiries.back())
            return interpStrikeSpline(m_strikes.back(), m_localVols.back(), K);

        const auto it = std::upper_bound(m_expiries.begin(), m_expiries.end(), T);
        const size_t i1 = static_cast<size_t>(it - m_expiries.begin());
        const size_t i0 = i1 - 1;

        const double s0 = interpStrikeSpline(m_strikes[i0], m_localVols[i0], K);
        const double s1 = interpStrikeSpline(m_strikes[i1], m_localVols[i1], K);
        const double t0 = m_expiries[i0];
        const double t1 = m_expiries[i1];
        const double w = std::clamp((T - t0) / (t1 - t0), 0.0, 1.0);
        return lerp(s0, s1, w);
    }
}

