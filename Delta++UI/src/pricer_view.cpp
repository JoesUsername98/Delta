#include "pricer_view.h"
#include <implot.h>
#include <implot_internal.h>
#include <string>


size_t PricerView::s_type_count = 0;

void PricerView::OnUIRender()
{
    ImGui::Begin( M_NAME.c_str() );

    renderTradeParams();
    renderMarketParams();
    renderCalcParams();

    m_state.recalcIfRequired();

    renderResults();

    m_state.reset();
    ImGui::End();

}

void PricerView::renderTradeParams() 
{
    if( ImGui::CollapsingHeader("Trade", ImGuiTreeNodeFlags_DefaultOpen) )
    {
            m_state.m_valueChanged |=	ImGui::Combo("Exercise", &m_state.m_exerciseTypeIdx, m_state.m_exerciseCombo.m_keysCArray, IM_ARRAYSIZE(m_state.m_exerciseCombo.m_keysCArray));
                ImGui::SameLine(); HelpMarker("Europeans exercises at maturity while Americans can exercise at any timestep");
            m_state.m_trd.m_optionExerciseType = static_cast<DPP::OptionExerciseType>(m_state.m_exerciseTypeIdx);

            m_state.m_valueChanged |=	ImGui::Combo("Payoff", &m_state.m_optionPayoffIdx, m_state.m_payoffCombo.m_keysCArray, IM_ARRAYSIZE(m_state.m_payoffCombo.m_keysCArray));
                        ImGui::SameLine(); HelpMarker("Calls pay off when above the strike and Puts payoff when below");
            m_state.m_trd.m_optionPayoffType = static_cast<DPP::OptionPayoffType>(m_state.m_optionPayoffIdx);

            m_state.m_valueChanged |=	ImGui::InputDouble("Maturity (Y)", &m_state.m_trd.m_maturity, 0.25, 1.0, "%.2f");
                ImGui::SameLine(); HelpMarker("The initital value of the underlying");

            m_state.m_valueChanged |=  ImGui::InputDouble("Strike Price", &m_state.m_trd.m_strike, 0.25, 1.0, "%.2f");
                ImGui::SameLine(); HelpMarker("Strike Price");
    }
}
void PricerView::renderMarketParams()
{
    if( ImGui::CollapsingHeader("Market", ImGuiTreeNodeFlags_DefaultOpen) )
    {
        m_state.m_valueChanged |=	ImGui::InputDouble("Underlying Value", &m_state.m_mkt.m_underlyingPrice, 0.01, 1.0, "%.2f");
                    ImGui::SameLine(); HelpMarker("The initital value of the underlying");
        m_state.m_valueChanged |=	ImGui::InputDouble("Volatility", &m_state.m_mkt.m_vol, 0.01, 1.0, "%.2f");
                    ImGui::SameLine(); HelpMarker("Constant Volatility");
        m_state.m_valueChanged |=	ImGui::InputDouble("Interest Rate", &m_state.m_mkt.m_interestRate, 0.01, 1.0, "%.2f");
                    ImGui::SameLine(); HelpMarker("Interest Rate");
    }
}
void PricerView::renderCalcParams()
{
    if( ImGui::CollapsingHeader("Calculation", ImGuiTreeNodeFlags_DefaultOpen) )
    {
        m_state.m_valueChanged |=	ImGui::Checkbox("Dynamic Recalculation", &m_state.m_dynamicRecalc);
        
        m_state.m_valueChanged |= ImGui::Combo("Engine", &m_state.m_calculationMethodIdx, m_state.m_calculationMethodCombo.m_keysCArray, IM_ARRAYSIZE(m_state.m_calculationMethodCombo.m_keysCArray));
                    ImGui::SameLine(); HelpMarker("Engines determine the calculation techinique");
        
        m_state.m_calculationMethod = static_cast<DPP::CalculationMethod>(m_state.m_calculationMethodIdx);
        if( m_state.m_calculationMethod != DPP::CalculationMethod::BlackScholes )
            m_state.m_valueChanged |=	ImGui::InputInt("Time Periods", &m_state.m_steps);
                     ImGui::SameLine(); HelpMarker("Number of Nodes / Time steps");
        
        if (m_state.m_calculationMethod == DPP::CalculationMethod::MonteCarlo)
        {
            m_state.m_valueChanged |= ImGui::InputInt("Sims", &m_state.m_sims);
                ImGui::SameLine(); HelpMarker("Number of simulations");
            m_state.m_valueChanged |= ImGui::Combo("Sim Scheme", &m_state.m_pathSchemeComboIdx, m_state.m_pathSchemeCombo.m_keysCArray, IM_ARRAYSIZE(m_state.m_pathSchemeCombo.m_keysCArray));
                ImGui::SameLine(); HelpMarker("Scheme dictates how price evolves in Monte Carlo Sim");
            m_state.m_valueChanged |= ImGui::InputInt("Seed", &m_state.m_seed);
                ImGui::SameLine(); HelpMarker("Random seed used for Monte Carlo path generation");
        }
        
        m_state.m_valueChanged |= ImGui::Checkbox("PV", &m_state.m_calcsToDo[ (int)Calculation::PV ] );
                    ImGui::SameLine(); HelpMarker("Present Value of the option");
        m_state.m_valueChanged |= ImGui::Checkbox("Delta", &m_state.m_calcsToDo[ (int)Calculation::Delta ] );
                    ImGui::SameLine(); HelpMarker("The option value's sensitivity to the underlying's price");
        m_state.m_valueChanged |= ImGui::Checkbox("Gamma", &m_state.m_calcsToDo[ (int)Calculation::Gamma ] );
                    ImGui::SameLine(); HelpMarker("The option value's sensitivity to the underlying's delta");
        m_state.m_valueChanged |= ImGui::Checkbox("Vega", &m_state.m_calcsToDo[ (int)Calculation::Vega ] );
                    ImGui::SameLine(); HelpMarker("The option value's sensitivity to the underlying's volatility");
        m_state.m_valueChanged |= ImGui::Checkbox("Rho", &m_state.m_calcsToDo[ (int)Calculation::Rho ] );
                    ImGui::SameLine(); HelpMarker("The option value's sensitivity to the risk free rate");

        if (!m_state.m_dynamicRecalc)
            m_state.m_btn_calcPressed = ImGui::Button("Calculate");
    }
}
void PricerView::renderResults()
{
	if (!m_state.m_engineBuildError.empty())
    {
        ImGui::TextColored( ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Engine Build Error: %s", m_state.m_engineBuildError.c_str() );
        return;
    }

    if (ImGui::BeginTable("Results", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV ) )
    {
        for( const auto& calc : m_state.m_calcs )
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text( magic_enum::enum_name(calc.m_calc).data() );
            ImGui::TableSetColumnIndex(1);
            if( m_state.m_engine->m_results.contains(calc.m_calc) ) {
                if (m_state.m_engine->m_debugResults.contains(DPP::DebugInfo::MCPaths))
                {
                    ImGui::Begin("Monte Carlo Paths");
                    const std::string mcPlotTitle = std::string("Monte Carlo Paths: ")
                        + m_state.m_pathSchemeCombo.m_keysCArray[m_state.m_pathSchemeComboIdx]
                        + "  seed=" + std::to_string(m_state.m_seed);
                    if (ImPlot::BeginPlot(mcPlotTitle.c_str()))
                    {
                        const auto& lines = m_state.m_engine->m_debugResults.at(DPP::DebugInfo::MCPaths);
						std::vector<double> time_axis(calc.m_steps);
						const double dt = m_state.m_trd.m_maturity / static_cast<double>(calc.m_steps);
                        for (size_t i = 0; i < calc.m_steps; ++i) 
                            time_axis[i] = static_cast<double>(i) * dt;
                        for (size_t s = 0; s < calc.m_sims; ++s)
                        {
                            std::string label = std::string("Path ") + std::to_string(s);
                            const double* path_ptr = lines.data() + s * calc.m_steps;
							bool already_exist = ImPlot::GetItem(label.c_str()) != nullptr;

                            ImPlot::PlotLine(
                                label.c_str(),
                                time_axis.data(),
                                path_ptr,
                                static_cast<int>(calc.m_steps)
                            );

                            bool just_created = false;
                            ImPlotItem* item = ImPlot::RegisterOrGetItem(label.c_str(), 0, &just_created);

							bool set_to_hidden_initially = !already_exist && s > 5 && item->Show;
							if ( set_to_hidden_initially ) // Only show the first 5 paths for clarity
                                item->Show = false;
                        }
                        ImPlot::EndPlot();
                    }
                    ImGui::End();
                }
                const auto &res = m_state.m_engine->m_results.at(calc.m_calc);
                if (res.has_value())
                    ImGui::Text( "%.6f",  res.value() );
                else
                    ImGui::Text( "%s", res.error().c_str() );
            }
            else
                ImGui::Text( "No error nor result!" );
        }
        ImGui::EndTable();
    }

    if (m_state.m_timeTaken.has_value())
        ImGui::Text("Time Taken: %i ms", m_state.m_timeTaken.value());
}