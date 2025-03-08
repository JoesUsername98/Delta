#include <gtest/gtest.h>

#include "engine_factory.h"

using namespace DPP;

#pragma region Binomial
#pragma region PV
TEST( engine, EuroCallPV )
{
	TradeData trd ( OptionExerciseType::European, OptionPayoffType::Call, 105., 1. );
	MarketData mkt ( 1.2, 100., 0.05 );
	CalcData calc ( Calculation::PV, 3 );

	auto engine = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	engine->run();

	EXPECT_TRUE( engine->m_errors.empty() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::PV ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::PV ), 48.170795535239122 );
}
TEST( engine, EuroPutPV )
{
	TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
	MarketData mkt ( 1.2, 100., 0.05 );
	CalcData calc ( Calculation::PV, 3 );

	auto engine = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	engine->run();

	EXPECT_TRUE( engine->m_errors.empty() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::PV ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::PV ), 48.049738737522595 );
}
TEST( engine, AmerCallPV )
{
	TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
	MarketData mkt ( 1.2, 100., 0.05 );
	CalcData calc ( Calculation::PV, 3 );

	auto engine = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	engine->run();

	EXPECT_TRUE( engine->m_errors.empty() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::PV ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::PV ), 48.170795535239122 );
}
TEST( engine, AmerPutPV )
{
	TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
	MarketData mkt ( 1.2, 100., 0.05 );
	CalcData calc ( Calculation::PV, 3 );

	auto engine = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	engine->run();

	EXPECT_TRUE( engine->m_errors.empty() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::PV ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::PV ), 48.758203318346808 );
}
#pragma endregion
#pragma endregion