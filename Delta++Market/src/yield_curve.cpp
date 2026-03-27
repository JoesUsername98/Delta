#include "Delta++Market/yield_curve.h"
#include <Delta++Solver/bootstrapper.h>
#include <algorithm>
#include <cmath>
#include <ranges>
#include <utility>

namespace DPP
{
    YieldCurve::YieldCurve(BootstrapResult&& r)
        : m_tenors(std::move(r.tenors))
        , m_discountFactors(std::move(r.discountFactors))
        , m_zeroRates(std::move(r.zeroRates))
        , m_zeroRateInterp(m_tenors, m_zeroRates)
    {}

    YieldCurve::YieldCurve(std::vector<double> tenors, std::vector<double> discountFactors, std::vector<double> zeroRates)
        : m_tenors(std::move(tenors))
        , m_discountFactors(std::move(discountFactors))
        , m_zeroRates(std::move(zeroRates))
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

    YieldCurve YieldCurve::parallelShift(double bump) const
    {
        std::vector<double> zr(m_zeroRates.size());
        std::ranges::transform(m_zeroRates, zr.begin(), [bump](double z) { return z + bump; });

        std::vector<double> df(m_tenors.size());
        std::ranges::transform(m_tenors, zr, df.begin(), [](double t, double z) { return std::exp(-z * t); });

        return YieldCurve(m_tenors, std::move(df), std::move(zr));
    }

    YieldCurve YieldCurve::keyRateBump(std::size_t knotIdx, double bump) const
    {
        if (knotIdx >= m_zeroRates.size())
            return *this;

        std::vector<double> zr = m_zeroRates;
        zr[knotIdx] += bump;

        std::vector<double> df(m_tenors.size());
        std::ranges::transform(m_tenors, zr, df.begin(), [](double t, double z) { return std::exp(-z * t); });

        return YieldCurve(m_tenors, std::move(df), std::move(zr));
    }

}
