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

