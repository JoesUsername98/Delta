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
            const double zr = std::log1p(inp.rate / 100.0);
            const double df = std::exp(-zr * inp.tenor);           

            result.tenors.push_back(inp.tenor);
            result.discountFactors.push_back(df);
            result.zeroRates.push_back(zr);
        }

        return result;
    }
}
