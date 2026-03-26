#include <gtest/gtest.h>
#include <cmath>

#include <Delta++Market/yield_curve.h>
#include <Delta++Market/vol_surface.h>
#include <Delta++Market/market_data_builder.h>
#include <Delta++Market/market_data_service.h>
#include <Delta++MarketAPI/http_client.h>
#include <Delta++MarketAPI/api_key_provider.h>

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
        {.tenor = 0.25, .rate = 4.50, .date = "2024-01-01", .source = "FRED"},
        {.tenor = 1.0,  .rate = 4.55, .date = "2024-01-01", .source = "FRED"},
        {.tenor = 2.0,  .rate = 4.60, .date = "2024-01-01", .source = "FRED"},
        {.tenor = 5.0,  .rate = 4.70, .date = "2024-01-01", .source = "FRED"},
        {.tenor = 10.0, .rate = 4.80, .date = "2024-01-01", .source = "FRED"},
    };

    auto result = YieldCurve::build(quotes);
    ASSERT_TRUE(result.has_value());

    // Discount factor at t=0 should be ~1
    EXPECT_NEAR(result->discount(0.001), 1.0, 0.01);

    // Zero rate at a knot should round-trip
    double zr1 = result->zeroRate(1.0);
    EXPECT_GT(zr1, 0.0);

    // Discount = exp(-zr * t)
    double df5 = result->discount(5.0);
    double zr5 = result->zeroRate(5.0);
    EXPECT_NEAR(df5, std::exp(-zr5 * 5.0), 1e-12);
}

TEST(Market_YieldCurve, EmptyQuotesError)
{
    auto result = YieldCurve::build({});
    EXPECT_FALSE(result.has_value());
}

TEST(Market_VolSurface, EmptyQuotesError)
{
    auto result = VolSurface::build({});
    EXPECT_FALSE(result.has_value());
}

TEST(Market_Builder, FredToRateQuotes)
{
    std::map<std::string, double> tenorMap = {
        {"DGS3MO", 0.25},
        {"DGS1", 1.0},
    };

    FredSeriesResponse resp3m;
    resp3m.seriesId = "DGS3MO";
    resp3m.observations = {{"2024-01-01", 4.50}, {"2024-02-01", 4.55}};

    FredSeriesResponse resp1y;
    resp1y.seriesId = "DGS1";
    resp1y.observations = {{"2024-01-01", 4.60}};

    std::vector<std::pair<std::string, FredSeriesResponse>> responses = {
        {"DGS3MO", resp3m},
        {"DGS1", resp1y},
    };

    auto quotes = fredToRateQuotes(tenorMap, responses, "2024-01-01");
    EXPECT_EQ(quotes.size(), 2);
    EXPECT_DOUBLE_EQ(quotes[0].tenor, 0.25);
    EXPECT_DOUBLE_EQ(quotes[0].rate, 4.50);
    EXPECT_DOUBLE_EQ(quotes[1].tenor, 1.0);
    EXPECT_DOUBLE_EQ(quotes[1].rate, 4.60);
}

// ---------- MarketDataService end-to-end with stub HTTP ----------

static std::string makeFredJson(const std::string& date, double value)
{
    return R"({"observations":[{"date":")" + date +
           R"(","value":")" + std::to_string(value) + R"("}]})";
}

// Stub HTTP that returns canned FRED JSON for any request
class MultiFredHttp : public IHttpClient
{
public:
    std::string m_date;
    std::map<std::string, double> m_values; // series_id -> rate

    HttpResponse get(const std::string&, const std::map<std::string, std::string>& params) const override
    {
        auto it = params.find("series_id");
        if (it != params.end())
        {
            auto vit = m_values.find(it->second);
            if (vit != m_values.end())
                return makeFredJson(m_date, vit->second);
        }
        return makeFredJson(m_date, 0.0);
    }
};

TEST(Market_Service, BuildYieldCurveEndToEnd)
{
    auto http = std::make_shared<MultiFredHttp>();
    http->m_date = "2024-03-01";
    http->m_values = {
        {"DGS3MO", 4.50},
        {"DGS1",   4.55},
        {"DGS2",   4.60},
        {"DGS5",   4.70},
        {"DGS10",  4.80},
        {"DGS30",  4.90},
    };

    auto keys = std::make_shared<StubKeys>();
    auto fred = std::make_shared<FredClient>(http, keys);
    auto av   = std::make_shared<AlphaVantageClient>(http, keys);

    MarketDataService svc(fred, av);

    // Request only the series we have stubs for
    auto result = svc.buildYieldCurve("2024-03-01",
        {"DGS3MO", "DGS1", "DGS2", "DGS5", "DGS10", "DGS30"});
    ASSERT_TRUE(result.has_value()) << result.error();

    EXPECT_EQ(result->tenors().size(), 6);
    EXPECT_GT(result->zeroRate(1.0), 0.0);

    // Discount identity: discount(t) = exp(-zeroRate(t) * t)
    for (double t : {0.25, 1.0, 5.0, 10.0})
    {
        double df = result->discount(t);
        double zr = result->zeroRate(t);
        EXPECT_NEAR(df, std::exp(-zr * t), 1e-12)
            << "at t=" << t;
    }
}
