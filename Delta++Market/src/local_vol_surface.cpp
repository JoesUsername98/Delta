#include <Delta++Market/local_vol_surface.h>
#include <Delta++Market/andreasen_huge.h>

#include <Delta++Solver/interpolation.h>

#include <algorithm>
#include <cmath>
#include <vector>

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
                           std::vector<std::vector<double>> localVols,
                           std::optional<AhForwardSurfaceData> ahForward)
    {
        if (expiries.size() < 2)
            return std::unexpected("Need >=2 expiries");
        if (strikes.size() != expiries.size() || callPrices.size() != expiries.size() || localVols.size() != expiries.size())
            return std::unexpected("Surface arrays must match expiries size");

        const size_t nT = expiries.size();
        for (size_t i = 0; i < nT; ++i)
        {
            if (strikes[i].size() < 3)
                return std::unexpected("Need >=3 strikes per expiry for stable local vol");
            if (callPrices[i].size() != strikes[i].size() || localVols[i].size() != strikes[i].size())
                return std::unexpected("Per-expiry strike/value sizes must match");
        }

        if (ahForward.has_value())
        {
            const auto& ah = *ahForward;
            if (ah.kGrid.size() < 3)
                return std::unexpected("AH kGrid too small");
            if (ah.cFwd.size() != nT || ah.localVariance.size() != nT)
                return std::unexpected("AH forward arrays must match expiries size");
            const size_t nK = ah.kGrid.size();
            for (size_t i = 0; i < nT; ++i)
            {
                if (ah.cFwd[i].size() != nK || ah.localVariance[i].size() != nK)
                    return std::unexpected("AH forward row length must match kGrid");
            }
        }

        LocalVolSurface s;
        s.m_expiries = std::move(expiries);
        s.m_strikes = std::move(strikes);
        s.m_callPrices = std::move(callPrices);
        s.m_localVols = std::move(localVols);
        s.m_ahForward = std::move(ahForward);
        return s;
    }

    double LocalVolSurface::callPriceLegacyBetween(double T, double K, size_t i0, size_t i1) const
    {
        const double c0 = interpStrikeSpline(m_strikes[i0], m_callPrices[i0], K);
        const double c1 = interpStrikeSpline(m_strikes[i1], m_callPrices[i1], K);
        const double t0 = m_expiries[i0];
        const double t1 = m_expiries[i1];
        const double w = std::clamp((T - t0) / (t1 - t0), 0.0, 1.0);
        return lerp(c0, c1, w);
    }

    double LocalVolSurface::callPriceGapBetween(double T, double K, size_t i0, size_t i1) const
    {
        const auto& ah = *m_ahForward;
        const double t0 = m_expiries[i0];
        const double t1 = m_expiries[i1];
        const double dTotal = T - t0;
        if (!(dTotal > 0.0))
            return interpStrikeSpline(m_strikes[i0], m_callPrices[i0], K);

        const auto& Kg = ah.kGrid;
        const size_t nK = Kg.size();
        const auto& cLeft = ah.cFwd[i0];
        const auto& cRightPillar = ah.cFwd[i1];
        const auto& w0 = ah.localVariance[i0];

        const size_t nSteps = std::max(size_t{1}, size_t(std::ceil(dTotal / std::max(1e-12, 0.25 * (t1 - t0)))));
        const double dt = dTotal / static_cast<double>(nSteps);

        std::vector<double> cOld = cLeft;
        std::vector<double> cNew(nK);
        std::vector<double> cBdry(nK);

        for (size_t k = 0; k < nSteps; ++k)
        {
            const double tau = t0 + static_cast<double>(k + 1) * dt;
            const double alpha = (tau - t0) / (t1 - t0);
            cBdry[0] = (1.0 - alpha) * cLeft[0] + alpha * cRightPillar[0];
            cBdry[nK - 1] = (1.0 - alpha) * cLeft[nK - 1] + alpha * cRightPillar[nK - 1];
            if (!AhDupireFd::implicitStep(Kg, cOld, cBdry, dt, w0, cNew))
                return callPriceLegacyBetween(T, K, i0, i1);
            cOld.swap(cNew);
        }

        CubicSplineInterpolator li(Kg, cOld);
        const double cFwd = li(K);
        const double df = ah.curve.discount(T);
        return cFwd * df;
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
        const double t0 = m_expiries[i0];
        const double t1 = m_expiries[i1];
        constexpr double eps = 1e-11;
        if (T <= t0 + eps * (1.0 + std::abs(t0)))
            return interpStrikeSpline(m_strikes[i0], m_callPrices[i0], K);
        if (T >= t1 - eps * (1.0 + std::abs(t1)))
            return interpStrikeSpline(m_strikes[i1], m_callPrices[i1], K);

        if (m_ahForward.has_value())
            return callPriceGapBetween(T, K, i0, i1);
        return callPriceLegacyBetween(T, K, i0, i1);
    }

    double LocalVolSurface::localVolLegacyBetween(double T, double K, size_t i0, size_t i1) const
    {
        const double s0 = interpStrikeSpline(m_strikes[i0], m_localVols[i0], K);
        const double s1 = interpStrikeSpline(m_strikes[i1], m_localVols[i1], K);
        const double t0 = m_expiries[i0];
        const double t1 = m_expiries[i1];
        const double w = std::clamp((T - t0) / (t1 - t0), 0.0, 1.0);
        return lerp(s0, s1, w);
    }

    double LocalVolSurface::localVolGapBetween(double K, size_t i0) const
    {
        const auto& ah = *m_ahForward;
        const auto& Kg = ah.kGrid;
        std::vector<double> sigK(Kg.size());
        for (size_t j = 0; j < Kg.size(); ++j)
            sigK[j] = std::clamp(std::sqrt(std::max(ah.localVariance[i0][j], 0.0)),
                                 AhDupireFd::kSigmaMin,
                                 AhDupireFd::kSigmaMax);
        CubicSplineInterpolator s(Kg, sigK);
        return s(K);
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
        const double t0 = m_expiries[i0];
        const double t1 = m_expiries[i1];
        constexpr double eps = 1e-11;
        if (T <= t0 + eps * (1.0 + std::abs(t0)))
            return interpStrikeSpline(m_strikes[i0], m_localVols[i0], K);
        if (T >= t1 - eps * (1.0 + std::abs(t1)))
            return interpStrikeSpline(m_strikes[i1], m_localVols[i1], K);

        if (m_ahForward.has_value())
            return localVolGapBetween(K, i0);
        return localVolLegacyBetween(T, K, i0, i1);
    }
}
