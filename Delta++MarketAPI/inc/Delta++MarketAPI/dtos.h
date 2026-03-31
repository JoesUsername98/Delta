#pragma once

#include <cstdint>
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

    // --- Delta++ market DB (SQLite) DTOs ---
    // Mirrors `options_eod_quotes` in `Delta++DB/sql/market_schema.sql`.
    struct OptionsEodQuoteRow
    {
        std::string quote_date;        // YYYY-MM-DD
        std::string expiration_date;   // YYYY-MM-DD
        double strike_price{};
        std::string underlying_ticker; // e.g. "SPX"
        std::string contract_type;     // "call" or "put"
        std::optional<double> bid;
        std::optional<double> ask;
    };

    // --- GET /v3/reference/options/contracts ---

    struct OptionsAdditionalUnderlying
    {
        std::optional<double> amount;
        std::string type;
        std::string underlying;
    };

    struct OptionsContractRow
    {
        std::optional<std::vector<OptionsAdditionalUnderlying>> additional_underlyings;
        std::optional<std::string> cfi;
        std::string contract_type;
        std::optional<int> correction;
        std::optional<std::string> exercise_style;
        std::string expiration_date;
        std::optional<std::string> primary_exchange;
        std::optional<double> shares_per_contract;
        std::optional<double> strike_price;
        std::string ticker;
        std::string underlying_ticker;
    };

    struct OptionsContractsEnvelope
    {
        std::string status;
        std::optional<std::string> next_url;
        std::vector<OptionsContractRow> results;
    };

    // --- GET /v2/aggs/ticker/{optionsTicker}/range/... ---

    struct OptionsAggregateBar
    {
        std::optional<double> c;
        std::optional<double> h;
        std::optional<double> l;
        std::optional<double> o;
        std::optional<double> v;
        std::optional<double> vw;
        std::optional<std::int64_t> n;
        std::optional<std::int64_t> t;
    };

    struct OptionsAggregatesEnvelope
    {
        std::optional<std::string> ticker;
        std::optional<bool> adjusted;
        std::optional<int> queryCount;
        std::optional<int> resultsCount;
        std::optional<int> count;
        std::string status;
        std::vector<OptionsAggregateBar> results;
    };
}
