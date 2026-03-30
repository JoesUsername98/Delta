#include "Delta++MarketAPI/massive_client.h"
#include "Delta++MarketAPI/massive_options_json.h"
#include "Delta++MarketAPI/massive_treasury_json.h"

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

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

        static int fromHex(const unsigned char c)
        {
            if (c >= '0' && c <= '9')
                return c - '0';
            if (c >= 'a' && c <= 'f')
                return 10 + (c - 'a');
            if (c >= 'A' && c <= 'F')
                return 10 + (c - 'A');
            return -1;
        }

        static std::string urlDecodeQueryComponent(std::string_view s)
        {
            // Decodes percent-encoding for query strings. Treat '+' as space (common in query encoding).
            std::string out;
            out.reserve(s.size());
            for (size_t i = 0; i < s.size(); ++i)
            {
                const char ch = s[i];
                if (ch == '%' && i + 2 < s.size())
                {
                    const int hi = fromHex(static_cast<unsigned char>(s[i + 1]));
                    const int lo = fromHex(static_cast<unsigned char>(s[i + 2]));
                    if (hi >= 0 && lo >= 0)
                    {
                        out.push_back(static_cast<char>((hi << 4) | lo));
                        i += 2;
                        continue;
                    }
                }
                if (ch == '+')
                    out.push_back(' ');
                else
                    out.push_back(ch);
            }
            return out;
        }

        static std::map<std::string, std::string> parseQueryString(std::string_view qs)
        {
            std::map<std::string, std::string> out;
            size_t pos = 0;
            while (pos < qs.size())
            {
                const size_t amp = qs.find('&', pos);
                const size_t end = (amp == std::string_view::npos) ? qs.size() : amp;
                const auto token = qs.substr(pos, end - pos);

                if (!token.empty())
                {
                    const size_t eq = token.find('=');
                    const auto k = token.substr(0, eq);
                    const auto v = (eq == std::string_view::npos) ? std::string_view{} : token.substr(eq + 1);
                    if (!k.empty())
                        out[urlDecodeQueryComponent(k)] = urlDecodeQueryComponent(v);
                }

                if (amp == std::string_view::npos)
                    break;
                pos = amp + 1;
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
    MassiveClient::getTreasuryYieldsAnyOf(const std::vector<std::string>& ymds) const
    {
        auto keyResult = m_keys->getKey("MASSIVE_API_KEY");
        if (!keyResult.has_value())
            return std::unexpected(keyResult.error());

        std::ostringstream anyOf;
        bool first = true;
        for (const auto& d : ymds)
        {
            if (d.empty())
                continue;
            if (!first)
                anyOf << ",";
            anyOf << d;
            first = false;
        }

        std::map<std::string, std::string> params = {
            {"apiKey", keyResult.value()},
            {"date.any_of", anyOf.str()},
            {"sort", "date.asc"},
            {"limit", std::to_string(static_cast<int>(ymds.size()))},
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
    MassiveClient::getTreasuryYieldsRange(const std::string_view fromYmd, const std::string_view toYmd, const int limit) const
    {
        auto keyResult = m_keys->getKey("MASSIVE_API_KEY");
        if (!keyResult.has_value())
            return std::unexpected(keyResult.error());

        std::map<std::string, std::string> params = {
            {"apiKey", keyResult.value()},
            {"date.gte", std::string(fromYmd)},
            {"date.lte", std::string(toYmd)},
            {"sort", "date.asc"},
            {"limit", std::to_string(limit)},
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

    std::expected<TreasuryYieldsEnvelope, std::string> MassiveClient::getTreasuryYieldsNextUrl(
        const std::string_view nextUrl) const
    {
        auto keyResult = m_keys->getKey("MASSIVE_API_KEY");
        if (!keyResult.has_value())
            return std::unexpected(keyResult.error());

        const std::string urlStr(nextUrl);
        const auto qpos = urlStr.find('?');
        const std::string baseUrl = (qpos == std::string::npos) ? urlStr : urlStr.substr(0, qpos);
        const std::string_view query =
            (qpos == std::string::npos) ? std::string_view{} : std::string_view(urlStr).substr(qpos + 1);

        std::map<std::string, std::string> params = parseQueryString(query);

        params.erase("apiKey");
        params["apiKey"] = keyResult.value();

        auto resp = m_http->get(baseUrl, params);
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

    std::expected<OptionsContractsEnvelope, std::string> MassiveClient::getOptionsContractsNextUrl(
        const std::string_view nextUrl) const
    {
        auto keyResult = m_keys->getKey("MASSIVE_API_KEY");
        if (!keyResult.has_value())
            return std::unexpected(keyResult.error());

        const std::string urlStr(nextUrl);
        const auto qpos = urlStr.find('?');
        const std::string baseUrl = (qpos == std::string::npos) ? urlStr : urlStr.substr(0, qpos);
        const std::string_view query = (qpos == std::string::npos) ? std::string_view{} : std::string_view(urlStr).substr(qpos + 1);

        std::map<std::string, std::string> params = parseQueryString(query);

        // Never trust apiKey in next_url; inject from key provider.
        params.erase("apiKey");
        params["apiKey"] = keyResult.value();

        // Force maximum page size for subsequent calls too.
        params["limit"] = "1000";

        auto resp = m_http->get(baseUrl, params);
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
