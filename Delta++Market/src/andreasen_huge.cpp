// Forward-measure Dupire PDE with implicit finite differences and sequential slice bootstrap
// (Andreasen–Huge style), not the analytic spot-measure Dupire formula on sparse knots.

#include <Delta++Market/andreasen_huge.h>

#include <Delta++Solver/interpolation.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace
{
    constexpr double kSigmaMin = 0.01;
    constexpr double kSigmaMax = 3.0;
    constexpr double kVarMin = kSigmaMin * kSigmaMin;
    constexpr double kVarMax = kSigmaMax * kSigmaMax;
    constexpr double kDupireEps = 1e-14;
    constexpr int kMaxIter = 80;
    constexpr double kTol = 1e-7;
    constexpr int kMinGridPoints = 32;

    bool isStrictlyIncreasing(const std::vector<double>& xs)
    {
        for (size_t i = 1; i < xs.size(); ++i)
            if (!(xs[i] > xs[i - 1]))
                return false;
        return true;
    }

    // (L*c)_j for interior j: second central difference on non-uniform K (three-point stencil).
    void laplacianTimes(const std::vector<double>& K, const std::vector<double>& c, std::vector<double>& out)
    {
        const size_t n = K.size();
        out.assign(n, 0.0);
        for (size_t j = 1; j + 1 < n; ++j)
        {
            const double hl = K[j] - K[j - 1];
            const double hr = K[j + 1] - K[j];
            const double Ljm = 2.0 / (hl * (hl + hr));
            const double Ljj = -2.0 / (hl * hr);
            const double Ljp = 2.0 / (hr * (hl + hr));
            out[j] = Ljm * c[j - 1] + Ljj * c[j] + Ljp * c[j + 1];
        }
    }

    // Coefficients for row j of discrete Laplacian (same as laplacianTimes).
    void laplacianRowCoeffs(size_t j, const std::vector<double>& K, double& Lm, double& Ljj, double& Ljp)
    {
        const double hl = K[j] - K[j - 1];
        const double hr = K[j + 1] - K[j];
        Lm = 2.0 / (hl * (hl + hr));
        Ljj = -2.0 / (hl * hr);
        Ljp = 2.0 / (hr * (hl + hr));
    }

    bool thomasSolve(const std::vector<double>& lower,
                     std::vector<double> diag,
                     const std::vector<double>& upper,
                     std::vector<double> rhs,
                     std::vector<double>& x)
    {
        const int n = static_cast<int>(diag.size());
        if (n <= 0 || static_cast<int>(lower.size()) != n || static_cast<int>(upper.size()) != n ||
            static_cast<int>(rhs.size()) != n)
            return false;
        x.assign(n, 0.0);

        for (int i = 1; i < n; ++i)
        {
            if (std::abs(diag[i - 1]) < kDupireEps)
                return false;
            const double m = lower[i] / diag[i - 1];
            diag[i] -= m * upper[i - 1];
            rhs[i] -= m * rhs[i - 1];
        }
        if (std::abs(diag[n - 1]) < kDupireEps)
            return false;
        x[n - 1] = rhs[n - 1] / diag[n - 1];
        for (int i = n - 2; i >= 0; --i)
            x[i] = (rhs[i] - upper[i] * x[i + 1]) / diag[i];
        return true;
    }

    // Implicit step: (I - 0.5*dT*K_j^2*w_j*L) c_new = c_old on interior; Dirichlet c_target at edges.
    bool implicitStep(const std::vector<double>& K,
                      const std::vector<double>& cOld,
                      const std::vector<double>& cBdry,
                      double dT,
                      const std::vector<double>& w,
                      std::vector<double>& cNew)
    {
        const size_t n = K.size();
        if (n < 3 || cOld.size() != n || cBdry.size() != n || w.size() != n)
            return false;

        const size_t ni = n - 2;
        std::vector<double> lower(ni), diag(ni), upper(ni), rhs(ni);

        for (size_t k = 0; k < ni; ++k)
        {
            const size_t j = k + 1;
            double Lm = 0.0, Ljj = 0.0, Ljp = 0.0;
            laplacianRowCoeffs(j, K, Lm, Ljj, Ljp);
            const double kj = K[j];
            const double a = 0.5 * dT * kj * kj * w[j];
            lower[k] = (k == 0) ? 0.0 : (-a * Lm);
            diag[k] = 1.0 - a * Ljj;
            upper[k] = (k + 1 == ni) ? 0.0 : (-a * Ljp);
            rhs[k] = cOld[j];
        }

        // Move fixed boundary values to RHS: M[j,j-1]*c0 and M[j,j+1]*c_{n-1}.
        {
            double Lm = 0.0, Ljj = 0.0, Ljp = 0.0;
            laplacianRowCoeffs(1, K, Lm, Ljj, Ljp);
            const double a = 0.5 * dT * K[1] * K[1] * w[1];
            rhs[0] += a * Lm * cBdry[0];
        }
        if (ni >= 2)
        {
            double Lm = 0.0, Ljj = 0.0, Ljp = 0.0;
            laplacianRowCoeffs(n - 2, K, Lm, Ljj, Ljp);
            const double a = 0.5 * dT * K[n - 2] * K[n - 2] * w[n - 2];
            rhs[ni - 1] += a * Ljp * cBdry[n - 1];
        }

        std::vector<double> z;
        if (!thomasSolve(lower, diag, upper, std::move(rhs), z))
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
            for (double k : row)
            {
                kMin = std::min(kMin, k);
                kMax = std::max(kMax, k);
            }
        if (!(kMin > 0.0) || !(kMax > kMin))
            return {};

        const double pad = 0.02 * (std::log(kMax) - std::log(kMin) + 1e-12);
        const double logMin = std::log(kMin) - pad;
        const double logMax = std::log(kMax) + pad;

        const size_t nK = std::max(static_cast<size_t>(kMinGridPoints),
                                   static_cast<size_t>(32 + 4 * expiries.size()));
        std::vector<double> K;
        K.reserve(nK);
        for (size_t i = 0; i < nK; ++i)
        {
            const double t = static_cast<double>(i) / static_cast<double>(nK - 1);
            K.push_back(std::exp(logMin + t * (logMax - logMin)));
        }
        return K;
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

        for (size_t i = 0; i < in.expiries.size(); ++i)
        {
            const auto& Ks = in.strikes[i];
            const auto& Cs = in.callPrices[i];
            if (Ks.size() < 3 || Cs.size() != Ks.size())
                return std::unexpected("Each expiry must have >=3 strikes and matching callPrices");
            for (size_t j = 1; j < Ks.size(); ++j)
                if (!(Ks[j] > Ks[j - 1]))
                    return std::unexpected("strikes must be strictly increasing within each expiry");
        }

        const std::vector<double> Kgrid = buildLogStrikeGrid(in.expiries, in.strikes);
        if (Kgrid.size() < 3)
            return std::unexpected("Could not build strike grid");

        const size_t nK = Kgrid.size();
        const size_t nT = in.expiries.size();

        // Forward undiscounted call prices c = C / DF on the common grid (per expiry).
        std::vector<std::vector<double>> cMarket(nT, std::vector<double>(nK));
        for (size_t i = 0; i < nT; ++i)
        {
            const double T = in.expiries[i];
            const double df = in.curve.discount(T);
            if (!(df > 0.0))
                return std::unexpected("invalid discount factor");

            std::vector<double> cfwd(in.strikes[i].size());
            for (size_t j = 0; j < in.strikes[i].size(); ++j)
                cfwd[j] = in.callPrices[i][j] / df;

            LinearInterpolator li(in.strikes[i], cfwd);
            for (size_t j = 0; j < nK; ++j)
                cMarket[i][j] = li(Kgrid[j]);
        }

        std::vector<std::vector<double>> localVarGrid(nT, std::vector<double>(nK, kVarMin));

        std::vector<double> LcBuf(nK);
        std::vector<double> cOld(nK), cNew(nK), w(nK, kVarMin);
        std::vector<double> cBdry(nK);

        for (size_t i = 0; i < nT; ++i)
        {
            const double Tprev = (i == 0) ? 0.0 : in.expiries[i - 1];
            const double Tcur = in.expiries[i];
            const double dT = Tcur - Tprev;
            if (!(dT > 0.0))
                return std::unexpected("non-positive time step");

            const std::vector<double>& cTarget = cMarket[i];

            if (i == 0)
            {
                for (size_t j = 0; j < nK; ++j)
                    cOld[j] = std::max(in.spot - Kgrid[j], 0.0);
            }
            else
            {
                for (size_t j = 0; j < nK; ++j)
                    cOld[j] = cMarket[i - 1][j];
            }

            cBdry = cTarget;

            laplacianTimes(Kgrid, cTarget, LcBuf);
            for (size_t j = 0; j < nK; ++j)
            {
                if (j == 0 || j + 1 == nK)
                {
                    w[j] = kVarMin;
                    continue;
                }
                const double denom = dT * Kgrid[j] * Kgrid[j] * std::max(LcBuf[j], kDupireEps);
                double num = 2.0 * (cTarget[j] - cOld[j]);
                if (denom > kDupireEps)
                    w[j] = std::clamp(num / denom, kVarMin, kVarMax);
                else
                    w[j] = kVarMin;
            }

            double err = 1.0;
            for (int it = 0; it < kMaxIter && err > kTol; ++it)
            {
                if (!implicitStep(Kgrid, cOld, cBdry, dT, w, cNew))
                    return std::unexpected("implicit PDE solve failed");

                laplacianTimes(Kgrid, cNew, LcBuf);
                for (size_t j = 1; j + 1 < nK; ++j)
                {
                    const double denom = dT * Kgrid[j] * Kgrid[j] * std::max(LcBuf[j], kDupireEps);
                    const double num = 2.0 * (cTarget[j] - cOld[j]);
                    if (denom > kDupireEps)
                        w[j] = std::clamp(num / denom, kVarMin, kVarMax);
                }
                w[0] = w[1];
                w[nK - 1] = w[nK - 2];

                err = 0.0;
                for (size_t j = 1; j + 1 < nK; ++j)
                {
                    const double scale = std::max(1.0, std::abs(cTarget[j]));
                    err = std::max(err, std::abs(cNew[j] - cTarget[j]) / scale);
                }
            }

            for (size_t j = 0; j < nK; ++j)
                localVarGrid[i][j] = w[j];
        }

        // Map local vol to each expiry's market strikes (piecewise linear in K).
        std::vector<std::vector<double>> localVolsOut(nT);
        for (size_t i = 0; i < nT; ++i)
        {
            std::vector<double> sigK(nK);
            for (size_t j = 0; j < nK; ++j)
                sigK[j] = std::clamp(std::sqrt(std::max(localVarGrid[i][j], 0.0)), kSigmaMin, kSigmaMax);

            LinearInterpolator li(Kgrid, sigK);
            const auto& Km = in.strikes[i];
            localVolsOut[i].resize(Km.size());
            for (size_t j = 0; j < Km.size(); ++j)
                localVolsOut[i][j] = li(Km[j]);
        }

        return LocalVolSurface::build(in.expiries, in.strikes, in.callPrices, std::move(localVolsOut));
    }
}
