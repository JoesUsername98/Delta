#include "pricer_state.h"

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

		m_calcs.emplace_back( (Calculation)i, m_steps, m_sims );
	}

	const auto start = std::chrono::high_resolution_clock::now();

	m_engine = EngineFactory::getEngine( m_calculationMethod, m_mkt, m_trd, m_calcs );
	m_engine->run();

	const auto end = std::chrono::high_resolution_clock::now();
	m_timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	return true;
}