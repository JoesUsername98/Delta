#pragma once

#include "../../_deps/walnut-cmake-src/Walnut/src/Walnut/Layer.h"

class DemoWindow : public Walnut::Layer
{
public:
	void OnUIRender() override
    {
        ImGui::ShowDemoWindow();
    }
};