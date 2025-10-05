#pragma once

#include "triangular_matrix.h"
#include <functional>

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
		const size_t m_timeSteps;
		const double m_timeStep;
		double m_initialPrice;
		double m_upFactor;
		double m_downFactor;
		double m_interestRate;
		double m_discountRate;
		double m_strikePrice;
		OptionPayoffType m_optionType;
		double m_probabilityHeads;
		OptionExerciseType m_exerciseType;
		std::string m_errorMsg;

		bool m_mustCalcDeltaHedging;
		bool m_mustCalcOptiomalStoppingTime;

		// Todo move to BAMP engine
		void calcDeltaHedging( TriMatrix& result );
		// Todo move to BAMP engine
		void calcOptiomalStoppingTime( TriMatrix& result ) const;

		std::function< void( Node& )> m_setPayoff;
		std::function< void( Node&, const Node*, const Node* )> m_setExerciseValue;
		std::function< double( const Node*, const Node* )> m_calcExpPV;
	};
}