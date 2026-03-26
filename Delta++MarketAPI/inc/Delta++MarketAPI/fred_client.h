#pragma once

#include "http_client.h"
#include "api_key_provider.h"
#include "dtos.h"
#include <memory>
#include <expected>

namespace DPP
{
    class FredClient
    {
    public:
        FredClient(std::shared_ptr<IHttpClient> http, std::shared_ptr<IApiKeyProvider> keys);

        std::expected<FredSeriesResponse, std::string>
        getSeriesObservations(const std::string& seriesId,
                              const std::string& startDate = "",
                              const std::string& endDate = "") const;

    private:
        std::shared_ptr<IHttpClient> m_http;
        std::shared_ptr<IApiKeyProvider> m_keys;
    };
}
