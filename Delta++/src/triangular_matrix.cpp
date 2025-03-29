#include "triangular_matrix.h"

namespace DPP
{
	TriMatrix::TriMatrix(const size_t steps, const double timeStep ) :
		m_dt( timeStep ),
		m_steps( steps ),
		m_matrix( std::vector< Node >( index( steps, steps ) + 1 ) )
	{};
	
	std::size_t TriMatrix::index(const std::size_t row, const std::size_t col) const
	{
		if (row > col)
			throw std::invalid_argument( "Invalid access: Not in the upper triangle." );

		return ( col * (col + 1) ) / 2 + row;
	}

	const std::vector<Node>& TriMatrix::getMatrix() const { return m_matrix; }
	double TriMatrix::getDt() const { return m_dt; }
	const Node* TriMatrix::getParentHeads( const Node& thisNode ) const 
	{ 
		if( thisNode.m_timeStep == 0 )
			return nullptr; 

		return &m_matrix[ index( thisNode.m_timeStep - 1 , thisNode.m_downMoves ) ];
	};
	const Node* TriMatrix::getHeads( const Node& thisNode ) const 
	{ 
		if ( thisNode.m_timeStep == m_steps )
			return nullptr;

		return &m_matrix[ index( thisNode.m_downMoves, thisNode.m_timeStep + 1 ) ];
	};
	const Node* TriMatrix::getParentTails( const Node& thisNode ) const 
	{
		if ( thisNode.m_timeStep == 0 )
			return nullptr;

		if ( thisNode.m_downMoves == 0 )
			return nullptr;

		return &m_matrix[ index( thisNode.m_downMoves - 1, thisNode.m_timeStep - 1 ) ];
	};
	const Node* TriMatrix::getTails( const Node& thisNode ) const 
	{
		if ( thisNode.m_timeStep == m_steps )
			return nullptr;

		if ( thisNode.m_downMoves == m_steps )
			return nullptr;

		return &m_matrix[ index( thisNode.m_downMoves + 1, thisNode.m_timeStep + 1 ) ];
	};
} 