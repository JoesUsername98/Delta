#include <gtest/gtest.h>

#include <Delta++DB/market_db.h>

#include <filesystem>
#include <random>
#include <string>

using namespace DPP;

namespace
{
    std::filesystem::path makeTempDbPath()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        return std::filesystem::temp_directory_path() / ("delta_marketdb_test_" + std::to_string(gen()) + ".sqlite");
    }
}

TEST(DeltaPP_MarketDB, UpsertAndQueryTreasuryRow)
{
    const auto path = makeTempDbPath();
    std::error_code ec;
    std::filesystem::remove(path, ec);

    TreasuryYieldRow row;
    row.date = "2024-01-02";
    row.yield_1_month = 4.50;
    row.yield_10_year = 4.80;

    auto w = DB::Market::upsertTreasuryYieldRow(path, row);
    ASSERT_TRUE(w.has_value()) << w.error();

    auto q = DB::Market::queryTreasuryYieldRow(path, "2024-01-02");
    ASSERT_TRUE(q.has_value()) << q.error();
    ASSERT_TRUE(q->has_value());
    EXPECT_EQ(q->value().date, "2024-01-02");
    ASSERT_TRUE(q->value().yield_1_month.has_value());
    EXPECT_DOUBLE_EQ(*q->value().yield_1_month, 4.50);
    ASSERT_TRUE(q->value().yield_10_year.has_value());
    EXPECT_DOUBLE_EQ(*q->value().yield_10_year, 4.80);

    std::filesystem::remove(path, ec);
}

TEST(DeltaPP_MarketDB, OptionsEodUpsertConflictAndQuery)
{
    const auto path = makeTempDbPath();
    std::error_code ec;
    std::filesystem::remove(path, ec);

    OptionsEodQuoteRow a;
    a.quote_date = "2023-06-01";
    a.expiration_date = "2023-06-16";
    a.strike_price = 4200.0;
    a.underlying_ticker = "SPX";
    a.contract_type = "call";
    a.bid = 1.1;
    a.ask = 1.2;

    ASSERT_TRUE(DB::Market::upsertOptionsEodQuotes(path, {a}).has_value());

    auto q1 = DB::Market::queryOptionsEodQuote(path, "2023-06-01", "2023-06-16", 4200.0, "SPX", "call");
    ASSERT_TRUE(q1.has_value()) << q1.error();
    ASSERT_TRUE(q1->has_value());
    ASSERT_TRUE(q1->value().bid.has_value());
    EXPECT_DOUBLE_EQ(*q1->value().bid, 1.1);
    ASSERT_TRUE(q1->value().ask.has_value());
    EXPECT_DOUBLE_EQ(*q1->value().ask, 1.2);

    OptionsEodQuoteRow b = a;
    b.bid = 2.0;
    b.ask = 2.5;
    ASSERT_TRUE(DB::Market::upsertOptionsEodQuotes(path, {b}).has_value());

    auto q2 = DB::Market::queryOptionsEodQuote(path, "2023-06-01", "2023-06-16", 4200.0, "SPX", "call");
    ASSERT_TRUE(q2.has_value());
    ASSERT_TRUE(q2->has_value());
    EXPECT_DOUBLE_EQ(*q2->value().bid, 2.0);
    EXPECT_DOUBLE_EQ(*q2->value().ask, 2.5);

    auto qMissing = DB::Market::queryOptionsEodQuote(path, "2023-06-01", "2023-06-16", 4200.0, "SPX", "put");
    ASSERT_TRUE(qMissing.has_value());
    EXPECT_FALSE(qMissing->has_value());

    std::filesystem::remove(path, ec);
}

