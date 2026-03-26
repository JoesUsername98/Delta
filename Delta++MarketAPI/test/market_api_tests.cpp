#include <gtest/gtest.h>

#include <Delta++MarketAPI/api_key_provider.h>
#include <Delta++MarketAPI/http_client.h>
#include <Delta++MarketAPI/fred_client.h>
#include <Delta++MarketAPI/dtos.h>

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

TEST(MarketAPI_FredClient, ParsesObservationsFromJson)
{
    std::string json = R"({
        "observations": [
            {"date": "2024-01-01", "value": "4.50"},
            {"date": "2024-02-01", "value": "4.55"},
            {"date": "2024-03-01", "value": "."}
        ]
    })";

    auto http = std::make_shared<StubHttpClient>(json);
    auto keys = std::make_shared<StubKeyProvider>();
    FredClient client(http, keys);

    auto result = client.getSeriesObservations("DGS3MO");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->seriesId, "DGS3MO");
    EXPECT_EQ(result->observations.size(), 3);
    EXPECT_DOUBLE_EQ(result->observations[0].value.value(), 4.50);
    EXPECT_DOUBLE_EQ(result->observations[1].value.value(), 4.55);
    EXPECT_FALSE(result->observations[2].value.has_value());
}

TEST(MarketAPI_FredClient, MissingApiKeyReturnsError)
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
    FredClient client(http, keys);

    auto result = client.getSeriesObservations("DGS3MO");
    EXPECT_FALSE(result.has_value());
}
