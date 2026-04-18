#pragma once

#include "http_client.h"
#include "api_key_provider.h"
#include "dtos.h"

#include <expected>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace DPP
{
    /// Optional query flags for GET /v2/aggs/ticker/.../range/...
    struct OptionsAggregatesQuery
    {
        std::optional<bool> adjusted;
        std::optional<std::string> sort;
        std::optional<int> limit;
    };

    /// HTTP client for Massive.com REST APIs (treasury, options reference, aggregates, etc.).
    class MassiveClient
    {
    public:
        MassiveClient(std::shared_ptr<IHttpClient> http, std::shared_ptr<IApiKeyProvider> keys);

        /// GET /fed/v1/treasury-yields with `date` filter; returns the row for that calendar date.
        std::expected<TreasuryYieldRow, std::string> getTreasuryYieldsForDate(std::string_view ymd) const;

        /// GET /fed/v1/treasury-yields with `date.gte` / `date.lte` (Massive max limit 50000 per page).
        std::expected<TreasuryYieldsEnvelope, std::string>
        getTreasuryYieldsRange(std::string_view fromYmd, std::string_view toYmd, int limit) const;

        /// GET a `next_url` from a prior treasury-yields response (full URL; typically includes apiKey).
        std::expected<TreasuryYieldsEnvelope, std::string> getTreasuryYieldsNextUrl(std::string_view absoluteUrl) const;

        /// GET /v3/reference/options/contracts — pass `underlying_ticker`, `limit`, `expiration_date`, etc.
        /// `apiKey` is injected from `MASSIVE_API_KEY`.
        std::expected<OptionsContractsEnvelope, std::string>
        getOptionsContracts(const std::map<std::string, std::string>& queryParams = {}) const;

        /// GET a `next_url` from a prior options contracts response (full URL; typically includes apiKey).
        std::expected<OptionsContractsEnvelope, std::string> getOptionsContractsNextUrl(std::string_view absoluteUrl) const;

        /// GET /v2/aggs/ticker/{optionsTicker}/range/{multiplier}/{timespan}/{from}/{to}
        /// Path segments are URL-encoded; optional `adjusted`, `sort`, `limit` as query params per Massive docs.
        std::expected<OptionsAggregatesEnvelope, std::string>
        getOptionsAggregates(const std::string& optionsTicker,
                               int multiplier,
                               std::string_view timespan,
                               std::string_view rangeFrom,
                               std::string_view rangeTo,
                               const OptionsAggregatesQuery& query = {}) const;

    private:
        std::shared_ptr<IHttpClient> m_http;
        std::shared_ptr<IApiKeyProvider> m_keys;
    };
} // namespace DPP
