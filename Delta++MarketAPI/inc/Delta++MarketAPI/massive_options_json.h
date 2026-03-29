#pragma once

#include "massive_treasury_json.h"

#include <expected>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

namespace DPP::detail
{
    inline void readOptionalInt64(const nlohmann::json& j, const char* key, std::optional<std::int64_t>& out)
    {
        const auto it = j.find(key);
        if (it == j.end() || it->is_null())
            return;
        out = it->get<std::int64_t>();
    }

    inline void readOptionalInt(const nlohmann::json& j, const char* key, std::optional<int>& out)
    {
        const auto it = j.find(key);
        if (it == j.end() || it->is_null())
            return;
        out = it->get<int>();
    }
} // namespace DPP::detail

namespace DPP
{
    inline void from_json(const nlohmann::json& j, OptionsAdditionalUnderlying& o)
    {
        detail::readOptionalDouble(j, "amount", o.amount);
        if (j.contains("type"))
            o.type = j["type"].get<std::string>();
        if (j.contains("underlying"))
            o.underlying = j["underlying"].get<std::string>();
    }

    inline void from_json(const nlohmann::json& j, OptionsContractRow& r)
    {
        if (j.contains("additional_underlyings") && j["additional_underlyings"].is_array())
        {
            std::vector<OptionsAdditionalUnderlying> v;
            v.reserve(j["additional_underlyings"].size());
            for (const auto& el : j["additional_underlyings"])
                v.push_back(el.get<OptionsAdditionalUnderlying>());
            r.additional_underlyings = std::move(v);
        }
        if (j.contains("cfi"))
            r.cfi = j["cfi"].get<std::string>();
        r.contract_type = j.value("contract_type", std::string{});
        detail::readOptionalInt(j, "correction", r.correction);
        if (j.contains("exercise_style"))
            r.exercise_style = j["exercise_style"].get<std::string>();
        r.expiration_date = j.value("expiration_date", std::string{});
        if (j.contains("primary_exchange"))
            r.primary_exchange = j["primary_exchange"].get<std::string>();
        detail::readOptionalDouble(j, "shares_per_contract", r.shares_per_contract);
        detail::readOptionalDouble(j, "strike_price", r.strike_price);
        r.ticker = j.value("ticker", std::string{});
        r.underlying_ticker = j.value("underlying_ticker", std::string{});
    }

    inline void from_json(const nlohmann::json& j, OptionsContractsEnvelope& e)
    {
        e.status = j.value("status", std::string{});
        if (j.contains("next_url"))
            e.next_url = j["next_url"].get<std::string>();
        if (j.contains("results") && j["results"].is_array())
        {
            e.results.clear();
            e.results.reserve(j["results"].size());
            for (const auto& item : j["results"])
                e.results.push_back(item.get<OptionsContractRow>());
        }
    }

    inline void from_json(const nlohmann::json& j, OptionsAggregateBar& b)
    {
        detail::readOptionalDouble(j, "c", b.c);
        detail::readOptionalDouble(j, "h", b.h);
        detail::readOptionalDouble(j, "l", b.l);
        detail::readOptionalDouble(j, "o", b.o);
        detail::readOptionalDouble(j, "v", b.v);
        detail::readOptionalDouble(j, "vw", b.vw);
        detail::readOptionalInt64(j, "n", b.n);
        detail::readOptionalInt64(j, "t", b.t);
    }

    inline void from_json(const nlohmann::json& j, OptionsAggregatesEnvelope& e)
    {
        e.status = j.value("status", std::string{});
        if (j.contains("ticker"))
            e.ticker = j["ticker"].get<std::string>();
        if (j.contains("adjusted"))
            e.adjusted = j["adjusted"].get<bool>();
        detail::readOptionalInt(j, "queryCount", e.queryCount);
        detail::readOptionalInt(j, "resultsCount", e.resultsCount);
        detail::readOptionalInt(j, "count", e.count);
        if (j.contains("results") && j["results"].is_array())
        {
            e.results.clear();
            e.results.reserve(j["results"].size());
            for (const auto& item : j["results"])
                e.results.push_back(item.get<OptionsAggregateBar>());
        }
    }

    inline std::expected<OptionsContractsEnvelope, std::string> parseOptionsContractsJson(std::string_view jsonText)
    {
        try
        {
            auto parsed = nlohmann::json::parse(jsonText);
            return parsed.get<OptionsContractsEnvelope>();
        }
        catch (const nlohmann::json::exception& ex)
        {
            return std::unexpected(std::string("JSON: ") + ex.what());
        }
    }

    inline std::expected<OptionsAggregatesEnvelope, std::string> parseOptionsAggregatesJson(std::string_view jsonText)
    {
        try
        {
            auto parsed = nlohmann::json::parse(jsonText);
            return parsed.get<OptionsAggregatesEnvelope>();
        }
        catch (const nlohmann::json::exception& ex)
        {
            return std::unexpected(std::string("JSON: ") + ex.what());
        }
    }
} // namespace DPP
