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
    void renderMassiveOptionsContractsSection();
    void renderMassiveOptionsAggregatesSection();

    ApiTesterState m_state;
};
