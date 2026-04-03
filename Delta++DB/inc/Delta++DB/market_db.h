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
    inline std::filesystem::path findRepoRootForDb()
    {
        std::error_code ec;
        auto p = std::filesystem::current_path(ec);
        if (ec)
            return {};

        // Walk up a few levels looking for a stable repo marker.
        for (int i = 0; i < 12; ++i)
        {
            if (std::filesystem::is_regular_file(p / "Delta++DB" / "sql" / "market_schema.sql"))
                return p;
            if (!p.has_parent_path())
                break;
            p = p.parent_path();
        }
        return {};
    }

    inline std::filesystem::path defaultMarketDbPath()
    {
        if (const auto root = findRepoRootForDb(); !root.empty())
            return root / "data" / "marketDB.sqlite";
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

    struct CallMidPoint
    {
        double yearsToExpiry{};
        double strike{};
        double mid{};
    };

    /// Alias for `DPP::PutCallMidPoint`; `queryPutCallMidsForDateUnderlying` returns paired positive rows per SQL HAVING.
    using PutCallMidPoint = DPP::PutCallMidPoint;

    std::expected<std::vector<std::string>, std::string>
    queryDistinctUnderlyingsForDate(const std::filesystem::path& dbPath, std::string_view quoteDate);

    std::expected<std::optional<double>, std::string>
    queryEquityLast(const std::filesystem::path& dbPath, std::string_view quoteDate, std::string_view ticker);

    std::expected<std::vector<CallMidPoint>, std::string>
    queryCallMidsForDateUnderlying(const std::filesystem::path& dbPath,
                                   std::string_view quoteDate,
                                   std::string_view underlyingTicker);

    std::expected<std::vector<PutCallMidPoint>, std::string>
    queryPutCallMidsForDateUnderlying(const std::filesystem::path& dbPath,
                                      std::string_view quoteDate,
                                      std::string_view underlyingTicker);
}

