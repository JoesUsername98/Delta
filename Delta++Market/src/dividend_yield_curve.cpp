#include <Delta++Market/dividend_yield_curve.h>
#include <Delta++Market/put_call_parity.h>
#include <Delta++Market/yield_curve.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <ranges>
#include <utility>

namespace DPP
{
    double DividendYieldCurve::q(const double tYears) const
    {
        if (m_constantOnly)
            return m_constantQ;
        if (m_spline.has_value())
            return (*m_spline)(tYears);
        return m_constantQ;
    }

    std::expected<DividendYieldCurveBuildResult, std::string> buildDividendYieldCurveFromParity(
        const double spot,
        const YieldCurve& rates,
        const std::span<const ParityExpiryInput> pillars)
    {
        if (pillars.empty())
            return std::unexpected("No put/call parity pillars provided for dividend yield curve");

        DividendYieldCurveBuildResult out{DividendYieldCurve(DividendYieldCurve::Internal{}), {}};
        out.pillars.reserve(pillars.size());

        std::vector<std::pair<double, double>> knotPairs;
        knotPairs.reserve(pillars.size());

        for (const ParityExpiryInput& in : pillars)
        {
            if (!(in.tYears > 0.0))
                continue;

            std::vector<double> strikes = in.strikes;
            std::vector<double> calls = in.callMids;
            std::vector<double> puts = in.putMids;
            if (!strikes.empty())
                std::ranges::sort(std::views::zip(strikes, calls, puts), {},
                                  [](const auto& e) { return std::get<0>(e); });

            const double r = rates.zeroRate(in.tYears);
            const auto fitRes = inferDividendYieldFromPutCallParity(spot, in.tYears, strikes, calls, puts);

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
                diag.nUsed = static_cast<int>(strikes.size());
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
