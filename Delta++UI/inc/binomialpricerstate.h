#pragma once

#include <chrono>

#include "inc\trimatrixbuilder.h"
#include "enumcombo.h"

using namespace std::string_literals;

struct BinomialPricerState
{
	//MODEL
	double m_underlyingValue = 100.;
	double m_vol = 1.2;
	double m_interestRate = 0.25;
	double m_strike = 105.;
	int m_timePeriods = 3;
	double m_maturity = 1.5;
	int m_optionPayoffIdx = 0;
	int m_exerciseTypeIdx = 0;
	std::optional<double> m_optionPrice = std::nullopt;
	std::optional<std::string> m_error = std::nullopt;
	std::optional<int> m_timeTaken = std::nullopt;

	const EnumCombo< DPP::OptionPayoffType> m_payoffCombo;
	const EnumCombo< DPP::OptionExerciseType> m_exerciseCombo;

	//VIEW_MODEL
	bool m_valueChanged = false;
	bool m_dynamicRecalc = false;
	bool m_btn_calcPressed = false;

	void reset();
	bool needsRecal();
	bool recalcIfRequired();
};
