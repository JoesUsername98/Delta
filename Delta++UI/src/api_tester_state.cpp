#include "api_tester_state.h"

#include <Delta++DB/market_db.h>
#include <Delta++Market/market_data_builder.h>
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

    // Doc-shaped stubs for options endpoints (offline mode).
    static constexpr const char* kStubOptionsContractsJson = R"({"request_id":"stub","results":[{"cfi":"OCASPS","contract_type":"call","exercise_style":"american","expiration_date":"2021-11-19","primary_exchange":"BATO","shares_per_contract":100,"strike_price":85,"ticker":"O:AAPL211119C00085000","underlying_ticker":"AAPL"}],"status":"OK"})";

    static constexpr const char* kStubOptionsAggsJson =
        R"({"adjusted":true,"count":2,"queryCount":2,"request_id":"stub","results":[{"c":26.2,"h":26.2,"l":26.2,"n":1,"o":26.2,"t":1632369600000,"v":2,"vw":26.2},{"c":28.3,"h":28.3,"l":28.3,"n":1,"o":28.3,"t":1632456000000,"v":2,"vw":28.3}],"resultsCount":2,"status":"OK","ticker":"O:RDFN211119C00025000"})";

    /// Stub HTTP: Massive treasury + options reference/aggs JSON by URL; minimal body otherwise (e.g. Alpha Vantage).
    class ApiTesterStubHttpClient : public DPP::IHttpClient
    {
    public:
        DPP::HttpResponse get(const std::string& url,
                              const std::map<std::string, std::string>& params) const override
        {
            if (url.find("massive.com") == std::string::npos)
                return std::string("{}");

            if (url.find("treasury-yields") != std::string::npos)
            {
                auto dateIt = params.find("date");
                const std::string date = (dateIt != params.end()) ? dateIt->second : std::string("1970-01-01");
                return makeMassiveTreasuryStubJson(date);
            }
            if (url.find("v3/reference/options/contracts") != std::string::npos)
                return std::string(kStubOptionsContractsJson);
            if (url.find("/v2/aggs/ticker/") != std::string::npos && url.find("/range/") != std::string::npos)
                return std::string(kStubOptionsAggsJson);
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
        const char* sourceTag =
            (m_yieldCurveSource == DPP::ApiTesterYieldCurveSource::Stub)     ? "Stub"
            : (m_yieldCurveSource == DPP::ApiTesterYieldCurveSource::MarketDb) ? "MarketDB"
                                                                               : "Massive";

        std::expected<DPP::YieldCurve, std::string> curveRes = std::unexpected(std::string("uninitialised"));

        if (m_yieldCurveSource == DPP::ApiTesterYieldCurveSource::MarketDb)
        {
            const auto dbPath = DPP::DB::Market::defaultMarketDbPath();
            const auto rowRes = DPP::DB::Market::queryTreasuryYieldRow(dbPath, date);
            if (!rowRes.has_value())
            {
                m_status = std::string(sourceTag) + " error @ " + date + " : " + rowRes.error();
                return;
            }
            if (!rowRes.value().has_value())
            {
                m_status = std::string(sourceTag) + " error @ " + date + " : no treasury_yields row";
                return;
            }

            const auto quotes = DPP::massiveTreasuryRowToRateQuotes(*rowRes.value());
            curveRes = DPP::YieldCurve::build(quotes);
        }
        else
        {
            std::shared_ptr<DPP::IHttpClient> http;
            std::shared_ptr<DPP::IApiKeyProvider> keys;

            if (m_yieldCurveSource == DPP::ApiTesterYieldCurveSource::Stub)
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
            curveRes = svc.buildYieldCurve(date);
        }

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

        if (m_yieldCurveSource == DPP::ApiTesterYieldCurveSource::Stub)
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

void ApiTesterState::fetchOptionsContracts()
{
    m_hasOptionsContracts = false;
    m_optionsContractsResult.reset();
    m_optionsContractsMsg.clear();

    try
    {
        std::shared_ptr<DPP::IHttpClient> http;
        std::shared_ptr<DPP::IApiKeyProvider> keys;

        if (m_yieldCurveSource == DPP::ApiTesterYieldCurveSource::Stub)
        {
            http = std::make_shared<ApiTesterStubHttpClient>();
            keys = std::make_shared<StubApiKeyProvider>();
        }
        else
        {
            http = std::make_shared<DPP::CurlHttpClient>();
            keys = std::make_shared<DPP::EnvApiKeyProvider>();
        }

        auto massive = std::make_shared<DPP::MassiveClient>(http, keys);

        std::map<std::string, std::string> q;
        if (m_ocUnderlying[0] != '\0')
            q["underlying_ticker"] = std::string(m_ocUnderlying);
        q["limit"] = std::to_string(m_ocLimit);
        if (m_ocExpiration[0] != '\0')
            q["expiration_date"] = std::string(m_ocExpiration);

        auto res = massive->getOptionsContracts(q);
        if (!res.has_value())
        {
            m_optionsContractsMsg = res.error();
            return;
        }

        m_optionsContractsResult = std::move(res.value());
        m_hasOptionsContracts = true;
        m_optionsContractsMsg = "OK: " + std::to_string(m_optionsContractsResult->results.size()) + " contract(s)";
    }
    catch (const std::exception& e)
    {
        m_optionsContractsMsg = std::string("Exception: ") + e.what();
    }
}

void ApiTesterState::fetchOptionsAggregates()
{
    m_hasOptionsAggs = false;
    m_optionsAggsResult.reset();
    m_optionsAggsMsg.clear();

    try
    {
        std::shared_ptr<DPP::IHttpClient> http;
        std::shared_ptr<DPP::IApiKeyProvider> keys;

        if (m_yieldCurveSource == DPP::ApiTesterYieldCurveSource::Stub)
        {
            http = std::make_shared<ApiTesterStubHttpClient>();
            keys = std::make_shared<StubApiKeyProvider>();
        }
        else
        {
            http = std::make_shared<DPP::CurlHttpClient>();
            keys = std::make_shared<DPP::EnvApiKeyProvider>();
        }

        auto massive = std::make_shared<DPP::MassiveClient>(http, keys);

        DPP::OptionsAggregatesQuery oq;
        if (m_oaSendAdjusted)
            oq.adjusted = m_oaAdjusted;
        if (m_oaSort[0] != '\0')
            oq.sort = std::string(m_oaSort);
        if (m_oaLimitQuery > 0)
            oq.limit = m_oaLimitQuery;

        auto res = massive->getOptionsAggregates(std::string(m_oaOptionsTicker), m_oaMultiplier, m_oaTimespan, m_oaFrom,
                                                  m_oaTo, oq);
        if (!res.has_value())
        {
            m_optionsAggsMsg = res.error();
            return;
        }

        m_optionsAggsResult = std::move(res.value());
        m_hasOptionsAggs = true;
        m_optionsAggsMsg = "OK: " + std::to_string(m_optionsAggsResult->results.size()) + " bar(s)";
    }
    catch (const std::exception& e)
    {
        m_optionsAggsMsg = std::string("Exception: ") + e.what();
    }
}
