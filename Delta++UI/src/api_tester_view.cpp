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
    /// One slot per YMD date picker row (must match number of renderYmdDateRow uses).
    constexpr int kYmdPickerSlotCount = 3;

    bool parseYmd(const char* s, int& y, int& mo, int& d)
    {
        return std::sscanf(s, "%d-%d-%d", &y, &mo, &d) == 3;
    }

    /// Calendar picker + text field for YYYY-MM-DD (ImGui ID stack makes popup unique per `slot`).
    void renderYmdDateRow(const char* label, char* buf, size_t bufSize, int slot)
    {
        IM_ASSERT(slot >= 0 && slot < kYmdPickerSlotCount);
        ImGui::PushID(slot);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(140);
        ImGui::InputText("##ymd", buf, bufSize);
        ImGui::SameLine();
        if (ImGui::Button("Pick date"))
            ImGui::OpenPopup("ymd_popup");

        if (ImGui::BeginPopup("ymd_popup"))
        {
            static ImPlotTime s_pickerTime[kYmdPickerSlotCount];
            static int s_pickerLevel[kYmdPickerSlotCount] = {};

            if (ImGui::IsWindowAppearing())
            {
                int y = 0;
                int mo = 0;
                int d = 0;
                if (parseYmd(buf, y, mo, d))
                    s_pickerTime[slot] = ImPlot::MakeTime(y, mo - 1, d);
                else
                    s_pickerTime[slot] = ImPlot::Today();
                s_pickerLevel[slot] = 0;
            }

            char dpId[32];
            std::snprintf(dpId, sizeof(dpId), "ymd_dp_%d", slot);
            if (ImPlot::ShowDatePicker(dpId, &s_pickerLevel[slot], &s_pickerTime[slot]))
            {
                std::tm tm{};
                ImPlot::GetTime(s_pickerTime[slot], &tm);
                std::snprintf(buf, bufSize, "%04d-%02d-%02d",
                              tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
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

    renderMassiveOptionsContractsSection();

    ImGui::Separator();

    renderMassiveOptionsAggregatesSection();

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

void ApiTesterWindow::renderMassiveOptionsContractsSection()
{
    if (!ImGui::CollapsingHeader("Massive options — all contracts (GET /v3/reference/options/contracts)",
                                 ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGui::PushID("massive_oc");
    ImGui::InputText("underlying_ticker", m_state.m_ocUnderlying, sizeof(m_state.m_ocUnderlying));
    ImGui::InputInt("limit", &m_state.m_ocLimit);
    if (m_state.m_ocLimit < 1)
        m_state.m_ocLimit = 1;
    if (m_state.m_ocLimit > 1000)
        m_state.m_ocLimit = 1000;
    renderYmdDateRow("expiration_date (optional, YYYY-MM-DD)", m_state.m_ocExpiration, sizeof(m_state.m_ocExpiration), 0);

    if (ImGui::Button("Fetch options contracts"))
        m_state.fetchOptionsContracts();

    ImGui::SameLine();
    if (ImGui::Button("Commit fetched rows to SQLite"))
        m_state.commitOptionsContractsToDb();

    ImGui::TextWrapped("%s", m_state.m_optionsContractsMsg.c_str());
    if (!m_state.m_optionsCommitDbMsg.empty())
        ImGui::TextWrapped("%s", m_state.m_optionsCommitDbMsg.c_str());

    if (m_state.m_hasOptionsContracts && m_state.m_optionsContractsResult.has_value())
    {
        const auto& env = *m_state.m_optionsContractsResult;
        if (ImGui::BeginTable("oc_rows", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, ImVec2(0, 180)))
        {
            ImGui::TableSetupColumn("ticker");
            ImGui::TableSetupColumn("underlying");
            ImGui::TableSetupColumn("expiration");
            ImGui::TableSetupColumn("strike");
            ImGui::TableSetupColumn("type");
            ImGui::TableSetupColumn("exercise");
            ImGui::TableHeadersRow();
            for (const auto& row : env.results)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(row.ticker.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(row.underlying_ticker.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(row.expiration_date.c_str());
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.4f", row.strike_price.value_or(0.0));
                ImGui::TableSetColumnIndex(4);
                ImGui::TextUnformatted(row.contract_type.c_str());
                ImGui::TableSetColumnIndex(5);
                ImGui::TextUnformatted(row.exercise_style.value_or(std::string("")).c_str());
            }
            ImGui::EndTable();
        }
        if (env.next_url.has_value() && !env.next_url->empty())
            ImGui::TextDisabled("next_url: %s", env.next_url->c_str());
    }
    ImGui::PopID();
}

void ApiTesterWindow::renderMassiveOptionsAggregatesSection()
{
    if (!ImGui::CollapsingHeader("Massive options — custom bars (GET /v2/aggs/ticker/.../range/...)",
                                 ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGui::PushID("massive_oa");
    ImGui::InputText("options_ticker", m_state.m_oaOptionsTicker, sizeof(m_state.m_oaOptionsTicker));
    ImGui::InputInt("multiplier", &m_state.m_oaMultiplier);
    if (m_state.m_oaMultiplier < 1)
        m_state.m_oaMultiplier = 1;
    ImGui::InputText("timespan", m_state.m_oaTimespan, sizeof(m_state.m_oaTimespan));
    renderYmdDateRow("from (YYYY-MM-DD or per API)", m_state.m_oaFrom, sizeof(m_state.m_oaFrom), 1);
    renderYmdDateRow("to (YYYY-MM-DD or per API)", m_state.m_oaTo, sizeof(m_state.m_oaTo), 2);

    ImGui::Checkbox("Send adjusted query param", &m_state.m_oaSendAdjusted);
    if (m_state.m_oaSendAdjusted)
    {
        ImGui::Indent();
        ImGui::Checkbox("adjusted=true (uncheck for false)", &m_state.m_oaAdjusted);
        ImGui::Unindent();
    }
    ImGui::InputText("sort (optional: asc / desc)", m_state.m_oaSort, sizeof(m_state.m_oaSort));
    ImGui::InputInt("limit (0 = omit)", &m_state.m_oaLimitQuery);
    if (m_state.m_oaLimitQuery < 0)
        m_state.m_oaLimitQuery = 0;

    if (ImGui::Button("Fetch options aggregates"))
        m_state.fetchOptionsAggregates();

    ImGui::TextWrapped("%s", m_state.m_optionsAggsMsg.c_str());

    if (m_state.m_hasOptionsAggs && m_state.m_optionsAggsResult.has_value())
    {
        const auto& env = *m_state.m_optionsAggsResult;
        if (env.ticker.has_value())
            ImGui::TextDisabled("ticker: %s", env.ticker->c_str());

        if (ImGui::BeginTable("oa_bars", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, ImVec2(0, 200)))
        {
            ImGui::TableSetupColumn("t (ms)");
            ImGui::TableSetupColumn("o");
            ImGui::TableSetupColumn("h");
            ImGui::TableSetupColumn("l");
            ImGui::TableSetupColumn("c");
            ImGui::TableSetupColumn("v");
            ImGui::TableSetupColumn("vw");
            ImGui::TableSetupColumn("n");
            ImGui::TableHeadersRow();
            for (const auto& bar : env.results)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%lld", static_cast<long long>(bar.t.value_or(0)));
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.4f", bar.o.value_or(0.0));
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.4f", bar.h.value_or(0.0));
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.4f", bar.l.value_or(0.0));
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%.4f", bar.c.value_or(0.0));
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%.4f", bar.v.value_or(0.0));
                ImGui::TableSetColumnIndex(6);
                ImGui::Text("%.4f", bar.vw.value_or(0.0));
                ImGui::TableSetColumnIndex(7);
                ImGui::Text("%lld", static_cast<long long>(bar.n.value_or(0)));
            }
            ImGui::EndTable();
        }
    }
    ImGui::PopID();
}
