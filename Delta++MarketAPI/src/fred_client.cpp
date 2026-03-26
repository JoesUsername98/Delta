#include "Delta++MarketAPI/fred_client.h"
#include <sstream>

namespace DPP
{
    FredClient::FredClient(std::shared_ptr<IHttpClient> http, std::shared_ptr<IApiKeyProvider> keys)
        : m_http(std::move(http)), m_keys(std::move(keys))
    {}

    std::expected<FredSeriesResponse, std::string>
    FredClient::getSeriesObservations(const std::string& seriesId,
                                       const std::string& startDate,
                                       const std::string& endDate) const
    {
        auto keyResult = m_keys->getKey("FRED_API_KEY");
        if (!keyResult.has_value())
            return std::unexpected(keyResult.error());

        std::map<std::string, std::string> params = {
            {"series_id", seriesId},
            {"api_key", keyResult.value()},
            {"file_type", "json"}
        };
        if (!startDate.empty()) params["observation_start"] = startDate;
        if (!endDate.empty())   params["observation_end"] = endDate;

        auto resp = m_http->get("https://api.stlouisfed.org/fred/series/observations", params);
        if (!resp.has_value())
            return std::unexpected(resp.error());

        // Minimal JSON parsing: extract date/value pairs from "observations" array
        // Uses simple string search — production code should use a proper JSON library
        FredSeriesResponse result;
        result.seriesId = seriesId;

        const auto& json = resp.value();
        const std::string obsKey = "\"observations\"";
        auto obsPos = json.find(obsKey);
        if (obsPos == std::string::npos)
            return std::unexpected("No 'observations' key in FRED response");

        // Find each { "date":"...", "value":"..." } block
        size_t pos = obsPos;
        while (true)
        {
            auto datePos = json.find("\"date\"", pos);
            if (datePos == std::string::npos) break;

            auto dateStart = json.find('"', datePos + 6) + 1;
            auto dateEnd = json.find('"', dateStart);

            auto valPos = json.find("\"value\"", dateEnd);
            if (valPos == std::string::npos) break;
            auto valStart = json.find('"', valPos + 7) + 1;
            auto valEnd = json.find('"', valStart);

            FredObservation obs;
            obs.date = json.substr(dateStart, dateEnd - dateStart);
            std::string valStr = json.substr(valStart, valEnd - valStart);
            if (valStr != ".")
                obs.value = std::stod(valStr);

            result.observations.push_back(std::move(obs));
            pos = valEnd;
        }

        return result;
    }
}
