#include <gtest/gtest.h>

#include <Delta++DB/reference_db.h>

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
        return std::filesystem::temp_directory_path()
               / (std::string("delta_reference_test_") + std::to_string(gen()) + ".sqlite");
    }
}

TEST(DeltaPP_ReferenceDB, UpsertAndQueryRoundTrip)
{
    const auto path = makeTempDbPath();
    std::error_code ec;
    std::filesystem::remove(path, ec);

    OptionsContractRow a;
    a.ticker = "O:TEST240119C00001000";
    a.underlying_ticker = "TEST";
    a.expiration_date = "2024-01-19";
    a.strike_price = 1.0;
    a.contract_type = "call";
    a.exercise_style = "american";

    OptionsContractRow b;
    b.ticker = "O:TEST240119P00001000";
    b.underlying_ticker = "TEST";
    b.expiration_date = "2024-01-19";
    b.strike_price = {};
    b.contract_type = "put";
    b.exercise_style = {};

    auto up = DB::upsertOptionsContracts(path, {a, b});
    ASSERT_TRUE(up.has_value()) << up.error();

    auto q = DB::queryAllOptionsContracts(path);
    ASSERT_TRUE(q.has_value()) << q.error();
    ASSERT_EQ(q->size(), 2u);

    // ORDER BY ticker — C before P
    const auto& r0 = (*q)[0];
    EXPECT_EQ(r0.ticker, a.ticker);
    EXPECT_EQ(r0.underlying_ticker, "TEST");
    EXPECT_EQ(r0.expiration_date, "2024-01-19");
    ASSERT_TRUE(r0.strike_price.has_value());
    EXPECT_DOUBLE_EQ(*r0.strike_price, 1.0);
    EXPECT_EQ(r0.contract_type, "call");
    ASSERT_TRUE(r0.exercise_style.has_value());
    EXPECT_EQ(*r0.exercise_style, "american");

    const auto& r1 = (*q)[1];
    EXPECT_EQ(r1.ticker, b.ticker);
    EXPECT_FALSE(r1.strike_price.has_value());
    EXPECT_EQ(r1.contract_type, "put");
    EXPECT_FALSE(r1.exercise_style.has_value() && !r1.exercise_style->empty());

    std::filesystem::remove(path, ec);
}

TEST(DeltaPP_ReferenceDB, UpsertConflictRefreshesRow)
{
    const auto path = makeTempDbPath();
    std::error_code ec;
    std::filesystem::remove(path, ec);

    OptionsContractRow v1;
    v1.ticker = "O:ABC";
    v1.underlying_ticker = "A";
    v1.expiration_date = "2024-01-01";
    v1.strike_price = 100.0;
    v1.contract_type = "call";
    v1.exercise_style = "american";

    ASSERT_TRUE(DB::upsertOptionsContracts(path, {v1}).has_value());

    OptionsContractRow v2 = v1;
    v2.underlying_ticker = "B";
    v2.strike_price = 200.0;

    ASSERT_TRUE(DB::upsertOptionsContracts(path, {v2}).has_value());

    auto q = DB::queryAllOptionsContracts(path);
    ASSERT_TRUE(q.has_value());
    ASSERT_EQ(q->size(), 1u);
    EXPECT_EQ((*q)[0].underlying_ticker, "B");
    ASSERT_TRUE((*q)[0].strike_price.has_value());
    EXPECT_DOUBLE_EQ(*(*q)[0].strike_price, 200.0);

    std::filesystem::remove(path, ec);
}
