#include "Delta++Solver/interpolation.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>

namespace DPP
{
    // ---------- LinearInterpolator ----------

    LinearInterpolator::LinearInterpolator(std::vector<double> xs, std::vector<double> ys)
        : m_xs(std::move(xs)), m_ys(std::move(ys))
    {
        if (m_xs.size() != m_ys.size() || m_xs.size() < 2)
            throw std::invalid_argument("LinearInterpolator requires >= 2 matching knots");
    }

    /**
     * @brief Piecewise linear interpolation with flat extrapolation.
     *
     * For x in [x_{i-1}, x_i], computes:
     *   y(x) = y_{i-1} + t * (y_i - y_{i-1})
     *   where t = (x - x_{i-1}) / (x_i - x_{i-1})
     *
     * For x < x_0: returns y_0 (flat extrapolation)
     * For x > x_{n-1}: returns y_{n-1} (flat extrapolation)
     */
    double 
    LinearInterpolator::operator()(double x) const
    {
        if (x <= m_xs.front()) 
            return m_ys.front();
        if (x >= m_xs.back()) 
            return m_ys.back();

        const auto it = std::lower_bound(m_xs.begin(), m_xs.end(), x);
        size_t i = static_cast<size_t>(it - m_xs.begin());
        if (i == 0) 
            i = 1;

        const double t = (x - m_xs[i - 1]) / (m_xs[i] - m_xs[i - 1]);
        return m_ys[i - 1] + t * (m_ys[i] - m_ys[i - 1]);
    }

    // ---------- CubicSplineInterpolator ----------

    CubicSplineInterpolator::CubicSplineInterpolator(std::vector<double> xs, std::vector<double> ys)
        : m_xs(std::move(xs)), m_ys(std::move(ys))
    {
        if (m_xs.size() != m_ys.size() || m_xs.size() < 2)
            throw std::invalid_argument("CubicSplineInterpolator requires >= 2 matching knots");
        build();
    }

    /**
     * @brief Builds coefficients for a natural cubic spline (a, b, c, d).
     *
     * On each [x_i, x_{i+1}], with dx = x - x_i:
     *   S_i(x) = a_i + b_i*dx + c_i*dx^2 + d_i*dx^3,   a_i = y_i.
     * Interior knots: C^1 and C^2 continuity; natural end conditions S''(x_0)=S''(x_n)=0 (c_0=c_n=0).
     * Solves the tridiagonal system for c, then b_i and d_i from the spline identities.
     */
    void 
    CubicSplineInterpolator::build()
    {
        const size_t n = m_xs.size() - 1;
        m_a.resize(n + 1);
        m_b.resize(n);
        m_c.resize(n + 1, 0.0);
        m_d.resize(n);

        for (size_t i = 0; i <= n; ++i) 
            m_a[i] = m_ys[i];

        std::vector<double> h(n);
        for (size_t i = 0; i < n; ++i) 
            h[i] = m_xs[i + 1] - m_xs[i];

        // Tridiagonal system for natural spline (c[0] = c[n] = 0)
        std::vector<double> alpha(n, 0.0);
        for (size_t i = 1; i < n; ++i)
            alpha[i] = 3.0 / h[i] * (m_a[i + 1] - m_a[i]) - 3.0 / h[i - 1] * (m_a[i] - m_a[i - 1]);

        std::vector<double> l(n + 1, 1.0), mu(n + 1, 0.0), z(n + 1, 0.0);
        for (size_t i = 1; i < n; ++i)
        {
            l[i] = 2.0 * (m_xs[i + 1] - m_xs[i - 1]) - h[i - 1] * mu[i - 1];
            mu[i] = h[i] / l[i];
            z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
        }
        for (size_t j = n; j-- > 0;)
        {
            m_c[j] = z[j] - mu[j] * m_c[j + 1];
            m_b[j] = (m_a[j + 1] - m_a[j]) / h[j] - h[j] * (m_c[j + 1] + 2.0 * m_c[j]) / 3.0;
            m_d[j] = (m_c[j + 1] - m_c[j]) / (3.0 * h[j]);
        }
    }

    /**
     * @brief Evaluates the natural cubic spline at x (flat extrapolation outside knots).
     *
     * On the segment [x_i, x_{i+1}] containing x, with dx = x - x_i:
     *   S(x) = a_i + b_i*dx + c_i*dx^2 + d_i*dx^3
     *
     * For x < x_0: returns y_0 (flat extrapolation)
     * For x > x_{n-1}: returns y_{n-1} (flat extrapolation)
     */
    double 
    CubicSplineInterpolator::operator()(double x) const
    {
        if (x <= m_xs.front()) 
            return m_ys.front();
        
        if (x >= m_xs.back())  
            return m_ys.back();

        const auto it = std::upper_bound(m_xs.begin(), m_xs.end(), x);
        const size_t i = static_cast<size_t>(it - m_xs.begin()) - 1;

        const double dx = x - m_xs[i];
        return m_a[i] + m_b[i] * dx + m_c[i] * dx * dx + m_d[i] * dx * dx * dx;
    }
}
