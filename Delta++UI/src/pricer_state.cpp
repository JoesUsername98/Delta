#include "pricer_state.h"

// Define static EnumCombo instances
const EnumCombo<DPP::OptionPayoffType> PricerState::m_payoffCombo{};
const EnumCombo<DPP::OptionExerciseType> PricerState::m_exerciseCombo{};
const EnumCombo<DPP::PathSchemeType> PricerState::m_pathSchemeCombo{};
const EnumCombo<DPP::CalculationMethod> PricerState::m_calculationMethodCombo{};

void PricerState::reset()
{
	m_btn_calcPressed = false;
	m_valueChanged = false;
}

bool PricerState::needsRecalc()
{
	m_valueChanged |= m_btn_calcPressed;
	bool needsRecalc = (m_valueChanged && m_dynamicRecalc) || (!m_dynamicRecalc && m_btn_calcPressed);
	return needsRecalc;
}

bool PricerState::recalcIfRequired()
{
	if (!needsRecalc())
		return false;

	m_calcs.clear();
	for( int i = (int)Calculation::PV; i < (int)Calculation::_SIZE; ++i )
	{
		if( !m_calcsToDo[i] )
			continue;

		m_calcs.emplace_back( (Calculation)i, m_steps, m_sims, static_cast<DPP::PathSchemeType>(m_pathSchemeComboIdx) );
	}

	const auto start = std::chrono::high_resolution_clock::now();

	auto engine_res = EngineFactory::getEngine( m_calculationMethod, m_mkt, m_trd, m_calcs );

	if (!engine_res.has_value())
	{
		m_engine.reset();
		m_engineBuildError = engine_res.error();
		return false;
	}

	m_engineBuildError.clear();
	m_engine = std::move(engine_res.value());
	m_engine->run();

	const auto end = std::chrono::high_resolution_clock::now();
	m_timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	return true;
}