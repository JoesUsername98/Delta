#include <string>

#include "../../_deps/walnut-cmake-src/Walnut/src/Walnut/Application.h"
#include "../../_deps/walnut-cmake-src/Walnut/src/Walnut/EntryPoint.h"
#include "../../_deps/walnut-cmake-src/Walnut/src/Walnut/Image.h"

#include "binomial_pricer_view.h"
#include "demo_view.h"

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
			if (ImGui::BeginMenu("New"))
			{
				if (ImGui::MenuItem("Binomial Pricer Window"))
				{
					app->PushLayer<BinomialPricerView>();
				}
				if( ImGui::MenuItem("Demo Window") )
				{
					app->PushLayer<DemoWindow>();
				}
				
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}

			ImGui::EndMenu();
		}
	});
	return app;
}