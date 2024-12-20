#include <gtest/gtest.h>

#include "engine.h"

using namespace DPP;

#pragma region TriMatBldErr
TEST( TriMatBldErr, initialPrice)
{
	const size_t stepsIn = 2;
	auto buildResult = TriMatrixBuilder::create(stepsIn, 30. / stepsIn)
		.withUnderlyingValueAndVolatility(-4, 0.2)
		.withInterestRate(0.25)
		.withPayoff(OptionPayoffType::Call, 5)
		.withRiskNuetralProb()
		.withPremium(OptionExerciseType::European)
		.withDelta()
		.withPsuedoOptimalStoppingTime();

	EXPECT_TRUE( buildResult.m_hasError );

	EXPECT_EQ( buildResult.getErrorMsg(), "initialPrice cannot be 0");
}
TEST( TriMatBldErr, upFactorLessThan0 )
{
	const size_t stepsIn = 2;
	auto buildResult = TriMatrixBuilder::create(stepsIn, 30. / stepsIn)
		.withUnderlyingValueAndUpFactor( 4, -1.)
		.withInterestRate(0.25)
		.withPayoff(OptionPayoffType::Call, 5)
		.withRiskNuetralProb()
		.withPremium(OptionExerciseType::European)
		.withDelta()
		.withPsuedoOptimalStoppingTime();

	EXPECT_TRUE(buildResult.m_hasError);

	EXPECT_EQ(buildResult.getErrorMsg(), "upFactor cannot be 0 or less");
}
TEST( TriMatBldErr, upFactorLessThan1 )
{
	const size_t stepsIn = 2;
	auto buildResult = TriMatrixBuilder::create(stepsIn, 30. / stepsIn)
		.withUnderlyingValueAndUpFactor(4, .1)
		.withInterestRate(0.25)
		.withPayoff(OptionPayoffType::Call, 5)
		.withRiskNuetralProb()
		.withPremium(OptionExerciseType::European)
		.withDelta()
		.withPsuedoOptimalStoppingTime();

	EXPECT_TRUE(buildResult.m_hasError);

	EXPECT_EQ(buildResult.getErrorMsg(), "upFactor cannot be less than downFactor");
}
TEST(TriMatBldErr, irArb)
{
	const size_t stepsIn = 2;
	auto buildResult = TriMatrixBuilder::create(stepsIn, 30. / stepsIn)
		.withUnderlyingValueAndUpFactor(4, 2)
		.withInterestRate(25)
		.withPayoff(OptionPayoffType::Call, 5)
		.withRiskNuetralProb()
		.withPremium(OptionExerciseType::European)
		.withDelta()
		.withPsuedoOptimalStoppingTime();

	EXPECT_TRUE(buildResult.m_hasError);

	EXPECT_EQ(buildResult.getErrorMsg(), "u > 1 + r to prevent arbitrage");
}
#pragma endregion
#pragma region pricing
TEST( Pricing, EuroCall)
{
	const size_t stepsIn = 3;
	auto buildResult = TriMatrixBuilder::create(stepsIn, 1. / stepsIn)
		.withUnderlyingValueAndVolatility( 100., 1.2 )
		.withInterestRate(0.05)
		.withPayoff(OptionPayoffType::Call, 105.)
		.withRiskNuetralProb()
		.withPremium(OptionExerciseType::European)
		.withDelta()
		.withPsuedoOptimalStoppingTime();

	EXPECT_TRUE(!buildResult.m_hasError);
	EXPECT_EQ(buildResult.getErrorMsg(), "");

	const auto result = buildResult.build();
	const auto mat = result.getMatrix();
	EXPECT_EQ( mat[0][0].m_data.m_optionValue, 48.170795535239122 );
}
TEST( Pricing, EuroPut)
{
	const size_t stepsIn = 3;
	auto buildResult = TriMatrixBuilder::create(stepsIn, 1. / stepsIn)
		.withUnderlyingValueAndVolatility(100., 1.2)
		.withInterestRate(0.05)
		.withPayoff(OptionPayoffType::Put, 105.)
		.withRiskNuetralProb()
		.withPremium(OptionExerciseType::European)
		.withDelta()
		.withPsuedoOptimalStoppingTime();

	EXPECT_TRUE(!buildResult.m_hasError);
	EXPECT_EQ(buildResult.getErrorMsg(), "");

	const auto result = buildResult.build();
	const auto mat = result.getMatrix();
	EXPECT_EQ(mat[0][0].m_data.m_optionValue, 48.049738737522595);
}
TEST(pricing, AmerCall)
{
	const size_t stepsIn = 3;
	auto buildResult = TriMatrixBuilder::create(stepsIn, 1. / stepsIn)
		.withUnderlyingValueAndVolatility(100., 1.2)
		.withInterestRate(0.05)
		.withPayoff(OptionPayoffType::Call, 105.)
		.withRiskNuetralProb()
		.withPremium(OptionExerciseType::American)
		.withDelta()
		.withPsuedoOptimalStoppingTime();

	EXPECT_TRUE(!buildResult.m_hasError);
	EXPECT_EQ(buildResult.getErrorMsg(), "");

	const auto result = buildResult.build();
	const auto mat = result.getMatrix();
	EXPECT_EQ(mat[0][0].m_data.m_optionValue, 48.170795535239122);
}
TEST(pricing, AmerPut)
{
	const size_t stepsIn = 3;
	auto buildResult = TriMatrixBuilder::create(stepsIn, 1. / stepsIn)
		.withUnderlyingValueAndVolatility(100., 1.2)
		.withInterestRate(0.05)
		.withPayoff(OptionPayoffType::Put, 105.)
		.withRiskNuetralProb()
		.withPremium(OptionExerciseType::American)
		.withDelta()
		.withPsuedoOptimalStoppingTime();

	EXPECT_TRUE(!buildResult.m_hasError);
	EXPECT_EQ(buildResult.getErrorMsg(), "");

	const auto result = buildResult.build();
	const auto mat = result.getMatrix();
	EXPECT_EQ(mat[0][0].m_data.m_optionValue, 48.758203318346808);
}
#pragma endregion
#pragma region Engine
#pragma region PV
TEST( engine, EuroCallPV )
{
	TradeData trd ( OptionExerciseType::European, OptionPayoffType::Call, 105., 1. );
	MarketData mkt ( 1.2, 100., 0.05 );
	CalcData calc ( Calculation::PV, 3 );

	auto engine = AbstractEngine::getEngine<BinomialEngine>( mkt, trd, calc );
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

	auto engine = AbstractEngine::getEngine<BinomialEngine>( mkt, trd, calc );
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

	auto engine = AbstractEngine::getEngine<BinomialEngine>( mkt, trd, calc );
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

	auto engine = AbstractEngine::getEngine<BinomialEngine>( mkt, trd, calc );
	engine->run();

	EXPECT_TRUE( engine->m_errors.empty() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::PV ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::PV ), 48.758203318346808 );
}
#pragma endregion
#pragma endregion
