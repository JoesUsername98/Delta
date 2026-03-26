#pragma once

#include <string>
#include <vector>
#include <optional>

namespace DPP
{
    struct FredObservation
    {
        std::string date;
        std::optional<double> value;
    };

    struct FredSeriesResponse
    {
        std::string seriesId;
        std::vector<FredObservation> observations;
    };

    struct OptionChainEntry
    {
        std::string expiry;
        double strike;
        double bid;
        double ask;
        std::optional<double> impliedVol;
        std::string type; // "call" or "put"
    };

    struct OptionChainResponse
    {
        std::string underlying;
        std::string date;
        std::vector<OptionChainEntry> options;
    };
}
