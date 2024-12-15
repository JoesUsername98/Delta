#include "binomial_pricer_view.h"


size_t BinomialPricerView::s_type_count = 0;

void BinomialPricerView::OnUIRender()
{
    ImGui::Begin( M_NAME.c_str() );

    m_state.m_valueChanged |=	ImGui::Checkbox("Dynamic Recalculation", &m_state.m_dynamicRecalc);
    m_state.m_valueChanged |=	ImGui::InputInt("Time Periods", &m_state.m_timePeriods);
                ImGui::SameLine(); HelpMarker("Number of Nodes");
    m_state.m_valueChanged |=	ImGui::InputDouble("Maturity (Y)", &m_state.m_maturity, 0.25, 1.0, "%.2f");
                ImGui::SameLine(); HelpMarker("The initital value of the underlying");
    m_state.m_valueChanged |=	ImGui::InputDouble("Underlying Value", &m_state.m_underlyingValue, 0.01, 1.0, "%.2f");
                ImGui::SameLine(); HelpMarker("The initital value of the underlying");
    m_state.m_valueChanged |=	ImGui::InputDouble("Volatility", &m_state.m_vol, 0.01, 1.0, "%.2f");
                ImGui::SameLine(); HelpMarker("Constant Volatility");
    m_state.m_valueChanged |=	ImGui::InputDouble("Interest Rate", &m_state.m_interestRate, 0.01, 1.0, "%.2f");
                ImGui::SameLine(); HelpMarker("Interest Rate");
    m_state.m_valueChanged |=  ImGui::InputDouble("Strike Price", &m_state.m_strike, 0.01, 1.0, "%.2f");
                ImGui::SameLine(); HelpMarker("Strike Price");
    m_state.m_valueChanged |=	ImGui::Combo("Payoff", &m_state.m_optionPayoffIdx, m_state.m_payoffCombo.m_keysCArray, IM_ARRAYSIZE(m_state.m_payoffCombo.m_keysCArray));
                ImGui::SameLine(); HelpMarker("Calls pay off when above the strike and Puts payoff when below");
    m_state.m_valueChanged |=	ImGui::Combo("Exercise", &m_state.m_exerciseTypeIdx, m_state.m_exerciseCombo.m_keysCArray, IM_ARRAYSIZE(m_state.m_exerciseCombo.m_keysCArray));
                ImGui::SameLine(); HelpMarker("Europeans exercises at maturity while Americans can exercise at any timestep");

    if (!m_state.m_dynamicRecalc)
        m_state.m_btn_calcPressed = ImGui::Button("Calculate");

    m_state.recalcIfRequired();

    if (m_state.m_error.has_value())
        ImGui::Text(m_state.m_error.value().c_str() );

    if(m_state.m_optionPrice.has_value() )
        ImGui::Text("Option Value: %.6f", m_state.m_optionPrice.value() );

    if (m_state.m_timeTaken.has_value())
        ImGui::Text("Time Taken: %i ms", m_state.m_timeTaken.value());

    m_state.reset();
    ImGui::End();

    //ImGui::ShowDemoWindow();
}