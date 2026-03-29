#pragma once

#include <string>
#include <vector>
#include <optional>

namespace DPP
{
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

    // Massive.com /fed/v1/treasury-yields — optional fields mirror API (sparse rows allowed).
    struct TreasuryYieldRow
    {
        std::string date;
        std::optional<double> yield_1_month;
        std::optional<double> yield_3_month;
        std::optional<double> yield_6_month;
        std::optional<double> yield_1_year;
        std::optional<double> yield_2_year;
        std::optional<double> yield_3_year;
        std::optional<double> yield_5_year;
        std::optional<double> yield_7_year;
        std::optional<double> yield_10_year;
        std::optional<double> yield_20_year;
        std::optional<double> yield_30_year;
    };

    struct TreasuryYieldsEnvelope
    {
        std::string status;
        std::vector<TreasuryYieldRow> results;
    };
}
