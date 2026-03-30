#pragma once

#include <Delta++MarketAPI/dtos.h>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace DPP::DB::Market
{
#if defined(DPP_MARKET_DB_PATH)
    inline std::filesystem::path defaultMarketDbPath()
    {
        return std::filesystem::path(DPP_MARKET_DB_PATH);
    }
#else
    inline std::filesystem::path defaultMarketDbPath()
    {
        return std::filesystem::path("data") / "marketDB.sqlite";
    }
#endif

    std::expected<void, std::string> upsertTreasuryYieldRow(const std::filesystem::path& dbPath,
                                                            const TreasuryYieldRow& row);

    std::expected<std::optional<TreasuryYieldRow>, std::string> queryTreasuryYieldRow(const std::filesystem::path& dbPath,
                                                                                      std::string_view ymd);

    std::expected<void, std::string> upsertOptionsEodQuotes(const std::filesystem::path& dbPath,
                                                              const std::vector<OptionsEodQuoteRow>& rows);

    std::expected<std::optional<OptionsEodQuoteRow>, std::string> queryOptionsEodQuote(
        const std::filesystem::path& dbPath,
        std::string_view quoteDate,
        std::string_view expirationDate,
        double strikePrice,
        std::string_view underlyingTicker,
        std::string_view contractType);
}

