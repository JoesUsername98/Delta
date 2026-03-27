#include "api_tester_state.h"

#include <Delta++Market/market_data_service.h>
#include <Delta++MarketAPI/alpha_vantage_client.h>
#include <Delta++MarketAPI/api_key_provider.h>
#include <Delta++MarketAPI/curl_http_client.h>
#include <Delta++MarketAPI/fred_client.h>
#include <Delta++MarketAPI/http_client.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "shared_curve_cache.h"

using DPP::MarketDataService;

namespace
{
    std::map<std::string, double> makeStubFredValues()
    {
        return {
            {"DGS1MO", 4.45},
            {"DGS3MO", 4.50},
            {"DGS6MO", 4.52},
            {"DGS1", 4.55},
            {"DGS2", 4.60},
            {"DGS5", 4.70},
            {"DGS10", 4.80},
            {"DGS30", 4.90},
        };
    }

    std::string makeFredJson(const std::string& date, double value)
    {
        std::ostringstream oss;
        oss << R"({"observations":[{"date":")" << date << R"(","value":")" << value << R"("}]})";
        return oss.str();
    }

    class StubHttpClient : public DPP::IHttpClient
    {
    public:
        explicit StubHttpClient(std::map<std::string, double> fredValues)
            : m_fredValues(std::move(fredValues))
        {
        }

        DPP::HttpResponse get(const std::string& /*url*/,
                                const std::map<std::string, std::string>& params) const override
        {
            auto sidIt = params.find("series_id");
            if (sidIt == params.end())
                return std::string("{}");

            const std::string seriesId = sidIt->second;

            auto dateIt = params.find("observation_start");
            if (dateIt == params.end())
                dateIt = params.find("observation_end");
            const std::string date = (dateIt != params.end()) ? dateIt->second : std::string("1970-01-01");

            auto vIt = m_fredValues.find(seriesId);
            if (vIt == m_fredValues.end())
                return std::unexpected("Stub: no value for FRED series_id '" + seriesId + "'");

            return makeFredJson(date, vIt->second);
        }

    private:
        std::map<std::string, double> m_fredValues;
    };

    class StubApiKeyProvider : public DPP::IApiKeyProvider
    {
    public:
        std::expected<std::string, std::string> getKey(const std::string& /*name*/) const override
        {
            return std::string("stub_key");
        }
    };
} // namespace

void ApiTesterState::refreshCurveAtT()
{
    if (!m_curve.has_value())
        return;
    m_curveZeroRateAtT = m_curve->zeroRate(m_tYears);
    m_curveDiscountAtT = m_curve->discount(m_tYears);
}

void ApiTesterState::fetchYieldCurveFromFred()
{
    m_hasCurve = false;
    m_curve.reset();
    m_curveTenors.clear();
    m_curveZeroRates.clear();
    m_status.clear();

    const std::string date(m_buildDate);

    try
    {
        std::shared_ptr<DPP::IHttpClient> http;
        std::shared_ptr<DPP::IApiKeyProvider> keys;

        if (m_useStub)
        {
            http = std::make_shared<StubHttpClient>(makeStubFredValues());
            keys = std::make_shared<StubApiKeyProvider>();
        }
        else
        {
            http = std::make_shared<DPP::CurlHttpClient>();
            keys = std::make_shared<DPP::EnvApiKeyProvider>();
        }

        auto fred = std::make_shared<DPP::FredClient>(http, keys);
        auto av = std::make_shared<DPP::AlphaVantageClient>(http, keys);

        MarketDataService svc(fred, av);

        auto curveRes = svc.buildYieldCurve(date);
        if (!curveRes.has_value())
        {
            m_status = "FRED error @ " + date + " : " + curveRes.error();
            return;
        }

        m_curve = std::move(curveRes.value());
        DPPUI::g_lastBuiltYieldCurve = m_curve;
        m_hasCurve = true;
        m_curveTenors = m_curve->tenors();
        m_curveZeroRates = m_curve->zeroRates();
        refreshCurveAtT();

        m_status = "FRED OK: " + date;
    }
    catch (const std::exception& e)
    {
        m_status = std::string("Exception: ") + e.what();
    }
}

void ApiTesterState::fetchVolSurfaceFromAv()
{
    m_hasVol = false;
    m_status.clear();

    const std::string symbol(m_optionSymbol);

    try
    {
        std::shared_ptr<DPP::IHttpClient> http;
        std::shared_ptr<DPP::IApiKeyProvider> keys;

        if (m_useStub)
        {
            http = std::make_shared<StubHttpClient>(makeStubFredValues());
            keys = std::make_shared<StubApiKeyProvider>();
        }
        else
        {
            http = std::make_shared<DPP::CurlHttpClient>();
            keys = std::make_shared<DPP::EnvApiKeyProvider>();
        }

        auto fred = std::make_shared<DPP::FredClient>(http, keys);
        auto av = std::make_shared<DPP::AlphaVantageClient>(http, keys);
        MarketDataService svc(fred, av);

        auto volRes = svc.buildVolSurface(symbol);
        if (!volRes.has_value())
        {
            m_status = "AlphaVantage error (placeholder): " + volRes.error();
            return;
        }

        const auto& surf = volRes.value();
        m_hasVol = true;
        m_volAtPoint = surf.vol(m_avExpiryYears, m_avStrike);

        std::ostringstream oss;
        oss << "AV OK (placeholder): vol(" << m_avExpiryYears << ", " << m_avStrike << ") = " << m_volAtPoint;
        m_status = oss.str();
    }
    catch (const std::exception& e)
    {
        m_status = std::string("Exception: ") + e.what();
    }
}
