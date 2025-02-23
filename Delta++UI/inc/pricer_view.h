#pragma once

#include "../../_deps/walnut-cmake-src/Walnut/src/Walnut/Layer.h"

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

public:
	PricerView() : M_ID( s_type_count++ ), M_NAME( "Pricer " + std::to_string(M_ID)  ) {};
	void OnUIRender() override;
};

