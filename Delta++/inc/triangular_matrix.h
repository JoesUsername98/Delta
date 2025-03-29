#pragma once

#include <vector>
#include <optional>
#include <exception>
#include <string>

#include "enums.h"
#include <stdexcept>

namespace DPP
{

	struct State
	{
		double m_payoff;
		double m_optionValue;
		double m_deltaHedging;
		double m_underlyingValue;

		std::optional<size_t> m_optimalExerciseTime;
	};

	struct Node
	{
		State m_data;
		size_t m_timeStep;
		size_t m_downMoves;
	};

	class TriMatrix
	{
	public:
		friend class TriMatrixBuilder;

		const std::vector< Node > & getMatrix() const;
		double getDt() const;
		const size_t m_steps;

	private:
		const double m_dt;
		std::vector< Node > m_matrix;

		TriMatrix( const size_t steps, const double timeStep = 1. );

		size_t index(const size_t row, const size_t col) const;

		const Node* getParentHeads( const Node& thisNode ) const;
		const Node* getHeads( const Node& thisNode ) const;
		const Node* getParentTails( const Node& thisNode ) const;
		const Node* getTails( const Node& thisNode ) const;
	};

}