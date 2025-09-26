#include <gtest/gtest.h>

#include "engine_factory.h"

using namespace DPP;

 constexpr int STEPS = 252;
 constexpr int SIMS = 1000;
 const double TOL = 1e-4;

#pragma region Binomial
#pragma region PV
TEST( engine_mc, EuroCallPV )
{
	TradeData trd ( OptionExerciseType::European, OptionPayoffType::Call, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::PV, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ), 7.639102364717548, TOL );
}
TEST( engine_mc, EuroPutPV )
{
	TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::PV, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ), 7.5817251742220275, TOL );
}
TEST( engine_mc, AmerCallPV )
{
	TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::PV, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ), 7.639102364717548, TOL );
}
TEST( engine_mc, AmerPutPV )
{
	TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::PV, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ), 7.5817251742220275, TOL );
}
#pragma endregion
#pragma region Delta
TEST( engine_mc, EuroCallDelta )
{
	TradeData trd ( OptionExerciseType::European, OptionPayoffType::Call, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Delta, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ), 0.52837133107700307, TOL );
}
TEST( engine_mc, EuroPutDelta )
{
	TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Delta, STEPS, 100);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ), -0.48654610202007209, TOL );
}
TEST( engine_mc, AmerCallDelta )
{
	TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Delta, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ), 0.52837133107700307, TOL );
}
TEST( engine_mc, AmerPutDelta )
{
	TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Delta, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ), -0.47099333655367914, TOL );
}
#pragma endregion
#pragma region Gamma
TEST( engine_mc, EuroCallGamma )
{
	TradeData trd ( OptionExerciseType::European, OptionPayoffType::Call, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Gamma, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ), 0.026363250718870468, TOL );
}
TEST( engine_mc, EuroPutGamma )
{
	TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Gamma, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ), 0.016369604042548502, TOL );
}
TEST( engine_mc, AmerCallGamma )
{
	TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Gamma, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ), 0.026363250718870468, TOL );
}
TEST( engine_mc, AmerPutGamma )
{
	TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Gamma, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ), 0.016369604042548502, TOL );
}
#pragma endregion
#pragma region Rho
TEST( engine_mc, EuroCallRho )
{
	TradeData trd ( OptionExerciseType::European, OptionPayoffType::Call, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Rho, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ), 45.158204921438561, TOL );
}
TEST( engine_mc, EuroPutRho )
{
	TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Rho, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ), -54.702280971096954, TOL );
}
TEST( engine_mc, AmerCallRho )
{
	TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Rho, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ), 45.158204921438561, TOL );
}
TEST( engine_mc, AmerPutRho )
{
	TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Rho, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ), -54.702280971096954, TOL );
}
#pragma endregion
#pragma region Vega
TEST( engine_mc, EuroCallVega )
{
	TradeData trd ( OptionExerciseType::European, OptionPayoffType::Call, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Vega, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ), 37.543515022240825, TOL );
}
TEST( engine_mc, EuroPutVega )
{
	TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Vega, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ), 38.545149649737894, TOL );
}
TEST( engine_mc, AmerCallVega )
{
	TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Vega, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ), 37.543515022240825, TOL );
}
TEST( engine_mc, AmerPutVega )
{
	TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
	MarketData mkt ( 0.2, 100., 0.05 );
	CalcData calc ( Calculation::Vega, STEPS, SIMS);

	auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
	engine_mc->run();

	EXPECT_TRUE( engine_mc->m_errors.empty() );
	EXPECT_EQ( engine_mc->m_results.size() , 1 );
	EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
	EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ), 38.545149649737894, TOL );
}
#pragma endregion
#pragma endregion
