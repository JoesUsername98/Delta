#include "pch.h"

#include "trimatrixbuilder.h"

namespace DPP
{

#define RTN_BUILDER_ERR(msg)  \
		m_hasError = true;  \
		m_errorMsg = std::string( msg ); \
		return *this;  



	TriMatrixBuilder::TriMatrixBuilder( const size_t steps, const double timeStep ) :
		m_timeSteps(steps),
		m_timeStep(timeStep),
		m_result(steps, timeStep),
		m_hasError(false),
		m_errorMsg("")
	{
	}
	TriMatrixBuilder& TriMatrixBuilder::withUnderlyingValueAndVolatility(const double initialPrice, const double vol)
	{
		const double upFactor = exp(vol * sqrt(m_timeStep));
		return withUnderlyingValueAndUpFactor(initialPrice, upFactor);
	}
	TriMatrixBuilder& TriMatrixBuilder::withUnderlyingValueAndUpFactor(const double initialPrice, const double upFactor)
	{
		if (m_hasError) 
			return *this;
		if (initialPrice < 0) { RTN_BUILDER_ERR("initialPrice cannot be 0"); }
		if (upFactor <= 0) { RTN_BUILDER_ERR("upFactor cannot be 0 or less"); }
		if (upFactor < 1 / upFactor) { RTN_BUILDER_ERR("upFactor cannot be less than downFactor"); }

		m_upFactor = upFactor;
		m_downFactor = 1 / m_upFactor;

		for (int step = m_result.m_matrix.size() - 1; step >= 0; step--)
			for (int downMoves = m_result.m_matrix[step].size() - 1; downMoves >= 0; downMoves--)
			{
				size_t noOfHeads = step - downMoves;
				Node& node = m_result.m_matrix[step][downMoves];
				node.m_timeStep = step;
				node.m_downMoves = downMoves;
				node.m_data.m_underlyingValue = initialPrice * pow(m_upFactor, noOfHeads) * pow(m_downFactor, downMoves);
			}

		return *this;
	}
	TriMatrixBuilder& TriMatrixBuilder::withInterestRate(const double constantInterestRate)
	{
		if (m_hasError) return *this;

		m_interestRate = constantInterestRate;

		for (auto& timeStep : m_result.m_matrix)
			for (auto& node : timeStep)
				node.m_data.m_interestRate = pow(1. + constantInterestRate, m_timeStep) - 1.;

		return *this;
	}
	TriMatrixBuilder& TriMatrixBuilder::withPayoff(const OptionPayoffType optionType, const double strikePrice)
	{
		if (m_hasError) return *this;

		for (auto& timeStep : m_result.m_matrix)
			for (auto& node : timeStep)
				switch (optionType)
				{
				case OptionPayoffType::Call:
				{
					node.m_data.m_payoff = node.m_data.m_underlyingValue - strikePrice;
					break;
				}
				case OptionPayoffType::Put:
				{
					node.m_data.m_payoff = strikePrice - node.m_data.m_underlyingValue;
					break;
				}
				default:
				{
					RTN_BUILDER_ERR("Invalid payoff strategy");
				}
				}

		return *this;
	}
	TriMatrixBuilder& TriMatrixBuilder::withRiskNuetralProb()
	{
		if (m_hasError) return *this;

		if (m_upFactor <= 1 + m_interestRate) { RTN_BUILDER_ERR("u > 1 + r to prevent arbitrage"); }

		const double growthFactor = exp(m_interestRate * m_timeStep); // Hull 12.6

		for (auto& timeStep : m_result.m_matrix)
			for (auto& node : timeStep)
				node.m_data.m_probabilityHeads = (growthFactor - m_downFactor) / (m_upFactor - m_downFactor); // Hull 12.5

		return *this;
	}
	TriMatrixBuilder& TriMatrixBuilder::withPremium(const OptionExerciseType exerciseType)
	{
		if (m_hasError) return *this;

		m_exerciseType = exerciseType;
		for (int step = m_timeSteps ; step >= 0; step--)
			for (int downMoves = m_result.m_matrix[step].size() - 1; downMoves >= 0; downMoves--)
			{
				Node& node = m_result.m_matrix[step][downMoves];
				const Node* heads = m_result.getHeads(node);
				const Node* tails = m_result.getTails(node);

				if (node.m_timeStep == m_timeSteps || exerciseType == OptionExerciseType::American)
				{
					switch (exerciseType)
					{
					case OptionExerciseType::European:
					{
						node.m_data.m_optionValue = max(node.m_data.m_payoff, 0);
						break;
					}
					case OptionExerciseType::American:
					{
						node.m_data.m_optionValue = max(node.m_data.m_payoff, (!heads || !tails) ? 0. :
							node.m_data.getDiscountRate() *
							(heads->m_data.m_optionValue * node.m_data.m_probabilityHeads +
								tails->m_data.m_optionValue * node.m_data.getProbabilityTails()));
						break;
					}
					default:
					{
						RTN_BUILDER_ERR("Invalid option pricing strategy");
					}
					}

					continue;
				}

				node.m_data.m_optionValue = node.m_data.getDiscountRate() *
					(heads->m_data.m_optionValue * node.m_data.m_probabilityHeads +
						tails->m_data.m_optionValue * node.m_data.getProbabilityTails());
			}

		return *this;
	}
	TriMatrixBuilder& TriMatrixBuilder::withDelta()
	{
		if (m_hasError) return *this;

		for ( int step = m_timeSteps - 2; step >= 0; step--)
			for ( int downMoves = m_result.m_matrix[step].size() - 1; downMoves >= 0; downMoves--)
			{
				Node& node = m_result.m_matrix[step][downMoves];
				const Node* heads = m_result.getHeads(node);
				const Node* tails = m_result.getTails(node);

				if (heads->m_data.m_underlyingValue == tails->m_data.m_underlyingValue)
				{
					RTN_BUILDER_ERR("Underlying Value of heads and tails node are the same leading to a divide by 0 error");
				}

				node.m_data.m_deltaHedging = (heads->m_data.m_optionValue - tails->m_data.m_optionValue)
					/ (heads->m_data.m_underlyingValue - tails->m_data.m_underlyingValue);
			}
		return *this;
	}
	TriMatrixBuilder& TriMatrixBuilder::withPsuedoOptimalStoppingTime()
	{
		if (m_hasError) return *this;

		if (m_exerciseType != OptionExerciseType::American)
			return *this;

		//Diag Traverseal
		for (size_t timeStep = 0; timeStep < m_timeSteps; ++timeStep)
		{
			size_t optimalExEarlyPutTimeStep = MAXSIZE_T;
			for (size_t diagMoves = 0; diagMoves < m_result.m_matrix[m_timeSteps - 1 - timeStep].size(); ++diagMoves)
			{
				State& state = m_result.m_matrix[timeStep + diagMoves][diagMoves].m_data;
				if (state.m_optionValue == state.m_payoff && optimalExEarlyPutTimeStep == MAXSIZE_T)
					optimalExEarlyPutTimeStep = timeStep + diagMoves;
				state.m_optimalExerciseTime = optimalExEarlyPutTimeStep;
			}
		}

		return *this;
	}
	TriMatrix TriMatrixBuilder::build()
	{
		if (m_hasError)
			throw std::exception( m_errorMsg.c_str() );

		return std::move(m_result);
	}
	const std::string& TriMatrixBuilder::getErrorMsg() const
	{
		return m_errorMsg;
	}
}