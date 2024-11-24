#pragma once
#include "framework.h";

#define DPP_EXPORTS

#include <vector>
#include <optional>
#include <exception>
#include <string>

namespace DPP
{
	extern DPP_API struct ExpectableState
	{
		double m_underlyingValue;
		double m_payoff;
		double m_interestRate;

		DPP_API double getDiscountRate() const;

	};
	
	extern DPP_API struct State : ExpectableState
	{
		double m_payoff;
		double m_optionValue;
		double m_deltaHedging;
		double m_probabilityHeads;
		std::optional<size_t> m_optimalExerciseTime;
		ExpectableState m_expected;

		DPP_API double getProbabilityTails() const;
	};

	extern DPP_API struct Node
	{
		State m_data;
		size_t m_timeStep;
		size_t m_downMoves;
	};

	extern DPP_API enum class OptionPayoffType
	{
		Call,
		Put,
		//TODO add more
	};

	extern DPP_API enum class OptionExerciseType
	{
		European,
		American,
		//TODO add more
	};

	extern DPP_API class TriMatrix
	{
	public:
		friend class TriMatrixBuilder;

		DPP_API const std::vector< std::vector< Node > >& getMatrix() const;
		DPP_API double getDt() const;
		DPP_API size_t getSteps() const;

	private:
		const double m_dt;
		std::vector< std::vector< Node > > m_matrix;

		TriMatrix( const size_t steps, const double timeStep = 1. );
		const Node* getParentHeads( const Node& thisNode ) const;
		const Node* getHeads( const Node& thisNode ) const;
		const Node* getParentTails( const Node& thisNode ) const;
		const Node* getTails( const Node& thisNode ) const;
	};

}