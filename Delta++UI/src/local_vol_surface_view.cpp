#include "local_vol_surface_view.h"

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>
#include <implot3d.h>

#include <cstdio>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <optional>

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

    // (3D grids and IV surface evaluation are cached in LocalVolSurfaceState at Bootstrap time.)
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

    ImGui::Separator();
    ImGui::TextDisabled("Yield curve: treasury_yields from MarketDB (cached in shared curve on bootstrap)");

    if (ImGui::Button("Bootstrap"))
    {
        m_state.refreshLastPrice();
        m_state.bootstrap();
    }

    ImGui::SameLine();
    if (ImGui::Button("Show IV 3D"))
        m_showIv3d = true;
    ImGui::SameLine();
    if (ImGui::Button("Show LocalVol 3D"))
        m_showLv3d = true;
    ImGui::SameLine();
    if (ImGui::Button("Show Call 3D"))
        m_showCall3d = true;

    const auto& data = m_state.data();
    const auto& surfOpt = m_state.surface();
    const bool haveSurf = surfOpt.has_value();
    const bool haveIv = !data.texp_years.empty();
    ImGui::Separator();
    ImGui::Text("Points: %d", static_cast<int>(data.texp_years.size()));
    if (!m_state.status().empty())
        ImGui::TextWrapped("%s", m_state.status().c_str());

    const auto& py = m_state.parityYields();
    if (!py.empty() && ImGui::CollapsingHeader("Implied dividend yield q(T) (put-call parity)", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("q(T) curve build time: %lld ms", m_state.parityCurveMs());

        // Plot q(T)
        std::vector<double> Ts;
        std::vector<double> qs;
        Ts.reserve(py.size());
        qs.reserve(py.size());
        for (const auto& row : py)
        {
            Ts.push_back(row.texp_years);
            qs.push_back(row.q);
        }
        if (Ts.size() >= 2 && ImPlot::BeginPlot("q(T)", ImVec2(-1, 220)))
        {
            ImPlot::SetupAxes("T (years)", "q");
            ImPlot::PlotLine("q", Ts.data(), qs.data(), static_cast<int>(Ts.size()));
            ImPlot::EndPlot();
        }

        if (ImGui::BeginTable("lv_parity_yields", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Expiry");
            ImGui::TableSetupColumn("T (years)");
            ImGui::TableSetupColumn("r");
            ImGui::TableSetupColumn("q");
            ImGui::TableSetupColumn("F");
            ImGui::TableSetupColumn("A");
            ImGui::TableSetupColumn("B");
            ImGui::TableSetupColumn("RMSE / n");
            ImGui::TableHeadersRow();

            for (const auto& row : py)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(row.expirationDate.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.6f", row.texp_years);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.6f", row.r);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.6f", row.q);
                ImGui::TableSetColumnIndex(4);
                if (std::isfinite(row.forward))
                    ImGui::Text("%.6f", row.forward);
                else
                    ImGui::TextDisabled("-");
                ImGui::TableSetColumnIndex(5);
                if (std::isfinite(row.A))
                    ImGui::Text("%.6f", row.A);
                else
                    ImGui::TextDisabled("-");
                ImGui::TableSetColumnIndex(6);
                if (std::isfinite(row.B))
                    ImGui::Text("%.6f", row.B);
                else
                    ImGui::TextDisabled("-");
                ImGui::TableSetColumnIndex(7);
                if (std::isfinite(row.rmse))
                    ImGui::Text("%.6g / %d", row.rmse, row.nUsed);
                else
                    ImGui::TextDisabled("- / %d", row.nUsed);
            }

            ImGui::EndTable();
        }
    }

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

    // -------- 2D slice plots (ImPlot) --------
    if (ImGui::CollapsingHeader("Slices (3M / 6M / 1Y)", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const auto& Kgrid = m_state.sliceK();
        const auto& Ts = m_state.sliceT();
        const auto& iv = m_state.sliceIv();
        const auto& lv = m_state.sliceLv();

        if (Kgrid.size() < 3 || Ts.empty())
        {
            ImGui::TextDisabled("No precomputed slices yet. Click Bootstrap first.");
        }
        else
        {
            for (size_t it = 0; it < Ts.size(); ++it)
            {
                const double Tsl = Ts[it];
                char title[64];
                std::snprintf(title, sizeof(title), "Slice T=%.2f", Tsl);
                if (ImPlot::BeginPlot(title, ImVec2(-1, 220)))
                {
                    ImPlot::SetupAxes("Strike (K)", "Vol");
                    if (it < iv.size() && iv[it].size() == Kgrid.size())
                        ImPlot::PlotLine("ImpliedVol", Kgrid.data(), iv[it].data(), static_cast<int>(Kgrid.size()));
                    if (it < lv.size() && lv[it].size() == Kgrid.size())
                        ImPlot::PlotLine("LocalVol", Kgrid.data(), lv[it].data(), static_cast<int>(Kgrid.size()));
                    ImPlot::EndPlot();
                }
            }
        }
    }

    // -------- 3D windows (ImPlot3D) --------
    const auto& ivGridOpt = m_state.ivGrid3d();
    const auto& lvGridOpt = m_state.lvGrid3d();
    const auto& callGridOpt = m_state.callGrid3d();

    if (m_showIv3d)
    {
        ImGui::Begin("Implied Vol Surface (3D)", &m_showIv3d);
        if (!ivGridOpt.has_value() || ivGridOpt->empty())
        {
            ImGui::TextDisabled("No cached IV surface yet. Click Bootstrap first.");
        }
        else
        {
            const auto& g = *ivGridOpt;
            if (ImPlot3D::BeginPlot("IV(T,K)", ImVec2(-1, -1)))
            {
                ImPlot3D::SetupAxes("Strike (K)", "Expiry (T)", "ImpliedVol");
                ImPlot3D::PlotSurface("IV", g.xs.data(), g.ys.data(), g.zs.data(), g.xCount, g.yCount);
                ImPlot3D::EndPlot();
            }
        }
        ImGui::End();
    }

    if (m_showLv3d)
    {
        ImGui::Begin("Local Vol Surface (3D)", &m_showLv3d);
        if (!lvGridOpt.has_value() || lvGridOpt->empty())
        {
            ImGui::TextDisabled("No cached local vol surface yet. Bootstrap must succeed (>=2 expiries, >=3 strikes/expiry).");
        }
        else
        {
            const auto& g = *lvGridOpt;
            if (ImPlot3D::BeginPlot("LocalVol(T,K)", ImVec2(-1, -1)))
            {
                ImPlot3D::SetupAxes("Strike (K)", "Expiry (T)", "LocalVol");
                ImPlot3D::PlotSurface("sigma_loc", g.xs.data(), g.ys.data(), g.zs.data(), g.xCount, g.yCount);
                ImPlot3D::EndPlot();
            }
        }
        ImGui::End();
    }

    if (m_showCall3d)
    {
        ImGui::Begin("Call Price Surface (3D)", &m_showCall3d);
        if (!callGridOpt.has_value() || callGridOpt->empty())
        {
            ImGui::TextDisabled("No cached call surface yet. Bootstrap must succeed.");
        }
        else
        {
            const auto& g = *callGridOpt;
            if (ImPlot3D::BeginPlot("C(T,K)", ImVec2(-1, -1)))
            {
                ImPlot3D::SetupAxes("Strike (K)", "Expiry (T)", "CallPrice");
                ImPlot3D::PlotSurface("C", g.xs.data(), g.ys.data(), g.zs.data(), g.xCount, g.yCount);
                ImPlot3D::EndPlot();
            }
        }
        ImGui::End();
    }

    ImGui::End();
}

