#pragma once

#include <algorithm>

namespace DPP
{
    struct IMCPayoff
    {
        virtual double operator()(double s) const = 0;
        virtual ~IMCPayoff() = default;
    };

    struct MCCallPayoff : IMCPayoff
    {
        explicit MCCallPayoff(double strike) : m_strike(strike) {}
        double operator()(double s) const override { return std::max<double>(s - m_strike, 0.0); }
    private:
        double m_strike;
    };

    struct MCPutPayoff : IMCPayoff
    {
        explicit MCPutPayoff(double strike) : m_strike(strike) {}
        double operator()(double s) const override { return std::max<double>(m_strike - s, 0.0); }
    private:
        double m_strike;
    };
}
