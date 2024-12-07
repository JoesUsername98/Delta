#pragma once

#include "framework.h";

#define DPP_EXPORTS

#include "triangularmatrix.h"

namespace DPP
{
	extern DPP_API class TriMatrixBuilder
	{
	public:
		static DPP_API TriMatrixBuilder create(const size_t steps, const double timeStep = 1.)
		{
			return TriMatrixBuilder(steps, timeStep);
		}

		DPP_API TriMatrixBuilder& withUnderlyingValueAndVolatility(const double initialPrice, const double vol);
		DPP_API TriMatrixBuilder& withUnderlyingValueAndUpFactor(const double initialPrice, const double upFactor);
		DPP_API TriMatrixBuilder& withInterestRate(const double constantInterestRate);
		DPP_API TriMatrixBuilder& withPayoff(const OptionPayoffType optionType, const double strikePrice);
		DPP_API TriMatrixBuilder& withRiskNuetralProb();
		DPP_API TriMatrixBuilder& withPremium(const OptionExerciseType exerciseType);
		DPP_API TriMatrixBuilder& withDelta();
		DPP_API TriMatrixBuilder& withPsuedoOptimalStoppingTime();
		DPP_API TriMatrix build();
		DPP_API const std::string& getErrorMsg() const;
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