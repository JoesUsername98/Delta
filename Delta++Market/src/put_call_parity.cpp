#include <Delta++Market/put_call_parity.h>

#include <Delta++Math/regression.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <ranges>
#include <vector>

namespace
{
    bool finite(double x) { return std::isfinite(x); }
}

namespace DPP
{
    std::expected<PutCallParityFit, std::string> inferDividendYieldFromPutCallParity(
        const double spot,
        const double tYears,
        const std::span<const PutCallMidPoint> rows)
    {
        if (!(spot > 0.0))
            return std::unexpected("spot must be > 0");
        if (!(tYears > 0.0))
            return std::unexpected("tYears must be > 0");
        if (rows.empty())
            return std::unexpected("no parity rows");

        std::vector<size_t> idx(rows.size());
        std::iota(idx.begin(), idx.end(), 0);
        std::ranges::sort(idx, {}, [&](const size_t i) { return rows[i].strike; });

        std::vector<double> xs;
        std::vector<double> ys;
        xs.reserve(rows.size());
        ys.reserve(rows.size());
        for (const size_t i : idx)
        {
            const auto& r = rows[i];
            const double K = r.strike;
            if (!r.callMid.has_value() || !r.putMid.has_value())
                continue;
            const double C = *r.callMid;
            const double P = *r.putMid;
            if (!(K > 0.0) || !finite(C) || !finite(P))
                continue;
            const double y = C - P;
            if (!finite(y))
                continue;
            xs.push_back(K);
            ys.push_back(y);
        }

        const auto fit = DPPMath::olsFitLine(xs, ys, 3);
        if (!fit.has_value())
            return std::unexpected("parity OLS fit failed: " + fit.error());

        const double A = fit->intercept;
        const double B = -fit->slope;
        if (!(finite(A) && finite(B)))
            return std::unexpected("non-finite regression result");
        if (!(A > 0.0))
            return std::unexpected("parity fit A must be > 0 to infer q");
        if (!(B > 0.0))
            return std::unexpected("parity fit B must be > 0");

        PutCallParityFit out;
        out.A = A;
        out.B = B;
        out.forward = A / B;
        out.q = -std::log(A / spot) / tYears;
        out.nUsed = fit->nUsed;
        out.rmse = fit->rmse;
        return out;
    }

    std::expected<PutCallParityFit, std::string> inferDividendYieldFromPutCallParity(
        const double spot,
        const double tYears,
        const std::vector<double>& strikes,
        const std::vector<double>& callMids,
        const std::vector<double>& putMids)
    {
        if (strikes.size() != callMids.size() || strikes.size() != putMids.size())
            return std::unexpected("strikes/callMids/putMids must have same size");

        std::vector<PutCallMidPoint> tmp;
        tmp.reserve(strikes.size());
        for (size_t i = 0; i < strikes.size(); ++i)
        {
            tmp.push_back(PutCallMidPoint{
                .expirationDate = {},
                .yearsToExpiry = tYears,
                .strike = strikes[i],
                .callMid = callMids[i],
                .putMid = putMids[i],
            });
        }
        return inferDividendYieldFromPutCallParity(spot, tYears, std::span<const PutCallMidPoint>(tmp.data(), tmp.size()));
    }
}
