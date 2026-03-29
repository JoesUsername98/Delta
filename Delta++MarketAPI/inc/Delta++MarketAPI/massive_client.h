#pragma once

#include "http_client.h"
#include "api_key_provider.h"
#include "dtos.h"

#include <expected>
#include <memory>
#include <string>
#include <string_view>

namespace DPP
{
    /// HTTP client for Massive.com REST APIs. Treasury yields are the first consumer; add equities,
    /// options, and reference endpoints here as DTOs + methods grow.
    class MassiveClient
    {
    public:
        MassiveClient(std::shared_ptr<IHttpClient> http, std::shared_ptr<IApiKeyProvider> keys);

        /// GET /fed/v1/treasury-yields with `date` filter; returns the row for that calendar date.
        std::expected<TreasuryYieldRow, std::string> getTreasuryYieldsForDate(std::string_view ymd) const;

    private:
        std::shared_ptr<IHttpClient> m_http;
        std::shared_ptr<IApiKeyProvider> m_keys;
    };
} // namespace DPP
