#include "binomial_pricer_state.h"

void BinomialPricerState::reset()
{
	m_btn_calcPressed = false;
	m_valueChanged = false;
}

bool BinomialPricerState::needsRecalc()
{
	m_valueChanged |= m_btn_calcPressed;
	bool needsRecalc = (m_valueChanged && m_dynamicRecalc) || (!m_dynamicRecalc && m_btn_calcPressed);
	return needsRecalc;
}

bool BinomialPricerState::recalcIfRequired()
{
	if (!needsRecalc())
		return false;

	m_calcs.clear();
	for( int i = (int)Calculation::PV; i < (int)Calculation::_SIZE; ++i )
	{
		if( !m_calcsToDo[i] )
			continue;

		m_calcs.emplace_back( (Calculation)i, m_steps, CalculationMethod::Binomial );
	}

	const auto start = std::chrono::high_resolution_clock::now();

	m_engine = std::make_unique<Engine>( m_mkt, m_trd, m_calcs );
	m_engine->run();

	const auto end = std::chrono::high_resolution_clock::now();
	m_timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	return true;
}