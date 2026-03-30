#pragma once

#include "http_client.h"
#include "api_key_provider.h"
#include "dtos.h"

#include <expected>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>
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

        /// GET /fed/v1/treasury-yields using `date.any_of` filter.
        /// `apiKey` is injected from `MASSIVE_API_KEY`.
        std::expected<TreasuryYieldsEnvelope, std::string>
        getTreasuryYieldsAnyOf(const std::vector<std::string>& ymds) const;

        /// GET /fed/v1/treasury-yields using `date.gte`/`date.lte` (inclusive) with a large limit.
        /// `apiKey` is injected from `MASSIVE_API_KEY`.
        std::expected<TreasuryYieldsEnvelope, std::string>
        getTreasuryYieldsRange(std::string_view fromYmd, std::string_view toYmd, int limit = 50000) const;

        /// Follows a Massive `next_url` returned by treasury yields requests.
        /// Parses URL into base + query params, injects `apiKey` from `MASSIVE_API_KEY`.
        std::expected<TreasuryYieldsEnvelope, std::string> getTreasuryYieldsNextUrl(std::string_view nextUrl) const;

        /// GET /v3/reference/options/contracts — pass `underlying_ticker`, `limit`, `expiration_date`, etc.
        /// `apiKey` is injected from `MASSIVE_API_KEY`.
        std::expected<OptionsContractsEnvelope, std::string>
        getOptionsContracts(const std::map<std::string, std::string>& queryParams = {}) const;

        /// Follows a Massive `next_url` returned by `getOptionsContracts`.
        /// This parses the URL into base + query params, injects `apiKey` from `MASSIVE_API_KEY`,
        /// and forces `limit=1000` so pagination keeps pulling max-sized pages.
        std::expected<OptionsContractsEnvelope, std::string> getOptionsContractsNextUrl(std::string_view nextUrl) const;

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
