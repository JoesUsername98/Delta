#include "local_vol_surface_view.h"

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include <cstdio>
#include <ctime>

namespace
{
    bool parseYmd(const char* s, int& y, int& mo, int& d)
    {
        return std::sscanf(s, "%d-%d-%d", &y, &mo, &d) == 3;
    }

    void renderYmdDateRow(const char* label, char* buf, size_t bufSize)
    {
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
            static ImPlotTime s_pickerTime;
            static int s_pickerLevel = 0;

            if (ImGui::IsWindowAppearing())
            {
                int y = 0, mo = 0, d = 0;
                if (parseYmd(buf, y, mo, d))
                    s_pickerTime = ImPlot::MakeTime(y, mo - 1, d);
                else
                    s_pickerTime = ImPlot::Today();
                s_pickerLevel = 0;
            }

            if (ImPlot::ShowDatePicker("lv_dp", &s_pickerLevel, &s_pickerTime))
            {
                std::tm tm{};
                ImPlot::GetTime(s_pickerTime, &tm);
                std::snprintf(buf, bufSize, "%04d-%02d-%02d",
                              tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

LocalVolSurfaceWindow::LocalVolSurfaceWindow()
{
    m_state.refreshUnderlyings();
    m_state.refreshLastPrice();
}

void LocalVolSurfaceWindow::OnUIRender()
{
    ImGui::Begin("Local Vol Surface");

    if (ImGui::Button("Reload underlyings"))
    {
        m_state.refreshUnderlyings();
        m_state.refreshLastPrice();
    }

    renderYmdDateRow("AsOf (YYYY-MM-DD)", m_state.m_asof, sizeof(m_state.m_asof));
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        m_state.refreshUnderlyings();
        m_state.refreshLastPrice();
    }

    const auto& underlyings = m_state.underlyings();
    if (!underlyings.empty())
    {
        std::vector<const char*> items;
        items.reserve(underlyings.size());
        for (const auto& s : underlyings)
            items.push_back(s.c_str());

        ImGui::SetNextItemWidth(200);
        if (ImGui::Combo("Underlying", &m_state.m_underlyingIdx, items.data(), static_cast<int>(items.size())))
        {
            m_state.refreshLastPrice();
        }
    }
    else
    {
        ImGui::TextDisabled("Underlying: (none for this date)");
    }

    if (m_state.lastPrice().has_value())
        ImGui::Text("Last Price: %.6f", *m_state.lastPrice());
    else
        ImGui::TextDisabled("Last Price: (missing; load equities)");

    if (ImGui::Button("Bootstrap"))
    {
        m_state.refreshLastPrice();
        m_state.bootstrap();
    }

    const auto& data = m_state.data();
    ImGui::Separator();
    ImGui::Text("Points: %d", static_cast<int>(data.texp_years.size()));
    if (!m_state.status().empty())
        ImGui::TextWrapped("%s", m_state.status().c_str());

    if (!data.texp_years.empty())
    {
        if (ImGui::CollapsingHeader("Preview (first 25 rows)", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::BeginTable("lv_preview", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("T (years)");
                ImGui::TableSetupColumn("Strike");
                ImGui::TableSetupColumn("Call mid");
                ImGui::TableSetupColumn("IV");
                ImGui::TableHeadersRow();

                const int n = (std::min)(25, static_cast<int>(data.texp_years.size()));
                for (int i = 0; i < n; ++i)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%.6f", data.texp_years[i]);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.6f", data.strikes[i]);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.6f", data.call_mids[i]);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.6f", data.implied_vol[i]);
                }
                ImGui::EndTable();
            }
        }
    }

    ImGui::End();
}

