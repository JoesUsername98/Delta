#include "tri_matrix_builder.h"

#include <math.h>
#include <limits>

namespace DPP
{

#define RTN_BUILDER_ERR(msg)  \
		m_hasError = true;  \
		m_errorMsg = std::string( msg ); \
		return *this;  

	TriMatrixBuilder::TriMatrixBuilder( const size_t steps, const double timeStep ) :
		m_timeSteps( steps ),
		m_timeStep( timeStep ),
		m_hasError( false ),
		m_errorMsg( "" ),
		m_mustCalcDeltaHedging( false ),
		m_mustCalcOptiomalStoppingTime( false )
	{
	}

	TriMatrixBuilder& TriMatrixBuilder::withUnderlyingValueAndVolatility( const double initialPrice, const double vol )
	{
		const double upFactor = exp( vol * sqrt( m_timeStep ) );
		return withUnderlyingValueAndUpFactor( initialPrice, upFactor );
	}
	TriMatrixBuilder& TriMatrixBuilder::withUnderlyingValueAndUpFactor( const double initialPrice, const double upFactor )
	{
		if ( m_hasError ) 
			return *this;

		m_initialPrice = initialPrice;

		if (m_initialPrice < 0 )
		{
			RTN_BUILDER_ERR( "initialPrice cannot be 0" );
		}

		m_upFactor = upFactor;

		if (m_upFactor <= 0 )
		{
			RTN_BUILDER_ERR( "upFactor cannot be 0 or less" );
		}

		m_downFactor = 1 / m_upFactor;

		if (m_upFactor < m_downFactor)
		{
			RTN_BUILDER_ERR( "upFactor cannot be less than downFactor" );
		}

		return *this;
	}
	TriMatrixBuilder& TriMatrixBuilder::withInterestRate(const double constantInterestRate)
	{
		if ( m_hasError ) 
			return *this;

		m_interestRate = constantInterestRate;
		const double interestRate = pow(1. + constantInterestRate, m_timeStep) - 1.;
		m_discountRate = 1. / (1. + interestRate);

		return *this;
	}
	TriMatrixBuilder& TriMatrixBuilder::withPayoff(const OptionPayoffType optionType, const double strikePrice)
	{
		if ( m_hasError ) 
			return *this;

		m_strikePrice = strikePrice; 
		m_optionType = optionType;
		
		switch (m_optionType) 
		{
			case OptionPayoffType::Call:
				m_setPayoff = [strike = m_strikePrice] (Node& node) { node.m_data.m_payoff = node.m_data.m_underlyingValue - strike; };
				break;
			case OptionPayoffType::Put:
				m_setPayoff = [strike = m_strikePrice](Node& node) { node.m_data.m_payoff = strike - node.m_data.m_underlyingValue; };
				break;
			default:
				RTN_BUILDER_ERR( "Invalid payoff strategy" );
		}

		return *this;
	}
	TriMatrixBuilder& TriMatrixBuilder::withRiskNuetralProb()
	{
		if ( m_hasError ) 
			return *this;

		if ( m_upFactor <= 1 + m_interestRate ) 
		{ 
			RTN_BUILDER_ERR( "u > 1 + r to prevent arbitrage" );
		}

		const double growthFactor = exp( m_interestRate * m_timeStep ); // Hull 12.6
		m_probabilityHeads = (growthFactor - m_downFactor) / (m_upFactor - m_downFactor); // Hull 12.5

		return *this;
	}
	TriMatrixBuilder& TriMatrixBuilder::withPremium( const OptionExerciseType exerciseType )
	{
		if ( m_hasError )
			return *this;

		m_exerciseType = exerciseType;

		m_calcExpPV = [discountRate = m_discountRate, pHeads = m_probabilityHeads] ( const Node* heads, const Node* tails )
		{ 
			return discountRate * ( heads->m_data.m_optionValue * pHeads + tails->m_data.m_optionValue * ( 1. - pHeads ) ); 
		};

		switch ( m_exerciseType )
		{
			case OptionExerciseType::European:
				m_setExerciseValue = [discountRate = m_discountRate] (Node& node, const Node* heads, const Node* tails ) { node.m_data.m_optionValue = std::max(node.m_data.m_payoff, 0.); };
				break;
			case OptionExerciseType::American:
				m_setExerciseValue = [discountRate = m_discountRate, pHeads = m_probabilityHeads, expPV = m_calcExpPV]
					(Node& node, const Node* heads, const Node* tails) 
					{ node.m_data.m_optionValue = std::max( node.m_data.m_payoff, (!heads || !tails) ? 0. : expPV( heads, tails ) ); };
				break;
			default:
				RTN_BUILDER_ERR("Invalid payoff strategy");
		}

		return *this;
	}
	TriMatrixBuilder& TriMatrixBuilder::withDelta()
	{
		if ( m_hasError ) 
			return *this;

		m_mustCalcDeltaHedging = true;

		return *this;
	}

	// Todo move to BAMP engine
	void TriMatrixBuilder::calcDeltaHedging( TriMatrix& result )
	{
		for ( int step = m_timeSteps - 2; step >= 0; step-- )
			for ( int downMoves = step; downMoves >= 0; downMoves-- )
			{
				Node& node = result.m_matrix[result.index( downMoves, step ) ];
				const Node* heads = result.getHeads( node );
				const Node* tails = result.getTails( node );

				if ( heads->m_data.m_underlyingValue == tails->m_data.m_underlyingValue )
				{
					m_hasError = true;
					m_errorMsg = "Underlying Value of heads and tails node are the same leading to a divide by 0 error"; 
					return;
				}

				node.m_data.m_deltaHedging = ( heads->m_data.m_optionValue - tails->m_data.m_optionValue )
					/ ( heads->m_data.m_underlyingValue - tails->m_data.m_underlyingValue );
			}
	}

	TriMatrixBuilder& TriMatrixBuilder::withPsuedoOptimalStoppingTime()
	{
		if ( m_hasError ) 
			return *this;

		if ( m_exerciseType != OptionExerciseType::American )
			return *this;

		m_mustCalcOptiomalStoppingTime = true;

		return *this;
	}

	// Todo move to BAMP engine
	void TriMatrixBuilder::calcOptiomalStoppingTime( TriMatrix& result ) const
	{
		//Diag Traverseal
		for ( size_t timeStep = 0; timeStep < m_timeSteps; ++timeStep )
		{
			size_t optimalExEarlyPutTimeStep = std::numeric_limits<size_t>::max();
			for ( size_t diagMoves = 0; diagMoves < m_timeSteps - 1 - timeStep ; ++diagMoves )
			{
				State& state = result.m_matrix[ result.index( diagMoves, timeStep + diagMoves ) ].m_data;
				if ( state.m_optionValue == state.m_payoff && optimalExEarlyPutTimeStep == std::numeric_limits<size_t>::max() )
					optimalExEarlyPutTimeStep = timeStep + diagMoves;
				state.m_optimalExerciseTime = optimalExEarlyPutTimeStep;
			}
		}
	}

	TriMatrix TriMatrixBuilder::build()
	{
		TriMatrix result( m_timeSteps, m_timeStep );

		for ( int step = m_timeSteps; step >= 0; step-- )
			for ( int downMoves = step; downMoves >= 0; downMoves-- )
			{
				const size_t noOfHeads = step - downMoves;
				Node& node = result.m_matrix[ result.index( downMoves, step ) ];

				node.m_timeStep = step;
				node.m_downMoves = downMoves;

				node.m_data.m_underlyingValue = m_initialPrice * pow( m_upFactor, noOfHeads ) * pow( m_downFactor, downMoves );

				m_setPayoff(node);

				const Node* heads = result.getHeads( node );
				const Node* tails = result.getTails( node );
				if ( node.m_timeStep == m_timeSteps || m_exerciseType == OptionExerciseType::American )
					m_setExerciseValue( node, heads, tails );
				else
					node.m_data.m_optionValue = m_calcExpPV( heads, tails );
			}

		if( m_mustCalcDeltaHedging )
			calcDeltaHedging( result );

		if ( m_mustCalcOptiomalStoppingTime )
			calcOptiomalStoppingTime( result );

		if ( m_hasError )
			throw std::runtime_error( m_errorMsg );

		return result;
	}
	const std::string& TriMatrixBuilder::getErrorMsg() const
	{
		return m_errorMsg;
	}
}