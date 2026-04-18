#include "Delta++MarketAPI/massive_client.h"
#include "Delta++MarketAPI/massive_options_json.h"
#include "Delta++MarketAPI/massive_treasury_json.h"

#include <algorithm>
#include <sstream>
#include <string>

namespace DPP
{
    namespace
    {
        /// RFC 3986 path segment: `pchar` allows unreserved, sub-delims, ":", "@", and pct-encoded.
        /// Unlike curl_easy_escape (form/query encoding), this leaves ":" alone so option tickers like
        /// `O:AAPL260508C00170000` stay `O:...` in the path instead of `O%3A...`.
        static std::string encodeUriPathSegment(const std::string& s)
        {
            static const char kHex[] = "0123456789ABCDEF";
            std::string out;
            out.reserve(s.size() + 8);
            for (const unsigned char c : s)
            {
                const bool unreserved = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
                    || c == '-' || c == '.' || c == '_' || c == '~';
                const bool subDelim = c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')'
                    || c == '*' || c == '+' || c == ',' || c == ';' || c == '=';
                if (unreserved || subDelim || c == ':' || c == '@')
                    out += static_cast<char>(c);
                else
                {
                    out += '%';
                    out += kHex[c >> 4];
                    out += kHex[c & 0xF];
                }
            }
            return out;
        }
    } // namespace

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

        if (env->results.size() == 1)
            return env->results.front();

        return std::unexpected("No treasury yield row matching date " + dateStr);
    }

    std::expected<TreasuryYieldsEnvelope, std::string>
    MassiveClient::getTreasuryYieldsRange(const std::string_view fromYmd, const std::string_view toYmd, const int limit) const
    {
        auto keyResult = m_keys->getKey("MASSIVE_API_KEY");
        if (!keyResult.has_value())
            return std::unexpected(keyResult.error());

        const int cap = std::clamp(limit, 1, 50000);
        std::map<std::string, std::string> params = {
            {"apiKey", keyResult.value()},
            {"date.gte", std::string(fromYmd)},
            {"date.lte", std::string(toYmd)},
            {"limit", std::to_string(cap)},
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

        return env;
    }

    std::expected<TreasuryYieldsEnvelope, std::string>
    MassiveClient::getTreasuryYieldsNextUrl(const std::string_view absoluteUrl) const
    {
        auto keyResult = m_keys->getKey("MASSIVE_API_KEY");
        if (!keyResult.has_value())
            return std::unexpected(keyResult.error());

        const std::string url(absoluteUrl);
        auto resp = m_http->get(url, {});
        if (!resp.has_value())
            return std::unexpected(resp.error());

        auto env = parseTreasuryYieldsJson(resp.value());
        if (!env.has_value())
            return std::unexpected(env.error());

        if (!env->status.empty() && env->status != "OK")
            return std::unexpected("Massive API status: " + env->status);

        return env;
    }

    std::expected<OptionsContractsEnvelope, std::string>
    MassiveClient::getOptionsContracts(const std::map<std::string, std::string>& queryParams) const
    {
        auto keyResult = m_keys->getKey("MASSIVE_API_KEY");
        if (!keyResult.has_value())
            return std::unexpected(keyResult.error());

        std::map<std::string, std::string> params = queryParams;
        params["apiKey"] = keyResult.value();

        auto resp = m_http->get("https://api.massive.com/v3/reference/options/contracts", params);
        if (!resp.has_value())
            return std::unexpected(resp.error());

        auto env = parseOptionsContractsJson(resp.value());
        if (!env.has_value())
            return std::unexpected(env.error());

        if (!env->status.empty() && env->status != "OK")
            return std::unexpected("Massive API status: " + env->status);

        return env;
    }

    std::expected<OptionsContractsEnvelope, std::string>
    MassiveClient::getOptionsContractsNextUrl(const std::string_view absoluteUrl) const
    {
        auto keyResult = m_keys->getKey("MASSIVE_API_KEY");
        if (!keyResult.has_value())
            return std::unexpected(keyResult.error());

        const std::string url(absoluteUrl);
        auto resp = m_http->get(url, {});
        if (!resp.has_value())
            return std::unexpected(resp.error());

        auto env = parseOptionsContractsJson(resp.value());
        if (!env.has_value())
            return std::unexpected(env.error());

        if (!env->status.empty() && env->status != "OK")
            return std::unexpected("Massive API status: " + env->status);

        return env;
    }

    std::expected<OptionsAggregatesEnvelope, std::string>
    MassiveClient::getOptionsAggregates(const std::string& optionsTicker,
                                        const int multiplier,
                                        const std::string_view timespan,
                                        const std::string_view rangeFrom,
                                        const std::string_view rangeTo,
                                        const OptionsAggregatesQuery& query) const
    {
        auto keyResult = m_keys->getKey("MASSIVE_API_KEY");
        if (!keyResult.has_value())
            return std::unexpected(keyResult.error());

        const std::string eTicker = encodeUriPathSegment(optionsTicker);
        const std::string eTs = encodeUriPathSegment(std::string(timespan));
        const std::string eFrom = encodeUriPathSegment(std::string(rangeFrom));
        const std::string eTo = encodeUriPathSegment(std::string(rangeTo));

        std::ostringstream url;
        url << "https://api.massive.com/v2/aggs/ticker/" << eTicker << "/range/" << multiplier << "/" << eTs << "/"
            << eFrom << "/" << eTo;

        std::map<std::string, std::string> params;
        params["apiKey"] = keyResult.value();
        if (query.adjusted.has_value())
            params["adjusted"] = *query.adjusted ? "true" : "false";
        if (query.sort.has_value())
            params["sort"] = *query.sort;
        if (query.limit.has_value())
            params["limit"] = std::to_string(*query.limit);

        auto resp = m_http->get(url.str(), params);
        if (!resp.has_value())
            return std::unexpected(resp.error());

        auto env = parseOptionsAggregatesJson(resp.value());
        if (!env.has_value())
            return std::unexpected(env.error());

        if (!env->status.empty() && env->status != "OK")
            return std::unexpected("Massive API status: " + env->status);

        return env;
    }
} // namespace DPP
