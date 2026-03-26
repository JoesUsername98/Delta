#pragma once

#include "http_client.h"
#include "api_key_provider.h"
#include "dtos.h"
#include <memory>
#include <expected>

namespace DPP
{
    class AlphaVantageClient
    {
    public:
        AlphaVantageClient(std::shared_ptr<IHttpClient> http, std::shared_ptr<IApiKeyProvider> keys);

        std::expected<OptionChainResponse, std::string>
        getOptionChain(const std::string& symbol) const;

    private:
        std::shared_ptr<IHttpClient> m_http;
        std::shared_ptr<IApiKeyProvider> m_keys;
    };
}
