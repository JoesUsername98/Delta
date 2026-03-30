#include <Delta++DataSync/yield_curve_sync.h>

#include <Delta++DB/market_db.h>
#include <Delta++MarketAPI/api_key_provider.h>
#include <Delta++MarketAPI/curl_http_client.h>
#include <Delta++MarketAPI/massive_client.h>

#include <chrono>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <thread>
#include <vector>

using namespace DPP;

namespace DPP::DataSync
{
    namespace
    {
        void sleepRateLimit()
        {
            std::this_thread::sleep_for(std::chrono::duration<double>(12.1));
        }

        bool isTransientOrRateLimited(const std::string& err)
        {
            std::string lower;
            lower.reserve(err.size());
            for (unsigned char c : err)
                lower.push_back(static_cast<char>(std::tolower(c)));

            return lower.find("timeout") != std::string::npos || lower.find("connection reset") != std::string::npos
                   || lower.find("couldn't connect") != std::string::npos
                   || lower.find("could not connect") != std::string::npos
                   || lower.find("too many requests") != std::string::npos || lower.find("rate limit") != std::string::npos
                   || lower.find("429") != std::string::npos;
        }

        struct DateRange
        {
            std::chrono::sys_days from;
            std::chrono::sys_days to;
        };

        std::optional<DateRange> parseRange(std::string_view fromYmd, std::string_view toYmd, std::string& err)
        {
            auto parse = [&](std::string_view s) -> std::optional<std::chrono::sys_days> {
                int y = 0, m = 0, d = 0;
                if (s.size() != 10 || s[4] != '-' || s[7] != '-')
                    return std::nullopt;
                try
                {
                    y = std::stoi(std::string(s.substr(0, 4)));
                    m = std::stoi(std::string(s.substr(5, 2)));
                    d = std::stoi(std::string(s.substr(8, 2)));
                }
                catch (...)
                {
                    return std::nullopt;
                }

                const std::chrono::year_month_day ymd{std::chrono::year{y}, std::chrono::month{static_cast<unsigned>(m)},
                                                      std::chrono::day{static_cast<unsigned>(d)}};
                if (!ymd.ok())
                    return std::nullopt;
                return std::chrono::sys_days{ymd};
            };

            auto f = parse(fromYmd);
            if (!f.has_value())
            {
                err = "Invalid --from_asof (expected YYYY-MM-DD)";
                return std::nullopt;
            }
            auto t = parse(toYmd);
            if (!t.has_value())
            {
                err = "Invalid --to_asof (expected YYYY-MM-DD)";
                return std::nullopt;
            }
            if (*f > *t)
            {
                err = "--from_asof must be <= --to_asof";
                return std::nullopt;
            }

            return DateRange{*f, *t};
        }

        std::string toYmd(std::chrono::sys_days d)
        {
            const std::chrono::year_month_day ymd{d};
            const int y = int(ymd.year());
            const unsigned m = unsigned(ymd.month());
            const unsigned da = unsigned(ymd.day());

            auto two = [](unsigned x) { return (x < 10) ? ("0" + std::to_string(x)) : std::to_string(x); };
            return std::to_string(y) + "-" + two(m) + "-" + two(da);
        }

    }

    std::expected<void, std::string> YieldCurveJob::verifyArgs(const ParsedArgs& args) const
    {
        if (!args.dbPath.has_value())
            return std::unexpected(std::string("yield_curve requires --db <path>"));
        if (args.dbPath->empty())
            return std::unexpected(std::string("--db path is empty"));
        if (!args.fromAsof.has_value() || args.fromAsof->empty())
            return std::unexpected(std::string("yield_curve requires --from_asof YYYY-MM-DD"));
        if (!args.toAsof.has_value() || args.toAsof->empty())
            return std::unexpected(std::string("yield_curve requires --to_asof YYYY-MM-DD"));

        std::string err;
        if (!parseRange(*args.fromAsof, *args.toAsof, err).has_value())
            return std::unexpected(err);

        return {};
    }

    int YieldCurveJob::run(const ParsedArgs& args)
    {
        const std::filesystem::path dbPath = *args.dbPath;

        std::string err;
        const auto range = parseRange(*args.fromAsof, *args.toAsof, err);
        if (!range.has_value())
        {
            std::cerr << err << "\n";
            return 2;
        }

        auto http = std::make_shared<CurlHttpClient>();
        auto keys = std::make_shared<EnvApiKeyProvider>();
        MassiveClient client(http, keys);

        const std::string fromYmd = toYmd(range->from);
        const std::string toYmdInclusive = toYmd(range->to);

        std::optional<std::string> nextUrl;
        bool first = true;

        for (;;)
        {
            std::expected<TreasuryYieldsEnvelope, std::string> res;
            if (first)
            {
                res = client.getTreasuryYieldsRange(fromYmd, toYmdInclusive, 50000);
            }
            else
            {
                if (!nextUrl.has_value() || nextUrl->empty())
                    break;
                res = client.getTreasuryYieldsNextUrl(*nextUrl);
            }

            sleepRateLimit(); // after every HTTP call

            if (!res.has_value())
            {
                if (isTransientOrRateLimited(res.error()))
                {
                    std::cerr << "Transient/rate-limited (retrying page): " << res.error() << "\n";
                    sleepRateLimit();
                    continue;
                }
                std::cerr << "Error: " << res.error() << "\n";
                return 1;
            }

            // Only advance past the initial range request once it has succeeded.
            first = false;

            for (const auto& row : res->results)
            {
                auto w = DB::Market::upsertTreasuryYieldRow(dbPath, row);
                if (!w.has_value())
                {
                    std::cerr << "SQLite error: " << w.error() << "\n";
                    return 1;
                }
            }

            nextUrl = res->next_url;
            if (!nextUrl.has_value() || nextUrl->empty())
                break;
        }

        return 0;
    }
}

