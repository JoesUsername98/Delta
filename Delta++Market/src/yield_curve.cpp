#include "Delta++Market/yield_curve.h"
#include <Delta++Solver/bootstrapper.h>
#include <algorithm>
#include <cmath>
#include <utility>

namespace DPP
{
    YieldCurve::YieldCurve(BootstrapResult&& r)
        : m_tenors(std::move(r.tenors))
        , m_discountFactors(std::move(r.discountFactors))
        , m_zeroRates(std::move(r.zeroRates))
        , m_zeroRateInterp(m_tenors, m_zeroRates)
    {}

    std::expected<YieldCurve, std::string>
    YieldCurve::build(const std::vector<RateQuote>& quotes)
    {
        if (quotes.empty())
            return std::unexpected("No rate quotes provided");

        // Bootstrap inputs (tenor + rate only); sort by tenor without copying full RateQuote rows.
        std::vector<BootstrapInput> inputs;
        inputs.reserve(quotes.size());
        for (const auto& q : quotes)
            inputs.emplace_back(q.tenor, q.rate);
        std::ranges::sort(inputs, {}, &BootstrapInput::tenor);

        auto bsResult = bootstrap(inputs);
        if (!bsResult.has_value())
            return std::unexpected(bsResult.error());

        return YieldCurve(std::move(bsResult.value()));
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
