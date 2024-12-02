#include <chrono>
#include <string>

//******************************************************
//*****            SUBMODULES            ***************
#define WL_PLATFORM_WINDOWS
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
//*****   ~~~~~~~  SUBMODULES   ~~~~~~  ****************
//******************************************************

//******************************************************
//*****            COPIED FILES            *************
#include "magic_enum.hpp"
//*****   ~~~~~~~  COPIED FILES   ~~~~~~  **************
//******************************************************

#include "../Delta++/trimatrixbuilder.h"

using namespace std::string_literals;
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

struct State 
{
	bool valueChanged = false;
	double underlyingValue = 100.;
	double vol = 1.2;
	double interestRate = 0.25;
	double strike = 105.;
	int timePeriods = 3;
	double maturity = 1.5;
	int optionPayoffIdx = 0;
	int exerciseTypeIdx = 0;
	std::optional<double> optionPrice = std::nullopt;
	std::optional<std::string> error = std::nullopt;
	std::optional<int> timeTaken = std::nullopt;
	bool dynamicRecalc = false;
};

class ExampleLayer : public Walnut::Layer
{
private:
	State state;

public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Binomial Pricer");

		state.valueChanged |=	ImGui::Checkbox("Dynamic Recalculation", &state.dynamicRecalc);
		state.valueChanged |=	ImGui::InputInt("Time Periods", &state.timePeriods);
					ImGui::SameLine(); HelpMarker("Number of Nodes");
		state.valueChanged |=	ImGui::InputDouble("Maturity (Y)", &state.maturity, 0.25, 1.0, "%.2f");
					ImGui::SameLine(); HelpMarker("The initital value of the underlying");
		state.valueChanged |=	ImGui::InputDouble("Underlying Value", &state.underlyingValue, 0.01, 1.0, "%.2f");
					ImGui::SameLine(); HelpMarker("The initital value of the underlying");
		state.valueChanged |=	ImGui::InputDouble("Volatility", &state.vol, 0.01, 1.0, "%.2f");
					ImGui::SameLine(); HelpMarker("Constant Volatility");
		state.valueChanged |=	ImGui::InputDouble("Interest Rate", &state.interestRate, 0.01, 1.0, "%.2f");
					ImGui::SameLine(); HelpMarker("Interest Rate");
		state.valueChanged |=  ImGui::InputDouble("Strike Price", &state.strike, 0.01, 1.0, "%.2f");
					ImGui::SameLine(); HelpMarker("Strike Price");

		static constexpr auto optionPayoffKeys = magic_enum::enum_names<DPP::OptionPayoffType>();
		static constexpr size_t optionPayoffSize = std::tuple_size<decltype(optionPayoffKeys)>::value;
		static const char* optionPayoffKeys_CArr[optionPayoffSize] = {};
		for (size_t i = 0; i < optionPayoffSize; ++i) { optionPayoffKeys_CArr[i] = optionPayoffKeys[i].data(); }
        
		state.valueChanged |=	ImGui::Combo("Payoff", &state.optionPayoffIdx, optionPayoffKeys_CArr, IM_ARRAYSIZE(optionPayoffKeys_CArr));
					ImGui::SameLine(); HelpMarker("Calls pay off when above the strike and Puts payoff when below");
        
		static constexpr auto exerciseTypeKeys = magic_enum::enum_names<DPP::OptionExerciseType>();
		static constexpr size_t exerciseTypeKeysSize = std::tuple_size<decltype(exerciseTypeKeys)>::value;
		static const char* exerciseTypeKeys_CArr[exerciseTypeKeysSize] = {};
		for (size_t i = 0; i < exerciseTypeKeysSize; ++i) { exerciseTypeKeys_CArr[i] = exerciseTypeKeys[i].data(); }
		state.valueChanged |=	ImGui::Combo("Exercise", &state.exerciseTypeIdx, exerciseTypeKeys_CArr, IM_ARRAYSIZE(exerciseTypeKeys_CArr));
					ImGui::SameLine(); HelpMarker("Europeans exercises at maturity while Americans can exercise at any timestep");

		bool calculateBtnPressed = false; 
		if (!state.dynamicRecalc)
			calculateBtnPressed = ImGui::Button("Calculate");

		state.valueChanged |= calculateBtnPressed;
		if ( (state.valueChanged && state.dynamicRecalc ) || ( !state.dynamicRecalc && calculateBtnPressed ) )
		{
			const auto start = std::chrono::high_resolution_clock::now();
			DPP::TriMatrixBuilder buildResult = DPP::TriMatrixBuilder::create(state.timePeriods, state.maturity/ state.timePeriods)
				.withUnderlyingValueAndVolatility(state.underlyingValue, state.vol)
				.withInterestRate(state.interestRate)
				.withPayoff(static_cast<DPP::OptionPayoffType>(state.optionPayoffIdx), state.strike)
				.withRiskNuetralProb()
				.withPremium(static_cast<DPP::OptionExerciseType>(state.exerciseTypeIdx))
				.withDelta()
				.withPsuedoOptimalStoppingTime();
			const auto end = std::chrono::high_resolution_clock::now();
			const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			state.timeTaken = duration;

			if (buildResult.m_hasError) 
			{
				state.optionPrice = std::nullopt;
				state.error = "ERROR! : "s + buildResult.getErrorMsg();
			}
			else
			{
				const DPP::TriMatrix result = buildResult.build();
				state.optionPrice = result.getMatrix()[0][0].m_data.m_optionValue;
				state.error = std::nullopt;
			}
		}

		if (state.error.has_value())
			ImGui::Text(state.error.value().c_str() );

		if(state.optionPrice.has_value() )
			ImGui::Text("Option Value: %.6f", state.optionPrice.value() );

		if (state.timeTaken.has_value())
			ImGui::Text("Time Taken: %i ms", state.timeTaken.value());

		state.valueChanged = false;

		ImGui::End();

		//ImGui::ShowDemoWindow();
	}
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
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