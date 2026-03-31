#include <Delta++Market/andreasen_huge.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    bool isStrictlyIncreasing(const std::vector<double>& xs)
    {
        for (size_t i = 1; i < xs.size(); ++i)
            if (!(xs[i] > xs[i - 1]))
                return false;
        return true;
    }

    // Central finite difference first derivative in T using nearest neighbours.
    double dC_dT(const std::vector<double>& Ts, const std::vector<double>& C_at_K, size_t iT)
    {
        if (Ts.size() < 2)
            return 0.0;
        if (iT == 0)
            return (C_at_K[1] - C_at_K[0]) / (Ts[1] - Ts[0]);
        if (iT + 1 >= Ts.size())
            return (C_at_K[iT] - C_at_K[iT - 1]) / (Ts[iT] - Ts[iT - 1]);
        return (C_at_K[iT + 1] - C_at_K[iT - 1]) / (Ts[iT + 1] - Ts[iT - 1]);
    }

    // Central finite differences in strike on a non-uniform grid.
    double dC_dK(const std::vector<double>& Ks, const std::vector<double>& Cs, size_t j)
    {
        const size_t n = Ks.size();
        if (n < 2)
            return 0.0;
        if (j == 0)
            return (Cs[1] - Cs[0]) / (Ks[1] - Ks[0]);
        if (j + 1 >= n)
            return (Cs[n - 1] - Cs[n - 2]) / (Ks[n - 1] - Ks[n - 2]);

        const double k0 = Ks[j - 1], k1 = Ks[j], k2 = Ks[j + 1];
        const double c0 = Cs[j - 1], c1 = Cs[j], c2 = Cs[j + 1];
        // derivative of quadratic through 3 points at k1
        const double a = ( (c2 - c1) / (k2 - k1) - (c1 - c0) / (k1 - k0) ) / (k2 - k0);
        const double b = (c1 - c0) / (k1 - k0) - a * (k1 - k0);
        return b + 2.0 * a * (k1 - k0);
    }

    double d2C_dK2(const std::vector<double>& Ks, const std::vector<double>& Cs, size_t j)
    {
        const size_t n = Ks.size();
        if (n < 3)
            return 0.0;
        if (j == 0)
            j = 1;
        if (j + 1 >= n)
            j = n - 2;

        const double k0 = Ks[j - 1], k1 = Ks[j], k2 = Ks[j + 1];
        const double c0 = Cs[j - 1], c1 = Cs[j], c2 = Cs[j + 1];
        // second derivative of quadratic through 3 points
        const double a = ( (c2 - c1) / (k2 - k1) - (c1 - c0) / (k1 - k0) ) / (k2 - k0);
        return 2.0 * a;
    }
}

namespace DPP
{
    std::expected<LocalVolSurface, std::string> bootstrapAndreasenHuge(const AHInput& in)
    {
        if (!(in.spot > 0.0))
            return std::unexpected("spot must be > 0");
        if (in.expiries.size() < 2)
            return std::unexpected("Need >=2 expiries");
        if (!isStrictlyIncreasing(in.expiries))
            return std::unexpected("expiries must be strictly increasing");
        if (in.strikes.size() != in.expiries.size() || in.callPrices.size() != in.expiries.size())
            return std::unexpected("strikes/callPrices must match expiries size");
        if (!in.dividendYields.empty() && in.dividendYields.size() != in.expiries.size())
            return std::unexpected("dividendYields must be empty or match expiries size");

        // Build a local vol estimate on the raw knots using the Dupire local vol relation.
        // This is a pragmatic \"paper-default\" starting point over irregular raw knots.
        std::vector<std::vector<double>> sig(in.expiries.size());

        // Precompute C(T_i, K_ij) and also build time series for each strike index is not aligned across expiries,
        // so we compute dC/dT locally using neighbouring expiries at the *same strike* via interpolation.
        for (size_t i = 0; i < in.expiries.size(); ++i)
        {
            const auto& Ks = in.strikes[i];
            const auto& Cs = in.callPrices[i];
            if (Ks.size() < 3 || Cs.size() != Ks.size())
                return std::unexpected("Each expiry must have >=3 strikes and matching callPrices");

            // Basic monotonic/sanity
            for (size_t j = 1; j < Ks.size(); ++j)
                if (!(Ks[j] > Ks[j - 1]))
                    return std::unexpected("strikes must be strictly increasing within each expiry");

            sig[i].resize(Ks.size(), std::numeric_limits<double>::quiet_NaN());
        }

        // Helper: interpolate call price at a given expiry index i and strike K using cubic spline.
        auto callAt = [&](size_t i, double K) -> double {
            const auto& Ks = in.strikes[i];
            const auto& Cs = in.callPrices[i];
            CubicSplineInterpolator s(Ks, Cs);
            return s(K);
        };

        for (size_t i = 0; i < in.expiries.size(); ++i)
        {
            const double T = in.expiries[i];
            const double rT = in.curve.zeroRate(T);
            const double qT = in.dividendYields.empty() ? 0.0 : in.dividendYields[i];
            const double rq = rT - qT;

            const auto& Ks = in.strikes[i];
            const auto& Cs = in.callPrices[i];

            for (size_t j = 0; j < Ks.size(); ++j)
            {
                const double K = Ks[j];
                const double C = Cs[j];
                if (!(K > 0.0) || !(C >= 0.0))
                {
                    sig[i][j] = std::numeric_limits<double>::quiet_NaN();
                    continue;
                }

                // Approximate dC/dT using neighbouring expiries at same strike via spline interpolation in strike.
                std::vector<double> C_series(in.expiries.size());
                for (size_t it = 0; it < in.expiries.size(); ++it)
                    C_series[it] = callAt(it, K);
                const double Ct = dC_dT(in.expiries, C_series, i);

                const double Ck = dC_dK(Ks, Cs, j);
                const double Ckk = d2C_dK2(Ks, Cs, j);

                const double denom = 0.5 * K * K * Ckk;
                const double numer = Ct + rq * K * Ck - rq * C; // incorporates continuous dividend yield q(T)

                if (!(std::abs(denom) > 1e-14) || !(numer >= 0.0))
                {
                    sig[i][j] = std::numeric_limits<double>::quiet_NaN();
                    continue;
                }

                const double lv2 = numer / denom;
                sig[i][j] = (lv2 > 0.0) ? std::sqrt(lv2) : std::numeric_limits<double>::quiet_NaN();
            }
        }

        // Replace NaNs with nearest valid (simple fill) to ensure interpolator behaves.
        for (size_t i = 0; i < sig.size(); ++i)
        {
            auto& s = sig[i];
            // forward fill
            double last = std::numeric_limits<double>::quiet_NaN();
            for (double& v : s)
            {
                if (std::isfinite(v))
                    last = v;
                else if (std::isfinite(last))
                    v = last;
            }
            // backward fill
            last = std::numeric_limits<double>::quiet_NaN();
            for (size_t j = s.size(); j-- > 0;)
            {
                if (std::isfinite(s[j]))
                    last = s[j];
                else if (std::isfinite(last))
                    s[j] = last;
            }
            for (double& v : s)
                if (!std::isfinite(v))
                    v = 0.0;
        }

        return LocalVolSurface::build(in.expiries, in.strikes, in.callPrices, std::move(sig));
    }
}

