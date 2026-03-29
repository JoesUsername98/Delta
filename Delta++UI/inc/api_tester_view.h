#pragma once

#include <Walnut/Layer.h>

#include "api_tester_state.h"

class ApiTesterWindow : public Walnut::Layer
{
public:
    ApiTesterWindow();

    void OnUIRender() override;

private:
    void renderYieldCurveSection();
    void renderZeroRatePlot();
    void renderAlphaVantageSection();

    ApiTesterState m_state;
};
