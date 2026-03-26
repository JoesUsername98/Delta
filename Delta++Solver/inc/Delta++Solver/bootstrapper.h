#pragma once

#include <vector>
#include <expected>
#include <string>

namespace DPP
{
    struct BootstrapInput
    {
        double tenor;   // in years
        double rate;    // annualised, e.g. 0.05 for 5%
    };

    struct BootstrapResult
    {
        std::vector<double> tenors;
        std::vector<double> discountFactors;
        std::vector<double> zeroRates;
    };

    using BootstrapOutcome = std::expected<BootstrapResult, std::string>;

    // Simple par-rate bootstrap assuming annual compounding
    BootstrapOutcome bootstrap(const std::vector<BootstrapInput>& inputs);
}
