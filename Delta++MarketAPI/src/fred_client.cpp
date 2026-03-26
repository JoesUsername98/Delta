#include "Delta++MarketAPI/fred_client.h"

#include <charconv>

namespace DPP
{
    bool FredClient::tryParseJsonDouble(std::string_view s, double& out)
    {
        const char* const begin = s.data();
        const char* const end = begin + s.size();
        const auto result = std::from_chars(begin, end, out);
        return result.ec == std::errc{} && result.ptr == end;
    }

    FredClient::FredClient(std::shared_ptr<IHttpClient> http, std::shared_ptr<IApiKeyProvider> keys)
        : m_http(std::move(http)), m_keys(std::move(keys))
    {}

    // Minimal JSON parsing: extract date/value pairs from "observations" array.
    // Uses simple string search — production code should use a proper JSON library.
    // Numeric values are parsed with std::from_chars (non-throwing); std::stod is avoided.
    std::expected<FredSeriesResponse, std::string>
    FredClient::parseFredSeriesResponseFromJson(std::string_view json, const std::string& seriesId)
    {
        FredSeriesResponse result;
        result.seriesId = seriesId;

        constexpr std::string_view obsKey = "\"observations\"";
        const auto obsPos = json.find(obsKey);
        if (obsPos == std::string_view::npos)
            return std::unexpected("No 'observations' key in FRED response");

        std::size_t pos = obsPos;
        while (true)
        {
            const auto datePos = json.find("\"date\"", pos);
            if (datePos == std::string_view::npos)
                break;

            const auto dateOpen = json.find('"', datePos + kQuotedDateKeyLen);
            if (dateOpen == std::string_view::npos)
                break;
            const auto dateStart = dateOpen + 1;
            const auto dateEnd = json.find('"', dateStart);
            if (dateEnd == std::string_view::npos)
                break;

            const auto valPos = json.find("\"value\"", dateEnd);
            if (valPos == std::string_view::npos)
                break;
            const auto valOpen = json.find('"', valPos + kQuotedValueKeyLen);
            if (valOpen == std::string_view::npos)
                break;
            const auto valStart = valOpen + 1;
            const auto valEnd = json.find('"', valStart);
            if (valEnd == std::string_view::npos)
                break;

            FredObservation obs;
            obs.date.assign(json.data() + dateStart, dateEnd - dateStart);

            const std::string_view valPiece = json.substr(valStart, valEnd - valStart);
            if (valPiece != ".")
            {
                double v = 0.0;
                if (tryParseJsonDouble(valPiece, v))
                    obs.value = v;
            }

            result.observations.push_back(std::move(obs));
            pos = valEnd;
        }

        return result;
    }

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
        if (!startDate.empty()) 
            params["observation_start"] = startDate;
        if (!endDate.empty())   
            params["observation_end"] = endDate;

        auto resp = m_http->get("https://api.stlouisfed.org/fred/series/observations", params);
        if (!resp.has_value())
            return std::unexpected(resp.error());

        return parseFredSeriesResponseFromJson(resp.value(), seriesId);
    }
}
