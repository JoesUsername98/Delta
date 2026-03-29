#include "api_tester_state.h"

#include <Delta++Market/market_data_service.h>
#include <Delta++MarketAPI/alpha_vantage_client.h>
#include <Delta++MarketAPI/api_key_provider.h>
#include <Delta++MarketAPI/curl_http_client.h>
#include <Delta++MarketAPI/http_client.h>
#include <Delta++MarketAPI/massive_client.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "shared_curve_cache.h"

using DPP::MarketDataService;

namespace
{
    std::string makeMassiveTreasuryStubJson(const std::string& date)
    {
        // Fixed stub yields (percent) aligned with former test curve shape.
        constexpr double y1mo = 4.45;
        constexpr double y3mo = 4.50;
        constexpr double y6mo = 4.52;
        constexpr double y1y = 4.55;
        constexpr double y2y = 4.60;
        constexpr double y5y = 4.70;
        constexpr double y10y = 4.80;
        constexpr double y30y = 4.90;

        std::ostringstream oss;
        oss << R"({"status":"OK","request_id":1,"count":1,"results":[{"date":")" << date << R"(",)";
        oss << R"("yield_1_month":)" << y1mo << R"(,"yield_3_month":)" << y3mo
            << R"(,"yield_6_month":)" << y6mo << R"(,"yield_1_year":)" << y1y
            << R"(,"yield_2_year":)" << y2y << R"(,"yield_5_year":)" << y5y
            << R"(,"yield_10_year":)" << y10y << R"(,"yield_30_year":)" << y30y;
        oss << "}]}";
        return oss.str();
    }

    /// Stub HTTP: Massive treasury JSON for treasury-yields URL; minimal body otherwise (e.g. Alpha Vantage).
    class ApiTesterStubHttpClient : public DPP::IHttpClient
    {
    public:
        DPP::HttpResponse get(const std::string& url,
                              const std::map<std::string, std::string>& params) const override
        {
            if (url.find("massive.com") != std::string::npos && url.find("treasury-yields") != std::string::npos)
            {
                auto dateIt = params.find("date");
                const std::string date = (dateIt != params.end()) ? dateIt->second : std::string("1970-01-01");
                return makeMassiveTreasuryStubJson(date);
            }
            return std::string("{}");
        }
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

void ApiTesterState::fetchYieldCurve()
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

        if (m_yieldCurveSource == DPP::YieldCurveSource::Stub)
        {
            http = std::make_shared<ApiTesterStubHttpClient>();
            keys = std::make_shared<StubApiKeyProvider>();
        }
        else
        {
            http = std::make_shared<DPP::CurlHttpClient>();
            keys = std::make_shared<DPP::EnvApiKeyProvider>();
        }

        auto av = std::make_shared<DPP::AlphaVantageClient>(http, keys);
        auto massive = std::make_shared<DPP::MassiveClient>(http, keys);

        MarketDataService svc(av, massive);

        const std::expected<DPP::YieldCurve, std::string> curveRes = svc.buildYieldCurve(date);

        const char* sourceTag = (m_yieldCurveSource == DPP::YieldCurveSource::Stub) ? "Stub" : "Massive";

        if (!curveRes.has_value())
        {
            m_status = std::string(sourceTag) + " error @ " + date + " : " + curveRes.error();
            return;
        }

        m_curve = std::move(curveRes.value());
        DPPUI::g_lastBuiltYieldCurve = m_curve;
        m_hasCurve = true;
        m_curveTenors = m_curve->tenors();
        m_curveZeroRates = m_curve->zeroRates();
        refreshCurveAtT();

        m_status = std::string(sourceTag) + " OK: " + date;
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

        if (m_yieldCurveSource == DPP::YieldCurveSource::Stub)
        {
            http = std::make_shared<ApiTesterStubHttpClient>();
            keys = std::make_shared<StubApiKeyProvider>();
        }
        else
        {
            http = std::make_shared<DPP::CurlHttpClient>();
            keys = std::make_shared<DPP::EnvApiKeyProvider>();
        }

        auto av = std::make_shared<DPP::AlphaVantageClient>(http, keys);
        auto massive = std::make_shared<DPP::MassiveClient>(http, keys);
        MarketDataService svc(av, massive);

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
