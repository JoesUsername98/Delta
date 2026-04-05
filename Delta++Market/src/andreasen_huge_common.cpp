// Shared Dupire forward FD in log-strike (uniform x = ln K): stencil, one implicit step, log-strike grid.

#include <Delta++Market/andreasen_huge_common.h>

#include <Delta++Math/numeric.h>
#include <Delta++Math/tridiagonal.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <ranges>
#include <vector>

namespace
{
    // Forward Dupire spatial operator in x = ln K for v(t,x) = c(t, e^x):
    //   L v := v_xx - v_x.
    // Uniform x-grid (constant dx = ln K[j] - ln K[j-1]).
    // Stencil: 3 points (j-1, j, j+1). Second-order centered differences for v_xx and v_x (each O(dx^2) on a uniform mesh).
    struct DupireXLogOperatorStencil
    {
        double dx{};
        /// (L v)_j = Tm*v[j-1] + Tj*v[j] + Tp*v[j+1] at interior j.
        double Tm{};
        double Tj{};
        double Tp{};
    };

    std::optional<DupireXLogOperatorStencil> dupireXLogOperatorStencilFromK(const std::vector<double>& K)
    {
        if (K.size() < 2)
            return std::nullopt;
        const double dx = std::log(K[1]) - std::log(K[0]);
        if (!(dx > 0.0))
            return std::nullopt;
        const double invDx2 = 1.0 / (dx * dx);
        const double inv2Dx = 1.0 / (2.0 * dx);
        DupireXLogOperatorStencil s;
        s.dx = dx;
        s.Tm = invDx2 + inv2Dx;
        s.Tj = -2.0 * invDx2;
        s.Tp = invDx2 - inv2Dx;
        return s;
    }
}

namespace DPP::AhDupireFd
{
    void dupireXOperatorTimes(const std::vector<double>& K, const std::vector<double>& c, std::vector<double>& out)
    {
        const size_t n = K.size();
        out.assign(n, 0.0);
        if (n < 3)
            return;
        const auto st = dupireXLogOperatorStencilFromK(K);
        if (!st)
            return;
        const double Tm = st->Tm;
        const double Tj = st->Tj;
        const double Tp = st->Tp;
        for (size_t j = 1; j + 1 < n; ++j)
            out[j] = Tm * c[j - 1] + Tj * c[j] + Tp * c[j + 1];
    }

    bool implicitStep(const std::vector<double>& K,
                      const std::vector<double>& cOld,
                      const std::vector<double>& cBdry,
                      const double dT,
                      const std::vector<double>& w,
                      std::vector<double>& cNew)
    {
        const size_t n = K.size();
        if (n < 3 || cOld.size() != n || cBdry.size() != n || w.size() != n)
            return false;

        const auto st = dupireXLogOperatorStencilFromK(K);
        if (!st)
            return false;
        const double Tm = st->Tm;
        const double Tj = st->Tj;
        const double Tp = st->Tp;

        const size_t ni = n - 2;
        std::vector<double> lower(ni), diag(ni), upper(ni), rhs(ni);

        for (size_t k = 0; k < ni; ++k)
        {
            const size_t j = k + 1;
            const double beta = 0.5 * dT * w[j];
            lower[k] = (k == 0) ? 0.0 : (-beta * Tm);
            diag[k] = 1.0 - beta * Tj;
            upper[k] = (k + 1 == ni) ? 0.0 : (-beta * Tp);
            rhs[k] = cOld[j];
        }

        rhs[0] += 0.5 * dT * w[1] * Tm * cBdry[0];
        rhs[ni - 1] += 0.5 * dT * w[n - 2] * Tp * cBdry[n - 1];

        std::vector<double> z;
        if (!DPPMath::thomasSolve(lower, diag, upper, std::move(rhs), z, kDupireEps))
            return false;

        cNew.resize(n);
        cNew[0] = cBdry[0];
        cNew[n - 1] = cBdry[n - 1];
        for (size_t k = 0; k < ni; ++k)
            cNew[k + 1] = z[k];
        return true;
    }

    std::vector<double> buildLogStrikeGrid(const std::vector<double>& expiries,
                                           const std::vector<std::vector<double>>& strikes)
    {
        double kMin = std::numeric_limits<double>::infinity();
        double kMax = -std::numeric_limits<double>::infinity();
        for (const auto& row : strikes)
        {
            if (row.empty())
                continue;
            const auto [lo, hi] = std::minmax_element(row.begin(), row.end());
            kMin = std::min(kMin, *lo);
            kMax = std::max(kMax, *hi);
        }
        if (!(kMin > 0.0) || !(kMax > kMin))
            return {};

        const double pad = 0.005 * (std::log(kMax) - std::log(kMin) + 1e-12);
        const double logMin = std::log(kMin) - pad;
        const double logMax = std::log(kMax) + pad;

        const size_t nK = std::max(static_cast<size_t>(kMinGridPoints),
                                   static_cast<size_t>(32 + 4 * expiries.size()));
        const std::vector<double> xGrid = DPPMath::linspace(logMin, logMax, nK);
        return std::ranges::transform_view(xGrid, [](double x) { return std::exp(x); })
             | std::ranges::to<std::vector<double>>();
    }
}
