#pragma once

#include <expected>
#include <string>
#include <vector>

namespace DPPMath
{
    struct OlsLineFit
    {
        // y = intercept + slope * x
        double intercept{};
        double slope{};
        int nUsed{};
        double rmse{};
    };

    // Ordinary least squares fit for y = a + b x.
    // Skips non-finite samples; requires at least minN used samples.
    std::expected<OlsLineFit, std::string> olsFitLine(
        const std::vector<double>& xs,
        const std::vector<double>& ys,
        int minN = 3);
}

