#pragma once

#include <vector>
#include <cstddef>

namespace DPP
{
    // Piecewise-linear interpolation on sorted (x, y) knots
    struct LinearInterpolator
    {
        std::vector<double> m_xs;
        std::vector<double> m_ys;

        LinearInterpolator() = default;
        LinearInterpolator(std::vector<double> xs, std::vector<double> ys);

        double operator()(double x) const;
    };

    // Natural cubic spline on sorted (x, y) knots
    struct CubicSplineInterpolator
    {
        std::vector<double> m_xs;
        std::vector<double> m_ys;
        std::vector<double> m_a, m_b, m_c, m_d;

        CubicSplineInterpolator() = default;
        CubicSplineInterpolator(std::vector<double> xs, std::vector<double> ys);

        double operator()(double x) const;

    private:
        void build();
    };
}
