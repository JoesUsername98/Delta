#include <string>

#include <Walnut/Application.h>
#include <Walnut/EntryPoint.h>
#include <Walnut/Image.h>

#include "pricer_view.h"
#include "demo_view.h"
#include "api_tester_view.h"
#include "local_vol_surface_view.h"

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Delta++";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<PricerView>();
	app->PushLayer<ApiTesterWindow>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("New"))
			{
				if (ImGui::MenuItem("Pricer Window"))
				{
					app->PushLayer<PricerView>();
				}
				if( ImGui::MenuItem("Demo Window") )
				{
					app->PushLayer<DemoWindow>();
				}
				if( ImGui::MenuItem("API Tester Window") )
				{
					app->PushLayer<ApiTesterWindow>();
				}
                if (ImGui::MenuItem("Local Vol Surface"))
                {
                    app->PushLayer<LocalVolSurfaceWindow>();
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
