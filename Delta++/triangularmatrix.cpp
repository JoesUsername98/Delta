#include "pch.h"

#include "triangularmatrix.h"

namespace DPP
{
	double ExpectableState::getDiscountRate() const { return 1. / (1. + m_interestRate); }
	double State::getProbabilityTails() const { return 1. - m_probabilityHeads; }

	TriMatrix::TriMatrix(const size_t steps, const double timeStep ) :
		m_dt(timeStep),
		m_matrix(steps + 1)
	{
		for (size_t i = 0; i < steps + 1; ++i)
			m_matrix[i] = std::vector< Node >(i + 1);
	};
	const std::vector<std::vector<Node>>& TriMatrix::getMatrix() const { return m_matrix; }
	double TriMatrix::getDt() const { return m_dt; }
	size_t TriMatrix::getSteps() const { return m_matrix.size(); }
	const Node* TriMatrix::getParentHeads( const Node& thisNode ) const 
	{ 
		if( thisNode.m_timeStep == 0 )
			return nullptr; 

		return &m_matrix[ thisNode.m_timeStep - 1 ][ thisNode.m_downMoves ];
	};
	const Node* TriMatrix::getHeads( const Node& thisNode ) const 
	{ 
		if ( thisNode.m_timeStep == getSteps() - 1 )
			return nullptr;

		return &m_matrix[ thisNode.m_timeStep + 1 ][ thisNode.m_downMoves ];
	};
	const Node* TriMatrix::getParentTails( const Node& thisNode ) const 
	{
		if ( thisNode.m_timeStep == 0 )
			return nullptr;

		if ( thisNode.m_downMoves == 0 )
			return nullptr;

		return &m_matrix[ thisNode.m_timeStep - 1 ][ thisNode.m_downMoves - 1 ];
	};
	const Node* TriMatrix::getTails( const Node& thisNode ) const 
	{
		if ( thisNode.m_timeStep == getSteps() - 1 )
			return nullptr;

		if ( thisNode.m_downMoves == getSteps() - 1 )
			return nullptr;

		return &m_matrix[ thisNode.m_timeStep + 1 ][ thisNode.m_downMoves + 1 ];
	};
} 