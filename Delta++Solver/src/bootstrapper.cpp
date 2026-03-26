#include "Delta++Solver/bootstrapper.h"
#include <cmath>

namespace DPP
{
    BootstrapOutcome bootstrap(const std::vector<BootstrapInput>& inputs)
    {
        if (inputs.empty())
            return std::unexpected("No bootstrap inputs provided");

        BootstrapResult result;
        result.tenors.reserve(inputs.size());
        result.discountFactors.reserve(inputs.size());
        result.zeroRates.reserve(inputs.size());

        for (const auto& inp : inputs)
        {
            if (inp.tenor <= 0.0)
                return std::unexpected("Tenor must be positive");

            const double onePlusY = 1.0 + inp.rate / 100.0;
            if (onePlusY <= 0.0)
                return std::unexpected("Effective rate 1 + rate/100 must be positive (rate > -100%)");

            const double df = 1.0 / std::pow(onePlusY, inp.tenor);
            
            // DF < 0 is not economically meaningful. Here DF > 0 iff onePlusY > 0.
            // if (!std::isfinite(df) || df <= 0.0)
            //    return std::unexpected("Discount factor is not a finite positive value");            
            const double zr = -std::log(df) / inp.tenor;

            result.tenors.push_back(inp.tenor);
            result.discountFactors.push_back(df);
            result.zeroRates.push_back(zr);
        }

        return result;
    }
}
