#pragma once

#include <string>
#include <optional>
#include <vector>

namespace DPP
{
    struct RateQuote
    {
        double tenor;       // in years
        double rate;        // annualised, e.g. 5.0 for 5%
        std::string date;
        std::string source; // e.g. "FRED"
    };

    struct OptionQuote
    {
        std::string expiry;
        double strike;
        double bid;
        double ask;
        std::optional<double> impliedVol;
        std::string underlying;
        std::string date;
    };
}
