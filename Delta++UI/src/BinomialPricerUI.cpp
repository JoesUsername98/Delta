#include <string>

//******************************************************
//*****            SUBMODULES            ***************
#define WL_PLATFORM_WINDOWS
#include "../../_deps/walnut-cmake-src/Walnut/src/Walnut/Application.h"
#include "../../_deps/walnut-cmake-src/Walnut/src/Walnut/EntryPoint.h"
#include "../../_deps/walnut-cmake-src/Walnut/src/Walnut/Image.h"
//*****   ~~~~~~~  SUBMODULES   ~~~~~~  ****************
//******************************************************

#include "binomialpricerstate.h"

static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

class BinomialPricerView : public Walnut::Layer
{
private:
	BinomialPricerState m_state;

public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Binomial Pricer");

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
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Delta++";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<BinomialPricerView>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}