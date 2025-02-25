#pragma once

#include <chrono>

#include "engine_factory.h"
#include "enum_combo.h"

using namespace std::string_literals;
using namespace DPP;

struct PricerState
{
	//MODEL
	TradeData m_trd;
	MarketData m_mkt;
	int m_steps = 3;
	std::vector<CalcData> m_calcs;
	std::unique_ptr<AbstractEngine> m_engine;
	std::array<bool, (int)Calculation::_SIZE> m_calcsToDo;
	std::optional<int> m_timeTaken = std::nullopt;

	int m_optionPayoffIdx = 0;
	int m_exerciseTypeIdx = 0;
	int m_calculationMethodIdx = 0;
	const EnumCombo< OptionPayoffType > m_payoffCombo;
	const EnumCombo< OptionExerciseType > m_exerciseCombo;
	const EnumCombo< CalculationMethod > m_calculationMethodCombo;

	//VIEW_MODEL
	bool m_valueChanged = false;
	bool m_dynamicRecalc = false;
	bool m_btn_calcPressed = false;

	void reset();
	bool needsRecalc();
	bool recalcIfRequired();

	PricerState( ) :
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
