#pragma once

#include "framework.h";

#define DPP_EXPORTS

#include "triangularmatrix.h"

namespace DPP
{
	class TriMatrixBuilder
	{
	public:
		static TriMatrixBuilder create(const size_t steps, const double timeStep = 1.)
		{
			return TriMatrixBuilder(steps, timeStep);
		}

		TriMatrixBuilder& withUnderlyingValueAndVolatility(const double initialPrice, const double vol);
		TriMatrixBuilder& withUnderlyingValueAndUpFactor(const double initialPrice, const double upFactor);
		TriMatrixBuilder& withInterestRate(const double constantInterestRate);
		TriMatrixBuilder& withPayoff(const OptionPayoffType optionType, const double strikePrice);
		TriMatrixBuilder& withRiskNuetralProb();
		TriMatrixBuilder& withPremium(const OptionExerciseType exerciseType);
		TriMatrixBuilder& withDelta();
		TriMatrixBuilder& withPsuedoOptimalStoppingTime();
		TriMatrix build();
		const std::string& getErrorMsg() const;
	public:
		bool m_hasError;

	private:
		explicit TriMatrixBuilder(const size_t steps, const double timeStep = 1.);

	private:
		TriMatrix m_result;
		size_t m_timeSteps;
		double m_timeStep;
		double m_upFactor;
		double m_downFactor;
		double m_interestRate;
		OptionExerciseType m_exerciseType;
		std::string m_errorMsg;
	};
}