#pragma once

#include <chrono>

#include "engine.h"
#include "enum_combo.h"

using namespace std::string_literals;
using namespace DPP;

struct BinomialPricerState
{
	//MODEL
	TradeData m_trd;
	int m_optionPayoffIdx = 0;
	int m_exerciseTypeIdx = 0;
	MarketData m_mkt;
	int m_steps = 3;
	std::vector<CalcData> m_calcs;
	std::unique_ptr<Engine> m_engine;
	std::array<bool, (int)Calculation::_SIZE> m_calcsToDo;
	std::optional<int> m_timeTaken = std::nullopt;

	const EnumCombo< OptionPayoffType > m_payoffCombo;
	const EnumCombo< OptionExerciseType > m_exerciseCombo;

	//VIEW_MODEL
	bool m_valueChanged = false;
	bool m_dynamicRecalc = false;
	bool m_btn_calcPressed = false;

	void reset();
	bool needsRecalc();
	bool recalcIfRequired();

	BinomialPricerState( ) :
	 m_trd( OptionExerciseType::European, OptionPayoffType::Call, 105., 1.5 ),
	 m_mkt( 1.2, 100., 0.25 ) 
	 {
		m_calcsToDo[ (int)Calculation::PV ] = true;
		m_calcsToDo[ (int)Calculation::Delta ] = false;
		m_calcsToDo[ (int)Calculation::Gamma ] = false;
		m_calcsToDo[ (int)Calculation::Vega ] = false;
		m_calcsToDo[ (int)Calculation::Rho ] = false;
		m_calcsToDo[ (int)Calculation::Delta ] = false;
	 }
};
