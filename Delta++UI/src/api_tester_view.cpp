#include "api_tester_view.h"

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include <cstdio>
#include <ctime>
#include <vector>

namespace
{
    constexpr int kDivisionsBetweenKnots = 10;

    bool parseYmd(const char* s, int& y, int& mo, int& d)
    {
        return std::sscanf(s, "%d-%d-%d", &y, &mo, &d) == 3;
    }

    void renderBuildDateRow(char* buildDate, size_t buildDateSize)
    {
        ImGui::PushID("yc_build_date");
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Build Date (YYYY-MM-DD)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(140);
        ImGui::InputText("##text", buildDate, buildDateSize);
        ImGui::SameLine();
        if (ImGui::Button("Pick date"))
            ImGui::OpenPopup("yc_date_picker");

        if (ImGui::BeginPopup("yc_date_picker"))
        {
            static ImPlotTime s_pickerTime;
            static int s_pickerLevel = 0;

            if (ImGui::IsWindowAppearing())
            {
                int y = 0;
                int mo = 0;
                int d = 0;
                if (parseYmd(buildDate, y, mo, d))
                    s_pickerTime = ImPlot::MakeTime(y, mo - 1, d);
                else
                    s_pickerTime = ImPlot::Today();
                s_pickerLevel = 0;
            }

            if (ImPlot::ShowDatePicker("yc_dp", &s_pickerLevel, &s_pickerTime))
            {
                std::tm tm{};
                ImPlot::GetTime(s_pickerTime, &tm);
                std::snprintf(buildDate, buildDateSize, "%04d-%02d-%02d",
                              tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    void sampleZeroRatesBetweenKnots(const DPP::YieldCurve& curve, std::vector<double>& outT,
                                     std::vector<double>& outZr)
    {
        outT.clear();
        outZr.clear();
        const auto& tenors = curve.tenors();
        if (tenors.size() < 2)
            return;

        for (size_t seg = 0; seg + 1 < tenors.size(); ++seg)
        {
            const double t0 = tenors[seg];
            const double t1 = tenors[seg + 1];
            for (int k = 0; k <= kDivisionsBetweenKnots; ++k)
            {
                if (seg > 0 && k == 0)
                    continue;
                const double t = t0 + (t1 - t0) * (static_cast<double>(k) / static_cast<double>(kDivisionsBetweenKnots));
                outT.push_back(t);
                outZr.push_back(curve.zeroRate(t));
            }
        }
    }
} // namespace

ApiTesterWindow::ApiTesterWindow() = default;

void ApiTesterWindow::OnUIRender()
{
    ImGui::Begin("API Tester");

    ImGui::TextUnformatted("Yield curve source");
    ImGui::SameLine();
    if (ImGui::RadioButton("Stub", m_state.m_yieldCurveSource == DPP::YieldCurveSource::Stub))
        m_state.m_yieldCurveSource = DPP::YieldCurveSource::Stub;
    ImGui::SameLine();
    if (ImGui::RadioButton("Massive", m_state.m_yieldCurveSource == DPP::YieldCurveSource::Massive))
        m_state.m_yieldCurveSource = DPP::YieldCurveSource::Massive;
    ImGui::SameLine();
    ImGui::TextDisabled("%s",
                        m_state.m_yieldCurveSource == DPP::YieldCurveSource::Stub ? "(no network)"
                                                                             : "(live network; MASSIVE_API_KEY)");

    ImGui::Separator();

    renderYieldCurveSection();

    ImGui::Separator();

    renderAlphaVantageSection();

    ImGui::Separator();
    ImGui::TextWrapped("%s", m_state.m_status.c_str());

    ImGui::End();
}

void ApiTesterWindow::renderYieldCurveSection()
{
    if (!ImGui::CollapsingHeader("Yield curve (Massive)", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    renderBuildDateRow(m_state.m_buildDate, sizeof(m_state.m_buildDate));
    if (ImGui::InputDouble("t (years)", &m_state.m_tYears, 0.01, 0.25, "%.2f"))
    {
        if (m_state.m_hasCurve && m_state.m_curve.has_value())
            m_state.refreshCurveAtT();
    }

    if (ImGui::Button("Fetch yield curve"))
        m_state.fetchYieldCurve();

    if (!m_state.m_hasCurve)
        return;

    ImGui::Text("zeroRate(t) = %.6f", m_state.m_curveZeroRateAtT);
    ImGui::Text("discount(t) = %.6f", m_state.m_curveDiscountAtT);

    if (ImGui::CollapsingHeader("Curve knots (tenor -> zeroRate)", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (m_state.m_curveTenors.size() == m_state.m_curveZeroRates.size())
        {
            if (ImGui::BeginTable("Knots", 2, ImGuiTableFlags_Borders))
            {
                ImGui::TableSetupColumn("Tenor (Y)");
                ImGui::TableSetupColumn("ZeroRate");
                ImGui::TableHeadersRow();

                for (size_t i = 0; i < m_state.m_curveTenors.size(); ++i)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%.6f", m_state.m_curveTenors[i]);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.6f", m_state.m_curveZeroRates[i]);
                }
                ImGui::EndTable();
            }
        }
    }

    renderZeroRatePlot();
}

void ApiTesterWindow::renderZeroRatePlot()
{
    if (!m_state.m_curve.has_value())
        return;

    if (!ImGui::CollapsingHeader("Zero rate curve (interpolated)", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    std::vector<double> plotT;
    std::vector<double> plotZr;
    sampleZeroRatesBetweenKnots(*m_state.m_curve, plotT, plotZr);
    if (plotT.size() < 2)
        return;

    if (ImPlot::BeginPlot("zero_rate_vs_tenor", ImVec2(-1, 220)))
    {
        ImPlot::SetupAxes("Tenor (years)", "Zero rate");
        ImPlot::PlotLine("zeroRate(t)", plotT.data(), plotZr.data(), static_cast<int>(plotT.size()));
        ImPlot::EndPlot();
    }
}

void ApiTesterWindow::renderAlphaVantageSection()
{
    if (!ImGui::CollapsingHeader("AlphaVantage Option Chain (placeholder)", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGui::InputText("Option Symbol", m_state.m_optionSymbol, sizeof(m_state.m_optionSymbol));
    ImGui::InputDouble("Expiry (years)", &m_state.m_avExpiryYears, 0.01, 0.25, "%.2f");
    ImGui::InputDouble("Strike", &m_state.m_avStrike, 0.25, 10.0, "%.2f");

    if (ImGui::Button("Fetch Vol Surface (AV placeholder)"))
        m_state.fetchVolSurfaceFromAv();

    if (!m_state.m_hasVol)
        return;

    ImGui::Text("vol(expiry, strike) = %.8f", m_state.m_volAtPoint);
}
