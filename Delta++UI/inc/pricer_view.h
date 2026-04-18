#pragma once

#include <cstdint>
#include <limits>

#include <Walnut/Layer.h>

#include "ui_utils.h"
#include "pricer_state.h"

class PricerView : public Walnut::Layer
{
private:
	PricerState m_state;
	static size_t s_type_count;
	const size_t M_ID;
	const std::string M_NAME;
	void renderTradeParams();
	void renderMarketParams();
	void renderCalcParams();
	void renderResults();
    void renderMCPathsPlot();
    void renderVol3DWindows();

    bool m_showIv3d = false;
    bool m_showLv3d = false;
    bool m_showCall3d = false;

    uint32_t m_lastYieldCurvePlotEpoch = std::numeric_limits<uint32_t>::max();
    uint32_t m_lastDividendPlotEpoch = std::numeric_limits<uint32_t>::max();

public:
	PricerView() : M_ID( s_type_count++ ), M_NAME( "Pricer " + std::to_string(M_ID)  ) {};
	void OnUIRender() override;
};

