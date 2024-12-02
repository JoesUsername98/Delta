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

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Binomial Pricer");

		// Abstract state to a view model
		static bool valueChanged = false;
		static double underlyingValue = 100.;
		static double vol = 1.2;
		static double interestRate = 0.25;
		static double strike = 105.;
		static int timePeriods = 3;
		static double maturity = 1.5;
		static int optionPayoffIdx = 0;
		static int exerciseTypeIdx = 0;
		static std::optional<double> optionPrice = std::nullopt;
		static std::optional<std::string> error = std::nullopt;
		static std::optional<int> timeTaken = std::nullopt;
		static bool dynamicRecalc = false;

		valueChanged |=	ImGui::Checkbox("Dynamic Recalculation", &dynamicRecalc);
		valueChanged |=	ImGui::InputInt("Time Periods", &timePeriods);
					ImGui::SameLine(); HelpMarker("Number of Nodes");
		valueChanged |=	ImGui::InputDouble("Maturity (Y)", &maturity, 0.25, 1.0, "%.2f");
					ImGui::SameLine(); HelpMarker("The initital value of the underlying");
		valueChanged |=	ImGui::InputDouble("Underlying Value", &underlyingValue, 0.01, 1.0, "%.2f");
					ImGui::SameLine(); HelpMarker("The initital value of the underlying");
		valueChanged |=	ImGui::InputDouble("Volatility", &vol, 0.01, 1.0, "%.2f");
					ImGui::SameLine(); HelpMarker("Constant Volatility");
		valueChanged |=	ImGui::InputDouble("Interest Rate", &interestRate, 0.01, 1.0, "%.2f");
					ImGui::SameLine(); HelpMarker("Interest Rate");
		valueChanged |=  ImGui::InputDouble("Strike Price", &strike, 0.01, 1.0, "%.2f");
					ImGui::SameLine(); HelpMarker("Strike Price");

		static constexpr auto optionPayoffKeys = magic_enum::enum_names<DPP::OptionPayoffType>();
		static constexpr size_t optionPayoffSize = std::tuple_size<decltype(optionPayoffKeys)>::value;
		static const char* optionPayoffKeys_CArr[optionPayoffSize] = {};
		for (size_t i = 0; i < optionPayoffSize; ++i) { optionPayoffKeys_CArr[i] = optionPayoffKeys[i].data(); }
        
		valueChanged |=	ImGui::Combo("Payoff", &optionPayoffIdx, optionPayoffKeys_CArr, IM_ARRAYSIZE(optionPayoffKeys_CArr));
					ImGui::SameLine(); HelpMarker("Calls pay off when above the strike and Puts payoff when below");
        
		static constexpr auto exerciseTypeKeys = magic_enum::enum_names<DPP::OptionExerciseType>();
		static constexpr size_t exerciseTypeKeysSize = std::tuple_size<decltype(exerciseTypeKeys)>::value;
		static const char* exerciseTypeKeys_CArr[exerciseTypeKeysSize] = {};
		for (size_t i = 0; i < exerciseTypeKeysSize; ++i) { exerciseTypeKeys_CArr[i] = exerciseTypeKeys[i].data(); }
		valueChanged |=	ImGui::Combo("Exercise", &exerciseTypeIdx, exerciseTypeKeys_CArr, IM_ARRAYSIZE(exerciseTypeKeys_CArr));
					ImGui::SameLine(); HelpMarker("Europeans exercises at maturity while Americans can exercise at any timestep");

		bool calculateBtnPressed = false; 
		if (!dynamicRecalc)
			calculateBtnPressed = ImGui::Button("Calculate");

		valueChanged |= calculateBtnPressed;
		if ( ( valueChanged && dynamicRecalc ) || ( !dynamicRecalc && calculateBtnPressed ) )
		{
			const auto start = std::chrono::high_resolution_clock::now();
			DPP::TriMatrixBuilder buildResult = DPP::TriMatrixBuilder::create(timePeriods, maturity/ timePeriods)
				.withUnderlyingValueAndVolatility(underlyingValue, vol)
				.withInterestRate(interestRate)
				.withPayoff(static_cast<DPP::OptionPayoffType>(optionPayoffIdx), strike)
				.withRiskNuetralProb()
				.withPremium(static_cast<DPP::OptionExerciseType>(exerciseTypeIdx))
				.withDelta()
				.withPsuedoOptimalStoppingTime();
			const auto end = std::chrono::high_resolution_clock::now();
			const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			timeTaken = duration;

			if (buildResult.m_hasError) 
			{
				optionPrice = std::nullopt;
				error = "ERROR! : "s + buildResult.getErrorMsg();
			}
			else
			{
				const DPP::TriMatrix result = buildResult.build();
				optionPrice = result.getMatrix()[0][0].m_data.m_optionValue;
				error = std::nullopt;
			}
		}

		if (error.has_value())
			ImGui::Text( error.value().c_str() );

		if( optionPrice.has_value() )
			ImGui::Text("Option Value: %.6f", optionPrice.value() );

		if (timeTaken.has_value())
			ImGui::Text("Time Taken: %i ms", timeTaken.value());

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