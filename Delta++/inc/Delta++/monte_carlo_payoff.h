#pragma once

#include <algorithm>

namespace DPP
{
    template <typename Derived>
    struct PayoffCRTP
    {
    };

    struct MCCallPayoff : PayoffCRTP<MCCallPayoff>
    {
        explicit MCCallPayoff(double strike) : m_strike(strike) {}
        double operator()(double s) const { return std::max<double>(s - m_strike, 0.0); }

    private:
        double m_strike;
    };

    struct MCPutPayoff : PayoffCRTP<MCPutPayoff>
    {
        explicit MCPutPayoff(double strike) : m_strike(strike) {}
        double operator()(double s) const { return std::max<double>(m_strike - s, 0.0); }

    private:
        double m_strike;
    };
}
