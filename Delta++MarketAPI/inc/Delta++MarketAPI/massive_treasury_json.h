#pragma once

#include "dtos.h"

#include <expected>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

namespace DPP::detail
{
    inline void readOptionalDouble(const nlohmann::json& j, const char* key, std::optional<double>& out)
    {
        const auto it = j.find(key);
        if (it == j.end() || it->is_null())
            return;
        out = it->get<double>();
    }
} // namespace DPP::detail

namespace DPP
{
    inline void from_json(const nlohmann::json& j, TreasuryYieldRow& r)
    {
        j.at("date").get_to(r.date);
        detail::readOptionalDouble(j, "yield_1_month", r.yield_1_month);
        detail::readOptionalDouble(j, "yield_3_month", r.yield_3_month);
        detail::readOptionalDouble(j, "yield_6_month", r.yield_6_month);
        detail::readOptionalDouble(j, "yield_1_year", r.yield_1_year);
        detail::readOptionalDouble(j, "yield_2_year", r.yield_2_year);
        detail::readOptionalDouble(j, "yield_3_year", r.yield_3_year);
        detail::readOptionalDouble(j, "yield_5_year", r.yield_5_year);
        detail::readOptionalDouble(j, "yield_7_year", r.yield_7_year);
        detail::readOptionalDouble(j, "yield_10_year", r.yield_10_year);
        detail::readOptionalDouble(j, "yield_20_year", r.yield_20_year);
        detail::readOptionalDouble(j, "yield_30_year", r.yield_30_year);
    }

    inline void from_json(const nlohmann::json& j, TreasuryYieldsEnvelope& e)
    {
        if (j.contains("status"))
            e.status = j["status"].get<std::string>();
        if (j.contains("results") && j["results"].is_array())
        {
            e.results.clear();
            e.results.reserve(j["results"].size());
            for (const auto& item : j["results"])
                e.results.push_back(item.get<TreasuryYieldRow>());
        }
    }

    /// serde-style parse: typed envelope + clear errors (Massive may add extra top-level keys; ignored).
    inline std::expected<TreasuryYieldsEnvelope, std::string> parseTreasuryYieldsJson(std::string_view jsonText)
    {
        try
        {
            auto j = nlohmann::json::parse(jsonText);
            return j.get<TreasuryYieldsEnvelope>();
        }
        catch (const nlohmann::json::exception& ex)
        {
            return std::unexpected(std::string("JSON: ") + ex.what());
        }
    }

    template<typename T>
    std::expected<T, std::string> parseJson(std::string_view jsonText)
    {
        try
        {
            auto j = nlohmann::json::parse(jsonText);
            return j.get<T>();
        }
        catch (const nlohmann::json::exception& ex)
        {
            return std::unexpected(std::string("JSON: ") + ex.what());
        }
    }
} // namespace DPP
