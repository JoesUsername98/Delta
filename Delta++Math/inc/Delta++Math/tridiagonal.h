#pragma once

#include <cmath>
#include <vector>

namespace DPPMath
{
    /// Thomas algorithm for a tridiagonal system A x = b, with
    /// A = diag(d) + strictly lower(l) + strictly upper(u).
    /// On success, writes the solution into `x`. `diag` and `rhs` are modified.
    inline bool thomasSolve(const std::vector<double>& lower,
                            std::vector<double> diag,
                            const std::vector<double>& upper,
                            std::vector<double> rhs,
                            std::vector<double>& x,
                            double diagEps)
    {
        const int n = static_cast<int>(diag.size());
        if (n <= 0 || static_cast<int>(lower.size()) != n || static_cast<int>(upper.size()) != n ||
            static_cast<int>(rhs.size()) != n)
            return false;
        x.assign(n, 0.0);

        for (int i = 1; i < n; ++i)
        {
            if (std::abs(diag[i - 1]) < diagEps)
                return false;
            const double m = lower[i] / diag[i - 1];
            diag[i] -= m * upper[i - 1];
            rhs[i] -= m * rhs[i - 1];
        }
        if (std::abs(diag[n - 1]) < diagEps)
            return false;
        x[n - 1] = rhs[n - 1] / diag[n - 1];
        for (int i = n - 2; i >= 0; --i)
            x[i] = (rhs[i] - upper[i] * x[i + 1]) / diag[i];
        return true;
    }
}
