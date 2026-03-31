#include <gtest/gtest.h>
#include <cmath>

#include <Delta++Market/yield_curve.h>
#include <Delta++Market/market_data_builder.h>
#include <Delta++Market/market_data_service.h>
#include <Delta++MarketAPI/http_client.h>
#include <Delta++MarketAPI/api_key_provider.h>
#include <Delta++MarketAPI/massive_client.h>

using namespace DPP;

// ---------- Stubs for end-to-end service tests (no live API calls) ----------

class StubHttp : public IHttpClient
{
public:
    std::string m_response;
    explicit StubHttp(std::string r) : m_response(std::move(r)) {}
    HttpResponse get(const std::string&, const std::map<std::string, std::string>&) const override
    {
        return m_response;
    }
};

class StubKeys : public IApiKeyProvider
{
public:
    std::expected<std::string, std::string> getKey(const std::string&) const override
    {
        return std::string("stub_key");
    }
};

TEST(Market_YieldCurve, BuildFromQuotes)
{
    std::vector<RateQuote> quotes = {
        {.tenor = 0.25, .rate = 4.50, .date = "2024-01-01", .source = "Massive"},
        {.tenor = 1.0,  .rate = 4.55, .date = "2024-01-01", .source = "Massive"},
        {.tenor = 2.0,  .rate = 4.60, .date = "2024-01-01", .source = "Massive"},
        {.tenor = 5.0,  .rate = 4.70, .date = "2024-01-01", .source = "Massive"},
        {.tenor = 10.0, .rate = 4.80, .date = "2024-01-01", .source = "Massive"},
    };

    auto result = YieldCurve::build(quotes);
    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->discount(0.001), 1.0, 0.01);

    double zr1 = result->zeroRate(1.0);
    EXPECT_GT(zr1, 0.0);

    double df5 = result->discount(5.0);
    double zr5 = result->zeroRate(5.0);
    EXPECT_NEAR(df5, std::exp(-zr5 * 5.0), 1e-12);
}

TEST(Market_YieldCurve, EmptyQuotesError)
{
    auto result = YieldCurve::build({});
    EXPECT_FALSE(result.has_value());
}

TEST(Market_Builder, MassiveTreasuryRowToRateQuotes)
{
    TreasuryYieldRow row;
    row.date = "2024-01-01";
    row.yield_2_year = 4.0;
    row.yield_10_year = 5.0;
    auto quotes = massiveTreasuryRowToRateQuotes(row);
    EXPECT_EQ(quotes.size(), 2u);
    EXPECT_EQ(quotes[0].source, "Massive");
    EXPECT_DOUBLE_EQ(quotes[0].tenor, 2.0);
    EXPECT_DOUBLE_EQ(quotes[1].tenor, 10.0);
}

// ---------- MarketDataService yield curve (Massive) with stub HTTP ----------

TEST(Market_Service, BuildYieldCurveEndToEnd)
{
    std::string json = R"({"status":"OK","results":[{"date":"2024-03-01",
        "yield_3_month":4.50,"yield_1_year":4.55,"yield_2_year":4.60,"yield_5_year":4.70,"yield_10_year":4.80,"yield_30_year":4.90}]})";
    auto http = std::make_shared<StubHttp>(json);
    auto keys = std::make_shared<StubKeys>();
    auto massive = std::make_shared<MassiveClient>(http, keys);
    MarketDataService svc(massive);

    auto result = svc.buildYieldCurve("2024-03-01");
    ASSERT_TRUE(result.has_value()) << result.error();

    EXPECT_EQ(result->tenors().size(), 6u);
    EXPECT_GT(result->zeroRate(1.0), 0.0);

    for (double t : {0.25, 1.0, 5.0, 10.0})
    {
        double df = result->discount(t);
        double zr = result->zeroRate(t);
        EXPECT_NEAR(df, std::exp(-zr * t), 1e-12) << "at t=" << t;
    }
}

TEST(Market_Service, BuildYieldCurveSparseYields)
{
    std::string json =
        R"({"status":"OK","results":[{"date":"2024-03-01","yield_1_year":3.0,"yield_2_year":3.2,"yield_5_year":3.5,"yield_10_year":4.0}]})";
    auto http = std::make_shared<StubHttp>(json);
    auto keys = std::make_shared<StubKeys>();
    auto massive = std::make_shared<MassiveClient>(http, keys);
    MarketDataService svc(massive);

    auto result = svc.buildYieldCurve("2024-03-01");
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_GE(result->tenors().size(), 2u);
    EXPECT_GT(result->zeroRate(2.0), 0.0);
}
