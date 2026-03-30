#include <Delta++DataSync/option_contract_sync.h>

#include <Delta++DB/reference_db.h>
#include <Delta++MarketAPI/api_key_provider.h>
#include <Delta++MarketAPI/curl_http_client.h>
#include <Delta++MarketAPI/massive_client.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <thread>

using namespace DPP;

namespace DPP::DataSync
{
    namespace
    {
        void sleepRateLimit()
        {
            std::this_thread::sleep_for(std::chrono::duration<double>(12.1));
        }

        bool isTransientCurlError(const std::string& err)
        {
            std::string lower;
            lower.reserve(err.size());
            for (unsigned char c : err)
                lower.push_back(static_cast<char>(std::tolower(c)));
            return lower.find("timeout") != std::string::npos || lower.find("connection reset") != std::string::npos
                   || lower.find("couldn't connect") != std::string::npos
                   || lower.find("could not connect") != std::string::npos;
        }
    }

    std::expected<void, std::string> OptionContractSyncJob::verifyArgs(const ParsedArgs& args) const
    {
        if (!args.dbPath.has_value())
            return std::unexpected(std::string("option_contract_sync requires --db <path>"));
        if (args.dbPath->empty())
            return std::unexpected(std::string("--db path is empty"));
        if (args.underlyingTickers.empty())
            return std::unexpected(std::string("option_contract_sync requires --underlying_ticker"));
        return {};
    }

    int OptionContractSyncJob::run(const ParsedArgs& args)
    {
        const std::filesystem::path dbPath = *args.dbPath;

        auto http = std::make_shared<CurlHttpClient>();
        auto keys = std::make_shared<EnvApiKeyProvider>();
        MassiveClient client(http, keys);

        bool needSleepBeforeNextCall = false;

        for (const auto& underlying : args.underlyingTickers)
        {
            std::cout << "Underlying: " << underlying << "\n";

            std::map<std::string, std::string> q;
            q["underlying_ticker"] = underlying;
            q["limit"] = "1000";

            std::optional<OptionsContractsEnvelope> envOpt;
            for (;;)
            {
                if (needSleepBeforeNextCall)
                    sleepRateLimit();

                auto envExp = client.getOptionsContracts(q);
                needSleepBeforeNextCall = true;
                if (envExp.has_value())
                {
                    envOpt = std::move(envExp.value());
                    break;
                }
                if (isTransientCurlError(envExp.error()))
                {
                    std::cerr << "Transient (retrying first page): " << envExp.error() << "\n";
                    continue;
                }
                std::cerr << "Massive error: " << envExp.error() << "\n";
                envOpt = std::nullopt;
                break;
            }
            if (!envOpt.has_value())
                continue;

            auto env = std::move(*envOpt);
            std::size_t totalUpserted = 0;

            {
                auto r = DB::upsertOptionsContracts(dbPath, env.results);
                if (!r.has_value())
                {
                    std::cerr << "SQLite error: " << r.error() << "\n";
                    return 1;
                }
                totalUpserted += env.results.size();
            }

            while (env.next_url.has_value() && !env.next_url->empty())
            {
                std::optional<OptionsContractsEnvelope> nextOpt;
                for (;;)
                {
                    sleepRateLimit();

                    auto nextExp = client.getOptionsContractsNextUrl(*env.next_url);
                    needSleepBeforeNextCall = true;
                    if (nextExp.has_value())
                    {
                        nextOpt = std::move(nextExp.value());
                        break;
                    }
                    if (isTransientCurlError(nextExp.error()))
                    {
                        std::cerr << "Transient (retrying same next_url): " << nextExp.error() << "\n";
                        continue;
                    }
                    std::cerr << "Massive error (next_url): " << nextExp.error() << "\n";
                    nextOpt = std::nullopt;
                    break;
                }
                if (!nextOpt.has_value())
                    break;

                env = std::move(*nextOpt);
                auto r = DB::upsertOptionsContracts(dbPath, env.results);
                if (!r.has_value())
                {
                    std::cerr << "SQLite error: " << r.error() << "\n";
                    return 1;
                }
                totalUpserted += env.results.size();

                std::cout << "  upserted: " << totalUpserted << " row(s)\n";
            }

            std::cout << "Done " << underlying << ": " << totalUpserted << " row(s) upserted into " << dbPath.string()
                      << "\n";
        }

        return 0;
    }
}
