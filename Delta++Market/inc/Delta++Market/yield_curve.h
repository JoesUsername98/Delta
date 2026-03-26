#pragma once

#include <vector>
#include <string>
#include <expected>
#include "quotes.h"
#include <Delta++Solver/interpolation.h>

namespace DPP
{
    class YieldCurve
    {
    public:
        static std::expected<YieldCurve, std::string>
        build(const std::vector<RateQuote>& quotes);

        double discount(double t) const;
        double zeroRate(double t) const;

        const std::vector<double>& tenors() const { return m_tenors; }
        const std::vector<double>& zeroRates() const { return m_zeroRates; }

    private:
        YieldCurve() = default;

        std::vector<double> m_tenors;
        std::vector<double> m_discountFactors;
        std::vector<double> m_zeroRates;
        CubicSplineInterpolator m_zeroRateInterp;
    };
}
