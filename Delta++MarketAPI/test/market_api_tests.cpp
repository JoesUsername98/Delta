#include <gtest/gtest.h>

#include <Delta++MarketAPI/api_key_provider.h>
#include <Delta++MarketAPI/http_client.h>
#include <Delta++MarketAPI/massive_client.h>
#include <Delta++MarketAPI/massive_treasury_json.h>

using namespace DPP;

// Stub HTTP client that returns canned responses for testing
class StubHttpClient : public IHttpClient
{
public:
    std::string m_response;

    explicit StubHttpClient(std::string response) : m_response(std::move(response)) {}

    HttpResponse get(const std::string& /*url*/,
                     const std::map<std::string, std::string>& /*params*/) const override
    {
        return m_response;
    }
};

// Stub key provider
class StubKeyProvider : public IApiKeyProvider
{
public:
    std::expected<std::string, std::string> getKey(const std::string& /*name*/) const override
    {
        return std::string("test_key");
    }
};

TEST(MarketAPI_EnvKey, MissingKeyReturnsError)
{
    EnvApiKeyProvider provider;
    auto result = provider.getKey("DEFINITELY_NOT_SET_KEY_12345");
    EXPECT_FALSE(result.has_value());
}

TEST(MarketAPI_MassiveJson, ParsesMinimalEnvelope)
{
    const char* json = R"({"status":"OK","results":[{"date":"1962-01-02","yield_10_year":4.06,"yield_1_year":3.22,"yield_5_year":3.88}]})";
    auto r = parseTreasuryYieldsJson(json);
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->status, "OK");
    ASSERT_EQ(r->results.size(), 1u);
    EXPECT_EQ(r->results[0].date, "1962-01-02");
    EXPECT_DOUBLE_EQ(r->results[0].yield_10_year.value(), 4.06);
}

TEST(MarketAPI_MassiveJson, ParsesRequestIdNumericAndFullYields)
{
    const char* json = R"({
        "count": 1,
        "request_id": 1,
        "status": "OK",
        "results": [{
            "date": "2024-01-15",
            "yield_1_month": 1.1, "yield_3_month": 2.2, "yield_6_month": 3.3,
            "yield_1_year": 4.4, "yield_2_year": 5.5, "yield_3_year": 6.6,
            "yield_5_year": 7.7, "yield_7_year": 8.8, "yield_10_year": 9.9,
            "yield_20_year": 10.1, "yield_30_year": 11.2
        }]
    })";
    auto r = parseTreasuryYieldsJson(json);
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->status, "OK");
    ASSERT_EQ(r->results.size(), 1u);
    const auto& row = r->results[0];
    EXPECT_DOUBLE_EQ(row.yield_1_month.value(), 1.1);
    EXPECT_DOUBLE_EQ(row.yield_30_year.value(), 11.2);
}

TEST(MarketAPI_MassiveJson, MalformedJson)
{
    auto r = parseTreasuryYieldsJson("{");
    EXPECT_FALSE(r.has_value());
}

TEST(MarketAPI_MassiveClient, GetTreasuryYieldsForDate)
{
    const char* json = R"({"status":"OK","results":[{"date":"2024-03-01","yield_10_year":4.06}]})";
    auto http = std::make_shared<StubHttpClient>(json);
    auto keys = std::make_shared<StubKeyProvider>();
    MassiveClient client(http, keys);
    auto row = client.getTreasuryYieldsForDate("2024-03-01");
    ASSERT_TRUE(row.has_value());
    EXPECT_DOUBLE_EQ(row->yield_10_year.value(), 4.06);
}

TEST(MarketAPI_MassiveClient, MissingApiKeyReturnsError)
{
    class NoKeyProvider : public IApiKeyProvider
    {
    public:
        std::expected<std::string, std::string> getKey(const std::string& name) const override
        {
            return std::unexpected("Key '" + name + "' not found");
        }
    };

    auto http = std::make_shared<StubHttpClient>("");
    auto keys = std::make_shared<NoKeyProvider>();
    MassiveClient client(http, keys);

    auto result = client.getTreasuryYieldsForDate("2024-03-01");
    EXPECT_FALSE(result.has_value());
}
