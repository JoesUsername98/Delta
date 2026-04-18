// AHInterpolator: pillar splines for forward calls; Dupire gap fill between expiries.

#include <Delta++Market/andreasen_huge_interpolator.h>

#include <Delta++Solver/interpolation.h>

#include <algorithm>
#include <cmath>
#include <expected>
#include <limits>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace
{
    double sigmaFromVariance(double w)
    {
        return std::clamp(std::sqrt(std::max(w, 0.0)), DPP::AhDupireFd::kSigmaMin, DPP::AhDupireFd::kSigmaMax);
    }

    double interpStrikeSpline(const std::vector<double>& Ks, const std::vector<double>& Vs, double K)
    {
        if (Ks.size() < 2)
            return Vs.empty() ? 0.0 : Vs.front();
        DPP::CubicSplineInterpolator s(Ks, Vs);
        return s(K);
    }

    enum class PillarTimeKind
    {
        Empty,
        BeforeOrAtFirst,
        AfterOrAtLast,
        OnPillar,
        Between
    };

    struct PillarTimeRoute
    {
        PillarTimeKind kind{};
        size_t i0{};
        size_t i1{};
    };

    PillarTimeRoute routePillarTime(double T, const std::vector<double>& expiries)
    {
        PillarTimeRoute r{};
        if (expiries.empty())
        {
            r.kind = PillarTimeKind::Empty;
            return r;
        }
        if (T <= expiries.front())
        {
            r.kind = PillarTimeKind::BeforeOrAtFirst;
            r.i0 = 0;
            return r;
        }
        if (T >= expiries.back())
        {
            r.kind = PillarTimeKind::AfterOrAtLast;
            r.i0 = expiries.size() - 1;
            return r;
        }

        const auto it = std::upper_bound(expiries.begin(), expiries.end(), T);
        const size_t i1 = static_cast<size_t>(it - expiries.begin());
        const size_t i0 = i1 - 1;
        const double t0 = expiries[i0];
        const double t1 = expiries[i1];
        constexpr double eps = 1e-11;
        if (T <= t0 + eps * (1.0 + std::abs(t0)))
        {
            r.kind = PillarTimeKind::OnPillar;
            r.i0 = i0;
            return r;
        }
        if (T >= t1 - eps * (1.0 + std::abs(t1)))
        {
            r.kind = PillarTimeKind::OnPillar;
            r.i0 = i1;
            return r;
        }
        r.kind = PillarTimeKind::Between;
        r.i0 = i0;
        r.i1 = i1;
        return r;
    }

    std::optional<size_t> pillarIndexForLocalVolTime(const PillarTimeRoute& r)
    {
        switch (r.kind)
        {
        case PillarTimeKind::Empty:
            return std::nullopt;
        case PillarTimeKind::BeforeOrAtFirst:
            return size_t{0};
        case PillarTimeKind::AfterOrAtLast:
            return r.i0;
        case PillarTimeKind::OnPillar:
            return r.i0;
        case PillarTimeKind::Between:
            return r.i0;
        }
        return std::nullopt;
    }
}

namespace DPP
{
    AHInterpolator::AHInterpolator(std::vector<double> expiries,
                                 std::vector<std::vector<double>> strikes,
                                 std::vector<std::vector<double>> callPrices,
                                 AhForwardSurfaceData ahForward)
        : m_expiries(std::move(expiries))
        , m_strikes(std::move(strikes))
        , m_callPrices(std::move(callPrices))
        , m_ahForward(std::move(ahForward))
    {
        // Cache per-pillar sigma grids to avoid repeated allocations and conversions during runtime.
        const size_t nT = m_ahForward.localVariance.size();
        const size_t nK = m_ahForward.kGrid.size();
        m_sigK.clear();
        m_sigK.reserve(nT);
        for (const size_t t : std::views::iota(size_t{0}, nT))
        {
            std::vector<double> row(nK);
            std::ranges::transform(
                std::views::iota(size_t{0}, nK),
                row.begin(),
                [&](const size_t j) { return sigmaFromVariance(m_ahForward.localVariance[t][j]); });
            m_sigK.push_back(std::move(row));
        }
    }

    std::expected<AHInterpolator, std::string>
    AHInterpolator::build(std::vector<double> expiries,
                          std::vector<std::vector<double>> strikes,
                          std::vector<std::vector<double>> callPrices,
                          AhForwardSurfaceData ahForward)
    {
        if (expiries.size() < 2)
            return std::unexpected("Need >=2 expiries");
        if (strikes.size() != expiries.size() || callPrices.size() != expiries.size())
            return std::unexpected("Surface arrays must match expiries size");

        const size_t nT = expiries.size();
        for (size_t i = 0; i < nT; ++i)
        {
            if (strikes[i].size() < 3)
                return std::unexpected("Need >=3 strikes per expiry for stable local vol");
            if (callPrices[i].size() != strikes[i].size())
                return std::unexpected("Per-expiry strike/value sizes must match");
        }

        if (ahForward.kGrid.size() < 3)
            return std::unexpected("AH kGrid too small");
        if (ahForward.cFwd.size() != nT || ahForward.localVariance.size() != nT)
            return std::unexpected("AH forward arrays must match expiries size");
        const size_t nK = ahForward.kGrid.size();
        for (size_t i = 0; i < nT; ++i)
        {
            if (ahForward.cFwd[i].size() != nK || ahForward.localVariance[i].size() != nK)
                return std::unexpected("AH forward row length must match kGrid");
        }

        return AHInterpolator(std::move(expiries), std::move(strikes), std::move(callPrices), std::move(ahForward));
    }

    std::expected<size_t, std::string> AHInterpolator::localVolPillarIndexForTime(double T) const
    {
        if (const auto idx = pillarIndexForLocalVolTime(routePillarTime(T, m_expiries)))
            return *idx;
        return std::unexpected(
            std::string{"AHInterpolator: empty expiry list; cannot route local vol pillar for Monte Carlo"});
    }

    double AHInterpolator::localVolOnPillar(size_t pillarIdx, double K) const { return localVolFromVarianceSlice(pillarIdx, K); }

    double AHInterpolator::localVolFromVarianceSlice(size_t pillarIdx, double K) const
    {
        return interpSigK(pillarIdx, K);
    }

    double AHInterpolator::callPriceGapBetween(double T, double K, size_t i0, size_t i1) const
    {
        const auto& ah = m_ahForward;
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
                return std::numeric_limits<double>::quiet_NaN();
            cOld.swap(cNew);
        }

        CubicSplineInterpolator li(Kg, cOld);
        const double cFwd = li(K);
        const double df = ah.curve.discount(T);
        return cFwd * df;
    }

    double AHInterpolator::callPrice(const double T, const double K) const
    {
        const auto r = routePillarTime(T, m_expiries);
        switch (r.kind)
        {
        case PillarTimeKind::Empty:
            return 0.0;
        case PillarTimeKind::BeforeOrAtFirst:
            return interpStrikeSpline(m_strikes.front(), m_callPrices.front(), K);
        case PillarTimeKind::AfterOrAtLast:
            return interpStrikeSpline(m_strikes.back(), m_callPrices.back(), K);
        case PillarTimeKind::OnPillar:
            return interpStrikeSpline(m_strikes[r.i0], m_callPrices[r.i0], K);
        case PillarTimeKind::Between:
            return callPriceGapBetween(T, K, r.i0, r.i1);
        }
        return 0.0;
    }

    double AHInterpolator::localVol(const double T, const double K) const
    {
        const auto r = routePillarTime(T, m_expiries);
        switch (r.kind)
        {
        case PillarTimeKind::Empty:
            return 0.0;
        case PillarTimeKind::BeforeOrAtFirst:
            return localVolFromVarianceSlice(0, K);
        case PillarTimeKind::AfterOrAtLast:
            return localVolFromVarianceSlice(m_expiries.size() - 1uz, K);
        case PillarTimeKind::OnPillar:
            return localVolFromVarianceSlice(r.i0, K);
        case PillarTimeKind::Between:
            return localVolFromVarianceSlice(r.i0, K);
        }
        return 0.0;
    }

    double AHInterpolator::interpSigK(size_t pillarIdx, double K) const
    {
        const auto& Kg = m_ahForward.kGrid;
        const auto& sig = m_sigK[pillarIdx];
        if (Kg.size() < 2)
            return sig.empty() ? 0.0 : sig.front();
        if (K <= Kg.front())
            return sig.front();
        if (K >= Kg.back())
            return sig.back();
        const auto it = std::ranges::upper_bound(Kg, K);
        const size_t i1 = static_cast<size_t>(it - Kg.begin());
        const size_t i0 = i1 - 1;
        const double x0 = Kg[i0];
        const double x1 = Kg[i1];
        const double y0 = sig[i0];
        const double y1 = sig[i1];
        const double t = (K - x0) / (x1 - x0);
        return y0 + t * (y1 - y0);
    }
}
