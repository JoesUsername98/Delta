#include "Delta++Market/yield_curve.h"
#include <Delta++Solver/bootstrapper.h>
#include <algorithm>
#include <cmath>

namespace DPP
{
    std::expected<YieldCurve, std::string>
    YieldCurve::build(const std::vector<RateQuote>& quotes)
    {
        if (quotes.empty())
            return std::unexpected("No rate quotes provided");

        // Sort by tenor
        auto sorted = quotes;
        std::ranges::sort(sorted, {}, &RateQuote::tenor);

        // Bootstrap
        std::vector<BootstrapInput> inputs;
        inputs.reserve(sorted.size());
        for (const auto& q : sorted)
            inputs.push_back({q.tenor, q.rate});

        auto bsResult = bootstrap(inputs);
        if (!bsResult.has_value())
            return std::unexpected(bsResult.error());

        YieldCurve curve;
        curve.m_tenors = bsResult->tenors;
        curve.m_discountFactors = bsResult->discountFactors;
        curve.m_zeroRates = bsResult->zeroRates;
        curve.m_zeroRateInterp = CubicSplineInterpolator(curve.m_tenors, curve.m_zeroRates);

        return curve;
    }

    double YieldCurve::discount(double t) const
    {
        double zr = zeroRate(t);
        return std::exp(-zr * t);
    }

    double YieldCurve::zeroRate(double t) const
    {
        return m_zeroRateInterp(t);
    }
}
