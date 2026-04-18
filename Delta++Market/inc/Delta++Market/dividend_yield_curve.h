#pragma once

#include <Delta++Market/put_call_parity.h>
#include <Delta++Market/yield_curve.h>
#include <Delta++Solver/interpolation.h>

#include <algorithm>
#include <cmath>
#include <concepts>
#include <expected>
#include <type_traits>
#include <limits>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace DPP
{

    /// One expiry: non-owning view of chain rows (`PutCallMidPoint`), e.g. one chunk from SQL ORDER BY expiry, strike.
    struct ParityExpiryPillar
    {
        double tYears{};
        std::string expirationDate;
        std::span<const PutCallMidPoint> rows;
    };

    /// Per-expiry parity diagnostics (maps to UI parity table).
    struct DividendYieldPillarDiag
    {
        std::string expirationDate;
        double texp_years{};
        double r{};
        double q{};
        double A{};
        double B{};
        double forward{};
        int nUsed{};
        double rmse{};
    };

    struct DividendYieldCurveBuildResult;

    /// Interpolated dividend yield q(T) from put–call parity pillars (cubic spline on knots; flat if one knot).
    class DividendYieldCurve
    {
    public:
        double q(double tYears) const;

        const std::vector<double>& tenors() const { return m_tenors; }
        const std::vector<double>& qKnots() const { return m_qKnots; }

        /// For each pillar: fit parity on row spans, then build q(T) on sorted unique tenors.
        /// Fails if `pillars` is empty (no inputs to process).
        template<std::ranges::forward_range R>
        requires std::same_as<std::remove_cv_t<std::ranges::range_value_t<R>>, ParityExpiryPillar>
        static std::expected<DividendYieldCurveBuildResult, std::string> buildFromParity(double spot,
                                                                                           const YieldCurve& rates,
                                                                                           R&& pillars);

        /// Constant dividend yield for all maturities (e.g. tests and flat market mode).
        static DividendYieldCurve flat(double q);

    private:
        DividendYieldCurve() = default;

        std::vector<double> m_tenors;
        std::vector<double> m_qKnots;
        bool m_constantOnly = false;
        double m_constantQ = 0.0;
        std::optional<CubicSplineInterpolator> m_spline;
    };

    struct DividendYieldCurveBuildResult
    {
        DividendYieldCurve curve;
        std::vector<DividendYieldPillarDiag> pillars;
    };

    template<std::ranges::forward_range R>
    requires std::same_as<std::remove_cv_t<std::ranges::range_value_t<R>>, ParityExpiryPillar>
    inline std::expected<DividendYieldCurveBuildResult, std::string> DividendYieldCurve::buildFromParity(
        double spot,
        const YieldCurve& rates,
        R&& pillars)
    {
        if (std::ranges::empty(pillars))
            return std::unexpected("No put/call parity pillars provided for dividend yield curve");

        DividendYieldCurveBuildResult out{DividendYieldCurve{}, {}};
        if constexpr (std::ranges::sized_range<std::remove_cvref_t<R>>)
        {
            const auto n = std::ranges::size(pillars);
            out.pillars.reserve(n);
        }

        std::vector<std::pair<double, double>> knotPairs;
        if constexpr (std::ranges::sized_range<std::remove_cvref_t<R>>)
            knotPairs.reserve(std::ranges::size(pillars));

        for (const ParityExpiryPillar& in : pillars)
        {
            const double r = rates.zeroRate(in.tYears);
            const auto fitRes = inferDividendYieldFromPutCallParity(spot, in.tYears, in.rows);

            DividendYieldPillarDiag diag;
            diag.expirationDate = in.expirationDate;
            diag.texp_years = in.tYears;
            diag.r = r;

            double qPillar = 0.0;
            if (fitRes.has_value())
            {
                qPillar = fitRes->q;
                diag.q = fitRes->q;
                diag.A = fitRes->A;
                diag.B = fitRes->B;
                diag.forward = fitRes->forward;
                diag.nUsed = fitRes->nUsed;
                diag.rmse = fitRes->rmse;
            }
            else
            {
                diag.q = 0.0;
                diag.A = std::numeric_limits<double>::quiet_NaN();
                diag.B = std::numeric_limits<double>::quiet_NaN();
                diag.forward = std::numeric_limits<double>::quiet_NaN();
                diag.nUsed = static_cast<int>(in.rows.size());
                diag.rmse = std::numeric_limits<double>::quiet_NaN();
            }

            out.pillars.push_back(std::move(diag));
            knotPairs.emplace_back(in.tYears, qPillar);
        }

        if (knotPairs.empty())
            return std::unexpected("No expiries with positive maturity for dividend yield curve");

        std::ranges::sort(knotPairs, {}, &std::pair<double, double>::first);

        std::vector<double> ts;
        std::vector<double> qs;
        ts.reserve(knotPairs.size());
        qs.reserve(knotPairs.size());
        for (const auto& [t, q] : knotPairs)
        {
            if (!ts.empty() && t == ts.back())
            {
                qs.back() = q;
                continue;
            }
            ts.push_back(t);
            qs.push_back(q);
        }

        DividendYieldCurve& c = out.curve;
        c.m_tenors = ts;
        c.m_qKnots = qs;

        if (ts.size() == 1)
        {
            c.m_constantOnly = true;
            c.m_constantQ = qs.front();
            c.m_spline.reset();
        }
        else
        {
            c.m_constantOnly = false;
            c.m_spline.emplace(ts, qs);
        }

        return out;
    }
}
