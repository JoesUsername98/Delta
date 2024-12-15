#pragma once

#include "../../_deps/walnut-cmake-src/Walnut/src/Walnut/Layer.h"

#include "ui_utils.h"
#include "binomial_pricer_state.h"

class BinomialPricerView : public Walnut::Layer
{
private:
	BinomialPricerState m_state;
	static size_t s_type_count;
	const size_t M_ID;
	const std::string M_NAME;

public:
	BinomialPricerView() : M_ID( s_type_count++ ), M_NAME( "Binomial Pricer " + std::to_string(M_ID)  ) {};
	void OnUIRender() override;
};

