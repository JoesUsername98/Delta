#include "Delta++MarketAPI/massive_client.h"
#include "Delta++MarketAPI/massive_treasury_json.h"

#include <map>
#include <string>

namespace DPP
{
    MassiveClient::MassiveClient(std::shared_ptr<IHttpClient> http, std::shared_ptr<IApiKeyProvider> keys)
        : m_http(std::move(http)), m_keys(std::move(keys))
    {}

    std::expected<TreasuryYieldRow, std::string> MassiveClient::getTreasuryYieldsForDate(std::string_view ymd) const
    {
        auto keyResult = m_keys->getKey("MASSIVE_API_KEY");
        if (!keyResult.has_value())
            return std::unexpected(keyResult.error());

        const std::string dateStr(ymd);
        std::map<std::string, std::string> params = {
            {"apiKey", keyResult.value()},
            {"date", dateStr},
            {"limit", "10"},
            {"sort", "date.asc"},
        };

        auto resp = m_http->get("https://api.massive.com/fed/v1/treasury-yields", params);
        if (!resp.has_value())
            return std::unexpected(resp.error());

        auto env = parseTreasuryYieldsJson(resp.value());
        if (!env.has_value())
            return std::unexpected(env.error());

        if (!env->status.empty() && env->status != "OK")
            return std::unexpected("Massive API status: " + env->status);

        if (env->results.empty())
            return std::unexpected("No treasury yield results for date " + dateStr);

        for (const auto& row : env->results)
        {
            if (row.date == dateStr)
                return row;
        }

        // If the API returns a single row without strict date match in body, accept it.
        if (env->results.size() == 1)
            return env->results.front();

        return std::unexpected("No treasury yield row matching date " + dateStr);
    }
} // namespace DPP
