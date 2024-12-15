#include "inc/binomialpricerstate.h"

void BinomialPricerState::reset()
{
	m_btn_calcPressed = false;
	m_valueChanged = false;
}

bool BinomialPricerState::needsRecal()
{
	m_valueChanged |= m_btn_calcPressed;
	bool needsRecalc = (m_valueChanged && m_dynamicRecalc) || (!m_dynamicRecalc && m_btn_calcPressed);
	return needsRecalc;
}

bool BinomialPricerState::recalcIfRequired()
{
	if (!needsRecal())
		return false;

	const auto start = std::chrono::high_resolution_clock::now();

	DPP::TriMatrixBuilder buildResult = DPP::TriMatrixBuilder::create(m_timePeriods, m_maturity / m_timePeriods)
		.withUnderlyingValueAndVolatility(m_underlyingValue, m_vol)
		.withInterestRate(m_interestRate)
		.withPayoff(static_cast<DPP::OptionPayoffType>(m_optionPayoffIdx), m_strike)
		.withRiskNuetralProb()
		.withPremium(static_cast<DPP::OptionExerciseType>(m_exerciseTypeIdx))
		.withDelta()
		.withPsuedoOptimalStoppingTime();

	const auto end = std::chrono::high_resolution_clock::now();
	m_timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	if (buildResult.m_hasError)
	{
		m_optionPrice = std::nullopt;
		m_error = "ERROR! : "s + buildResult.getErrorMsg();
	}
	else
	{
		const DPP::TriMatrix result = buildResult.build();
		m_optionPrice = result.getMatrix()[0][0].m_data.m_optionValue;
		m_error = std::nullopt;
	}

	return true;
}