#include "api_tester_view.h"

#include <Delta++MarketAPI/fred_client.h>
#include <Delta++MarketAPI/alpha_vantage_client.h>
#include <Delta++Market/market_data_builder.h>

#include <algorithm>
#include <cmath>
#include <sstream>

using DPP::MarketDataService;

namespace
{
    // Canonical deterministic stub values for the standard FRED series map.
    // Values are annualised in percent (matching existing solver/test expectations).
    std::map<std::string, double> makeStubFredValues()
    {
        return {
            {"DGS1MO",  4.45},
            {"DGS3MO",  4.50},
            {"DGS6MO",  4.52},
            {"DGS1",    4.55},
            {"DGS2",    4.60},
            {"DGS5",    4.70},
            {"DGS10",   4.80},
            {"DGS30",   4.90},
        };
    }
}

// ------------------ ApiTesterWindow ------------------

ApiTesterWindow::ApiTesterWindow() = default;

void ApiTesterWindow::OnUIRender()
{
    ImGui::Begin("API Tester");

    m_status.clear();

    ImGui::Checkbox("Stub Mode", &m_useStub);
    ImGui::SameLine();
    ImGui::TextDisabled("%s", m_useStub ? "(no network)" : "(live network)");

    ImGui::Separator();

    renderFredSection();

    ImGui::Separator();

    renderAlphaVantageSection();

    ImGui::Separator();
    ImGui::TextWrapped("%s", m_status.c_str());

    ImGui::End();
}

void ApiTesterWindow::renderFredSection()
{
    if (!ImGui::CollapsingHeader("FRED Yield Curve (build + query)", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGui::InputText("Build Date (YYYY-MM-DD)", m_buildDate, sizeof(m_buildDate));
    ImGui::InputDouble("t (years)", &m_tYears, 0.01, 0.25, "%.2f");

    if (ImGui::Button("Fetch Yield Curve (FRED)"))
    {
        m_hasCurve = false;
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
            // AlphaVantage is unused for FRED curve build, but MarketDataService requires it.
            auto av = std::make_shared<DPP::AlphaVantageClient>(http, keys);

            DPP::MarketDataService svc(fred, av);

            auto curveRes = svc.buildYieldCurve(date);
            if (!curveRes.has_value())
            {
                m_status = "FRED error: " + curveRes.error();
                return;
            }

            const auto& curve = curveRes.value();
            m_hasCurve = true;
            m_curveZeroRateAtT = curve.zeroRate(m_tYears);
            m_curveDiscountAtT = curve.discount(m_tYears);
            m_curveTenors = curve.tenors();
            m_curveZeroRates = curve.zeroRates();

            std::ostringstream oss;
            oss << "FRED OK: zeroRate(t) = " << m_curveZeroRateAtT
                << ", discount(t) = " << m_curveDiscountAtT;
            m_status = oss.str();
        }
        catch (const std::exception& e)
        {
            m_status = std::string("Exception: ") + e.what();
        }
    }

    if (!m_hasCurve)
        return;

    ImGui::Text("zeroRate(t) = %.6f", m_curveZeroRateAtT);
    ImGui::Text("discount(t) = %.6f", m_curveDiscountAtT);

    if (ImGui::CollapsingHeader("Curve knots (tenor -> zeroRate)", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (m_curveTenors.size() == m_curveZeroRates.size())
        {
            if (ImGui::BeginTable("Knots", 2, ImGuiTableFlags_Borders))
            {
                ImGui::TableSetupColumn("Tenor (Y)");
                ImGui::TableSetupColumn("ZeroRate");
                ImGui::TableHeadersRow();

                for (size_t i = 0; i < m_curveTenors.size(); ++i)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%.6f", m_curveTenors[i]);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.6f", m_curveZeroRates[i]);
                }
                ImGui::EndTable();
            }
        }
    }
}

void ApiTesterWindow::renderAlphaVantageSection()
{
    if (!ImGui::CollapsingHeader("AlphaVantage Option Chain (placeholder)", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGui::InputText("Option Symbol", m_optionSymbol, sizeof(m_optionSymbol));
    ImGui::InputDouble("Expiry (years)", &m_avExpiryYears, 0.01, 0.25, "%.2f");
    ImGui::InputDouble("Strike", &m_avStrike, 0.25, 10.0, "%.2f");

    if (ImGui::Button("Fetch Vol Surface (AV placeholder)"))
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
            DPP::MarketDataService svc(fred, av);

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

    if (!m_hasVol)
        return;

    ImGui::Text("vol(expiry, strike) = %.8f", m_volAtPoint);
}

// ------------------ StubHttpClient ------------------

ApiTesterWindow::StubHttpClient::StubHttpClient(std::map<std::string, double> fredValues)
    : m_fredValues(std::move(fredValues))
{
}

std::string ApiTesterWindow::StubHttpClient::makeFredJson(const std::string& date, double value)
{
    std::ostringstream oss;
    oss << R"({"observations":[{"date":")" << date << R"(","value":")" << value << R"("}]})";
    return oss.str();
}

DPP::HttpResponse ApiTesterWindow::StubHttpClient::get(
    const std::string& /*url*/,
    const std::map<std::string, std::string>& params) const
{
    auto sidIt = params.find("series_id");
    if (sidIt == params.end())
    {
        // AlphaVantage placeholder path: AlphaVantageClient ignores JSON body right now.
        return std::string("{}");
    }

    const std::string seriesId = sidIt->second;

    auto dateIt = params.find("observation_start");
    if (dateIt == params.end())
    {
        dateIt = params.find("observation_end");
    }
    const std::string date = (dateIt != params.end()) ? dateIt->second : std::string("1970-01-01");

    auto vIt = m_fredValues.find(seriesId);
    if (vIt == m_fredValues.end())
        return std::unexpected("Stub: no value for FRED series_id '" + seriesId + "'");

    return makeFredJson(date, vIt->second);
}

// ------------------ StubApiKeyProvider ------------------

std::expected<std::string, std::string> ApiTesterWindow::StubApiKeyProvider::getKey(
    const std::string& /*name*/) const
{
    return std::string("stub_key");
}

