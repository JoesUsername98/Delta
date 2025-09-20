#pragma once

#include <Walnut/Layer.h>

class DemoWindow : public Walnut::Layer
{
public:
	void OnUIRender() override
    {
        ImGui::ShowDemoWindow();
    }
};