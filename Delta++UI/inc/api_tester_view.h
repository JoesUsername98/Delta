#pragma once

#include <Walnut/Layer.h>

#include <Delta++Market/market_data_service.h>
#include <Delta++MarketAPI/api_key_provider.h>
#include <Delta++MarketAPI/curl_http_client.h>
#include <Delta++MarketAPI/http_client.h>

#include <imgui.h>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <expected>

class ApiTesterWindow : public Walnut::Layer
{
public:
    ApiTesterWindow();

    void OnUIRender() override;

private:
    // --- Stub HTTP + key provider (used when checkbox is enabled) ---
    class StubHttpClient : public DPP::IHttpClient
    {
    public:
        explicit StubHttpClient(std::map<std::string, double> fredValues);

        DPP::HttpResponse get(const std::string& url,
                               const std::map<std::string, std::string>& params = {}) const override;

    private:
        std::map<std::string, double> m_fredValues;

        static std::string makeFredJson(const std::string& date, double value);
    };

    class StubApiKeyProvider : public DPP::IApiKeyProvider
    {
    public:
        std::expected<std::string, std::string> getKey(const std::string& /*name*/) const override;
    };

private:
    void renderFredSection();
    void renderAlphaVantageSection();

private:
    bool m_useStub = true;

    // FRED inputs
    char m_buildDate[11] = "2024-03-01";
    double m_tYears = 1.0;

    // AlphaVantage placeholder inputs
    char m_optionSymbol[32] = "AAPL";
    double m_avExpiryYears = 0.5;
    double m_avStrike = 100.0;

    // UI outputs
    std::string m_status;

    bool m_hasCurve = false;
    double m_curveZeroRateAtT = 0.0;
    double m_curveDiscountAtT = 1.0;
    std::vector<double> m_curveTenors;
    std::vector<double> m_curveZeroRates;

    bool m_hasVol = false;
    double m_volAtPoint = 0.0;
};

