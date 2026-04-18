#include "pricer_view.h"

#include <Delta++DB/market_db.h>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>
#include <implot3d.h>

#include <magic_enum/magic_enum.hpp>

#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <ranges>
#include <vector>

size_t PricerView::s_type_count = 0;

namespace
{
    constexpr int kDivisionsBetweenKnots = 10;
    constexpr int kPricerYmdSlot = 10;

    bool parseYmd(const char* s, int& y, int& mo, int& d)
    {
        return std::sscanf(s, "%d-%d-%d", &y, &mo, &d) == 3;
    }

    void renderYmdDateRowPricer(const char* label, char* buf, size_t bufSize)
    {
        ImGui::PushID(kPricerYmdSlot);
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

            if (ImPlot::ShowDatePicker("pricer_dp", &s_pickerLevel, &s_pickerTime))
            {
                std::tm tm{};
                ImPlot::GetTime(s_pickerTime, &tm);
                std::snprintf(buf, bufSize, "%04d-%02d-%02d",
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
                const double t =
                    t0 + (t1 - t0) * (static_cast<double>(k) / static_cast<double>(kDivisionsBetweenKnots));
                outT.push_back(t);
                outZr.push_back(curve.zeroRate(t));
            }
        }
    }
} // namespace

void PricerView::OnUIRender()
{
    ImGui::Begin(M_NAME.c_str());

    renderMarketParams();
    renderTradeParams();
    renderCalcParams();

    m_state.recalcIfRequired();

    renderResults();

    renderVol3DWindows();

    m_state.reset();
    ImGui::End();
}

void PricerView::renderTradeParams()
{
    if (ImGui::CollapsingHeader("Trade", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (!m_state.m_equityTickers.empty())
        {
            std::vector<const char*> items;
            items.reserve(m_state.m_equityTickers.size());
            for (const auto& s : m_state.m_equityTickers)
                items.push_back(s.c_str());
            ImGui::SetNextItemWidth(220);
            if (ImGui::Combo("Underlying", &m_state.m_underlyingIdx, items.data(),
                             static_cast<int>(items.size())))
            {
                m_state.syncTradeUnderlyingTicker();
                m_state.m_valueChanged = true;
            }
        }
        else
        {
            ImGui::TextDisabled("Underlying: (no tickers in equities table)");
        }

        m_state.m_valueChanged |=
            ImGui::Combo("Exercise", &m_state.m_exerciseTypeIdx, m_state.m_exerciseCombo.m_keysCArray,
                         IM_ARRAYSIZE(m_state.m_exerciseCombo.m_keysCArray));
        ImGui::SameLine();
        HelpMarker("Europeans exercises at maturity while Americans can exercise at any timestep");
        m_state.m_trd.m_optionExerciseType = static_cast<DPP::OptionExerciseType>(m_state.m_exerciseTypeIdx);

        m_state.m_valueChanged |=
            ImGui::Combo("Payoff", &m_state.m_optionPayoffIdx, m_state.m_payoffCombo.m_keysCArray,
                         IM_ARRAYSIZE(m_state.m_payoffCombo.m_keysCArray));
        ImGui::SameLine();
        HelpMarker("Calls pay off when above the strike and Puts payoff when below");
        m_state.m_trd.m_optionPayoffType = static_cast<DPP::OptionPayoffType>(m_state.m_optionPayoffIdx);

        m_state.m_valueChanged |=
            ImGui::InputDouble("Maturity (Y)", &m_state.m_trd.m_maturity, 0.25, 1.0, "%.2f");
        ImGui::SameLine();
        HelpMarker("Time to maturity in years");

        m_state.m_valueChanged |= ImGui::InputDouble("Strike Price", &m_state.m_trd.m_strike, 0.25, 1.0, "%.2f");
        ImGui::SameLine();
        HelpMarker("Strike Price");
    }
}

void PricerView::renderMarketParams()
{
    if (!ImGui::CollapsingHeader("Market", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGui::PushID(static_cast<int>(M_ID));

    renderYmdDateRowPricer("As-of (YYYY-MM-DD)", m_state.m_asof, sizeof(m_state.m_asof));
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        m_state.refreshEquityTickers();
        m_state.m_valueChanged = true;
    }

    if (ImGui::Button("Reload equity tickers"))
    {
        m_state.refreshEquityTickers();
        m_state.m_valueChanged = true;
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Spot");
    ImGui::SameLine();
    m_state.m_valueChanged |=
        ImGui::InputDouble("Underlying Value", &m_state.m_mkt.m_underlyingPrice, 0.01, 1.0, "%.2f");
    ImGui::SameLine();
    HelpMarker("Spot used for pricing and for parity / local vol when loading from the DB");
    ImGui::SameLine();
    if (ImGui::Button("Load latest price"))
    {
        m_state.m_marketError.clear();
        if (m_state.m_trd.m_underlyingTicker.empty())
            m_state.m_marketError = "Select an underlying in the Trade section first.";
        else
        {
            const auto path = DPP::DB::Market::defaultMarketDbPath();
            auto px = DPP::DB::Market::queryEquityLast(path, m_state.m_asof, m_state.m_trd.m_underlyingTicker);
            if (!px.has_value())
                m_state.m_marketError = px.error();
            else if (!px->has_value())
                m_state.m_marketError =
                    "No equity last price for " + m_state.m_trd.m_underlyingTicker + " on " + m_state.m_asof;
            else
            {
                const double last = **px;
                m_state.m_mkt.m_underlyingPrice = last;
                m_state.m_trd.m_strike = last;
                m_state.m_marketStatus = "Loaded equity last for " + m_state.m_trd.m_underlyingTicker
                    + "; strike set to spot (ATM)";
                m_state.m_valueChanged = true;
            }
        }
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Interest rate (yield curve)");
    if (ImGui::RadioButton("Flat rate stub", m_state.m_rateSource == PricerRateSource::FlatStub))
    {
        m_state.m_rateSource = PricerRateSource::FlatStub;
        m_state.applyFlatYieldCurve();
        m_state.m_valueChanged = true;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("DB yield curve", m_state.m_rateSource == PricerRateSource::MarketDb))
    {
        m_state.m_rateSource = PricerRateSource::MarketDb;
        m_state.m_valueChanged = true;
    }

    if (m_state.m_rateSource == PricerRateSource::FlatStub)
    {
        if (ImGui::InputDouble("Flat zero rate (stub)", &m_state.m_flatRateStub, 0.005, 0.05, "%.4f"))
        {
            m_state.applyFlatYieldCurve();
            m_state.m_valueChanged = true;
        }
    }
    else
    {
        if (ImGui::Button("Load treasury curve from DB"))
        {
            if (m_state.tryLoadTreasuryYieldFromDb())
                m_state.m_valueChanged = true;
        }

        if (!m_state.m_yieldCurveTenors.empty() && m_state.m_yieldCurveTenors.size() == m_state.m_yieldCurveZeroRates.size())
        {
            if (ImGui::CollapsingHeader("Curve knots (tenor -> zeroRate)", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::BeginTable("yc_knots", 2, ImGuiTableFlags_Borders))
                {
                    ImGui::TableSetupColumn("Tenor (Y)");
                    ImGui::TableSetupColumn("ZeroRate");
                    ImGui::TableHeadersRow();
                    for (size_t i = 0; i < m_state.m_yieldCurveTenors.size(); ++i)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%.6f", m_state.m_yieldCurveTenors[i]);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.6f", m_state.m_yieldCurveZeroRates[i]);
                    }
                    ImGui::EndTable();
                }
            }

            if (ImGui::CollapsingHeader("Zero rate curve (interpolated)", ImGuiTreeNodeFlags_DefaultOpen))
            {
                std::vector<double> plotT;
                std::vector<double> plotZr;
                sampleZeroRatesBetweenKnots(m_state.m_mkt.m_yieldCurve, plotT, plotZr);
                if (plotT.size() >= 2)
                {
                    if (m_state.m_yieldCurvePlotEpoch != m_lastYieldCurvePlotEpoch)
                    {
                        ImPlot::SetNextAxesToFit();
                        m_lastYieldCurvePlotEpoch = m_state.m_yieldCurvePlotEpoch;
                    }
                    if (ImPlot::BeginPlot("zeroRatePlot", ImVec2(-1, 220)))
                    {
                        ImPlot::SetupAxes("Tenor (years)", "Zero rate");
                        ImPlot::PlotLine("zeroRate", plotT.data(), plotZr.data(), static_cast<int>(plotT.size()));
                        ImPlot::EndPlot();
                    }
                }
            }
        }
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Dividend yield");
    if (ImGui::RadioButton("Flat dividend stub##div", m_state.m_dividendSource == PricerDividendSource::FlatStub))
    {
        m_state.m_dividendSource = PricerDividendSource::FlatStub;
        m_state.applyFlatDividendCurve();
        m_state.m_valueChanged = true;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("DB implied (parity)##div", m_state.m_dividendSource == PricerDividendSource::MarketDbImplied))
    {
        m_state.m_dividendSource = PricerDividendSource::MarketDbImplied;
        m_state.m_valueChanged = true;
    }

    if (m_state.m_dividendSource == PricerDividendSource::FlatStub)
    {
        if (ImGui::InputDouble("Flat q", &m_state.m_flatDividendStub, 0.001, 0.01, "%.4f"))
        {
            m_state.applyFlatDividendCurve();
            m_state.m_valueChanged = true;
        }
    }
    else
    {
        if (ImGui::Button("Load implied dividend from DB"))
        {
            if (m_state.tryLoadImpliedDividendFromDb())
                m_state.m_valueChanged = true;
        }

        const auto& Ts = m_state.m_mkt.m_dividendYieldCurve.tenors();
        const auto& qs = m_state.m_mkt.m_dividendYieldCurve.qKnots();
        if (!Ts.empty() && Ts.size() == qs.size())
        {
            if (ImGui::CollapsingHeader("Implied dividend yield q(T)", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (m_state.m_dividendPlotEpoch != m_lastDividendPlotEpoch)
                {
                    ImPlot::SetNextAxesToFit();
                    m_lastDividendPlotEpoch = m_state.m_dividendPlotEpoch;
                }
                if (ImPlot::BeginPlot("div_q_plot", ImVec2(-1, 220)))
                {
                    ImPlot::SetupAxes("T (years)", "q");
                    const int n = static_cast<int>(Ts.size());
                    if (n >= 2)
                        ImPlot::PlotLine("q", Ts.data(), qs.data(), n);
                    else
                        ImPlot::PlotScatter("q", Ts.data(), qs.data(), n);
                    ImPlot::EndPlot();
                }
            }
        }
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Volatility");
    if (ImGui::RadioButton("Flat vol##vol", m_state.m_volSource == PricerVolSource::Flat))
    {
        m_state.m_volSource = PricerVolSource::Flat;
        m_state.m_mkt.m_localVolSurface.reset();
        m_state.m_valueChanged = true;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("DB local vol (AH bootstrap)##vol", m_state.m_volSource == PricerVolSource::LocalVolBootstrap))
    {
        m_state.m_volSource = PricerVolSource::LocalVolBootstrap;
        m_state.m_valueChanged = true;
    }

    if (m_state.m_volSource == PricerVolSource::Flat)
    {
        m_state.m_valueChanged |= ImGui::InputDouble("Volatility", &m_state.m_mkt.m_vol, 0.01, 1.0, "%.2f");
        ImGui::SameLine();
        HelpMarker("Constant Black–Scholes volatility when no local vol surface is attached");
    }
    else
    {
        if (ImGui::Button("Bootstrap local vol surface"))
        {
            if (m_state.tryBootstrapLocalVolSurface())
                m_state.m_valueChanged = true;
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

        const auto& surf = m_state.m_lvState.surface();
        if (surf.has_value() && m_state.m_trd.m_maturity > 0.0)
        {
            const auto& ivIn = m_state.m_lvState.localVolInputData();
            std::vector<double> kUnique = ivIn.strikes;
            std::sort(kUnique.begin(), kUnique.end());
            kUnique.erase(std::unique(kUnique.begin(), kUnique.end()), kUnique.end());
            if (kUnique.size() >= 2)
            {
                std::vector<double> lvLine(kUnique.size());
                const double Topt = m_state.m_trd.m_maturity;
                for (size_t i = 0; i < kUnique.size(); ++i)
                    lvLine[i] = surf->localVol(Topt, kUnique[i]);

                if (ImGui::CollapsingHeader("Local vol slice at option maturity", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (ImPlot::BeginPlot("lv_maturity_slice", ImVec2(-1, 220)))
                    {
                        ImPlot::SetupAxes("Strike (K)", "Local vol");
                        ImPlot::PlotLine("sigma_loc", kUnique.data(), lvLine.data(), static_cast<int>(kUnique.size()));
                        ImPlot::EndPlot();
                    }
                }
            }
        }
    }

    if (!m_state.m_marketStatus.empty())
        ImGui::TextWrapped("%s", m_state.m_marketStatus.c_str());
    if (!m_state.m_marketError.empty())
        ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f), "%s", m_state.m_marketError.c_str());

    ImGui::PopID();
}

void PricerView::renderCalcParams()
{
    if (ImGui::CollapsingHeader("Calculation", ImGuiTreeNodeFlags_DefaultOpen))
    {
        m_state.m_valueChanged |= ImGui::Checkbox("Dynamic Recalculation", &m_state.m_dynamicRecalc);

        m_state.m_valueChanged |=
            ImGui::Combo("Engine", &m_state.m_calculationMethodIdx, m_state.m_calculationMethodCombo.m_keysCArray,
                         IM_ARRAYSIZE(m_state.m_calculationMethodCombo.m_keysCArray));
        ImGui::SameLine();
        HelpMarker("Engines determine the calculation techinique");

        m_state.m_calculationMethod = static_cast<DPP::CalculationMethod>(m_state.m_calculationMethodIdx);
        if (m_state.m_calculationMethod != DPP::CalculationMethod::BlackScholes)
            m_state.m_valueChanged |= ImGui::InputInt("Time Periods", &m_state.m_steps);
        ImGui::SameLine();
        HelpMarker("Number of Nodes / Time steps");

        if (m_state.m_calculationMethod == DPP::CalculationMethod::MonteCarlo)
        {
            m_state.m_valueChanged |= ImGui::InputInt("Sims", &m_state.m_sims);
            ImGui::SameLine();
            HelpMarker("Number of simulations");
            m_state.m_valueChanged |=
                ImGui::Combo("Sim Scheme", &m_state.m_pathSchemeComboIdx, m_state.m_pathSchemeCombo.m_keysCArray,
                             IM_ARRAYSIZE(m_state.m_pathSchemeCombo.m_keysCArray));
            ImGui::SameLine();
            HelpMarker("Scheme dictates how price evolves in Monte Carlo Sim");
            m_state.m_valueChanged |= ImGui::InputInt("Seed", &m_state.m_seed);
            ImGui::SameLine();
            HelpMarker("Random seed used for Monte Carlo path generation");
        }

        m_state.m_valueChanged |= ImGui::Checkbox("PV", &m_state.m_calcsToDo[(int)Calculation::PV]);
        ImGui::SameLine();
        HelpMarker("Present Value of the option");
        m_state.m_valueChanged |= ImGui::Checkbox("Delta", &m_state.m_calcsToDo[(int)Calculation::Delta]);
        ImGui::SameLine();
        HelpMarker("The option value's sensitivity to the underlying's price");
        m_state.m_valueChanged |= ImGui::Checkbox("Gamma", &m_state.m_calcsToDo[(int)Calculation::Gamma]);
        ImGui::SameLine();
        HelpMarker("The option value's sensitivity to the underlying's delta");
        m_state.m_valueChanged |= ImGui::Checkbox("Vega", &m_state.m_calcsToDo[(int)Calculation::Vega]);
        ImGui::SameLine();
        HelpMarker("The option value's sensitivity to the underlying's volatility");
        m_state.m_valueChanged |= ImGui::Checkbox("Rho (key-rate)", &m_state.m_calcsToDo[(int)Calculation::Rho]);
        ImGui::SameLine();
        HelpMarker("Key-rate rhos: one rho per curve knot (tenor).");
        m_state.m_valueChanged |=
            ImGui::Checkbox("Rho (parallel)", &m_state.m_calcsToDo[(int)Calculation::RhoParallel]);
        ImGui::SameLine();
        HelpMarker("Parallel-shift rho: shifts the entire curve up/down.");

        if (!m_state.m_dynamicRecalc)
            m_state.m_btn_calcPressed = ImGui::Button("Calculate");
    }
}

void PricerView::renderMCPathsPlot()
{
    if (!m_state.m_engine || !m_state.m_engine->m_debugResults.contains(DPP::DebugInfo::MCPaths))
        return;

    const auto pvIt = std::ranges::find_if(m_state.m_calcs, [](const CalcData& c) { return c.m_calc == Calculation::PV; });
    if (pvIt == m_state.m_calcs.end())
        return;

    const auto& pvCalc = *pvIt;
    const auto& lines = m_state.m_engine->m_debugResults.at(DPP::DebugInfo::MCPaths);

    const std::string mcWindowTitle = std::string("Monte Carlo Paths ") + std::to_string(M_ID);
    ImGui::Begin(mcWindowTitle.c_str());

    const std::string mcPlotTitle = std::string("Monte Carlo Paths: ")
        + m_state.m_pathSchemeCombo.m_keysCArray[m_state.m_pathSchemeComboIdx]
        + "  seed=" + std::to_string(m_state.m_seed);

    if (ImPlot::BeginPlot(mcPlotTitle.c_str()))
    {
        std::vector<double> time_axis(pvCalc.m_steps);
        const double dt = m_state.m_trd.m_maturity / static_cast<double>(pvCalc.m_steps);
        for (size_t i = 0; i < pvCalc.m_steps; ++i)
            time_axis[i] = static_cast<double>(i) * dt;

        for (size_t s = 0; s < pvCalc.m_sims; ++s)
        {
            const std::string label = std::string("Path ") + std::to_string(s);
            const double* path_ptr = lines.data() + s * pvCalc.m_steps;
            bool already_exist = ImPlot::GetItem(label.c_str()) != nullptr;

            ImPlot::PlotLine(
                label.c_str(),
                time_axis.data(),
                path_ptr,
                static_cast<int>(pvCalc.m_steps)
            );

            bool just_created = false;
            ImPlotItem* item = ImPlot::RegisterOrGetItem(label.c_str(), 0, &just_created);

            bool set_to_hidden_initially = !already_exist && s > 5 && item->Show;
            if (set_to_hidden_initially) // Only show the first 5 paths for clarity
                item->Show = false;
        }
        ImPlot::EndPlot();
    }

    ImGui::End();
}

void PricerView::renderVol3DWindows()
{
    const auto& ivGridOpt = m_state.m_lvState.ivGrid3d();
    const auto& lvGridOpt = m_state.m_lvState.lvGrid3d();
    const auto& callGridOpt = m_state.m_lvState.callGrid3d();

    if (m_showIv3d)
    {
        ImGui::Begin("Implied Vol Surface (3D)", &m_showIv3d);
        if (!ivGridOpt.has_value() || ivGridOpt->empty())
            ImGui::TextDisabled("No cached IV surface yet. Bootstrap local vol first.");
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
            ImGui::TextDisabled("No cached local vol surface yet.");
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
            ImGui::TextDisabled("No cached call surface yet.");
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
}

void PricerView::renderResults()
{
    if (!m_state.m_engineBuildError.empty())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Engine Build Error: %s", m_state.m_engineBuildError.c_str());
        return;
    }

    renderMCPathsPlot();

    if (ImGui::BeginTable("Results", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV))
    {
        for (const auto& calc : m_state.m_calcs)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", magic_enum::enum_name(calc.m_calc).data());
            ImGui::TableSetColumnIndex(1);
            if (m_state.m_engine->m_results.contains(calc.m_calc))
            {
                const auto& res = m_state.m_engine->m_results.at(calc.m_calc);
                if (res.has_value())
                {
                    if (const auto* v = std::get_if<double>(&res.value()))
                    {
                        ImGui::Text("%.6f", *v);
                    }
                    else if (const auto* curve = std::get_if<DPP::CurveRho>(&res.value()))
                    {
                        if (ImGui::BeginTable("CurveRho", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV))
                        {
                            ImGui::TableSetupColumn("Tenor (Y)");
                            ImGui::TableSetupColumn("Rho");
                            ImGui::TableHeadersRow();
                            for (const auto& p : *curve)
                            {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text("%.6f", p.tenor);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::Text("%.6f", p.rho);
                            }
                            ImGui::EndTable();
                        }
                    }
                }
                else
                    ImGui::Text("%s", res.error().c_str());
            }
            else
                ImGui::Text("No error nor result!");
        }
        ImGui::EndTable();
    }

    if (m_state.m_timeTaken.has_value())
        ImGui::Text("Time Taken: %i ms", m_state.m_timeTaken.value());
}
