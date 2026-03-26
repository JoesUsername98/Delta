#include "Delta++MarketAPI/alpha_vantage_client.h"
#include <sstream>

namespace DPP
{
    AlphaVantageClient::AlphaVantageClient(std::shared_ptr<IHttpClient> http,
                                             std::shared_ptr<IApiKeyProvider> keys)
        : m_http(std::move(http)), m_keys(std::move(keys))
    {}

    std::expected<OptionChainResponse, std::string>
    AlphaVantageClient::getOptionChain(const std::string& symbol) const
    {
        auto keyResult = m_keys->getKey("AV_API_KEY");
        if (!keyResult.has_value())
            return std::unexpected(keyResult.error());

        std::map<std::string, std::string> params = {
            {"function", "HISTORICAL_OPTIONS"},
            {"symbol", symbol},
            {"apikey", keyResult.value()}
        };

        auto resp = m_http->get("https://www.alphavantage.co/query", params);
        if (!resp.has_value())
            return std::unexpected(resp.error());

        // Placeholder: real implementation requires JSON parsing of the option chain
        OptionChainResponse result;
        result.underlying = symbol;
        result.date = ""; // populated from response

        return result;
    }
}
