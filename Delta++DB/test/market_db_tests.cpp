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

TEST(DeltaPP_MarketDB, DistinctUnderlyingsAndEquityLastAndCallMids)
{
    const auto path = makeTempDbPath();
    std::error_code ec;
    std::filesystem::remove(path, ec);

    // Seed a single options quote row.
    OptionsEodQuoteRow a;
    a.quote_date = "2023-06-01";
    a.expiration_date = "2023-06-16";
    a.strike_price = 4200.0;
    a.underlying_ticker = "SPX";
    a.contract_type = "call";
    a.bid = 1.0;
    a.ask = 3.0;
    ASSERT_TRUE(DB::Market::upsertOptionsEodQuotes(path, {a}).has_value());

    // Distinct underlyings for date.
    auto und = DB::Market::queryDistinctUnderlyingsForDate(path, "2023-06-01");
    ASSERT_TRUE(und.has_value()) << und.error();
    ASSERT_EQ(und->size(), 1u);
    EXPECT_EQ((*und)[0], "SPX");

    // Equity last should be missing unless loader populated equities table.
    auto lastMissing = DB::Market::queryEquityLast(path, "2023-06-01", "SPX");
    ASSERT_TRUE(lastMissing.has_value()) << lastMissing.error();
    EXPECT_FALSE(lastMissing->has_value());

    // Call mids should return one point with mid=(1+3)/2=2.
    auto mids = DB::Market::queryCallMidsForDateUnderlying(path, "2023-06-01", "SPX");
    ASSERT_TRUE(mids.has_value()) << mids.error();
    ASSERT_EQ(mids->size(), 1u);
    EXPECT_NEAR((*mids)[0].mid, 2.0, 1e-12);
    EXPECT_DOUBLE_EQ((*mids)[0].strike, 4200.0);
    EXPECT_GT((*mids)[0].yearsToExpiry, 0.0);

    std::filesystem::remove(path, ec);
}

