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
#pragma region Pricing
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
#pragma region BlackScholes
//https://www.math.drexel.edu/~pg/fin/VanillaCalculator.html
#pragma region PV
TEST(engine_BS, EuroCallPV)
{
	TradeData trd( OptionExerciseType::European, OptionPayoffType::Call, 5., 3. );
	MarketData mkt(.2, 4., .25);
	CalcData calc( Calculation::PV, 69 /*doesn't matter*/ );

	auto engine = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	engine->run();

	EXPECT_TRUE(engine->m_errors.empty());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::PV) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::PV), 1.6675924577040089);
}
TEST(engine_BS, EuroPutPV)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Put, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::PV, 69 /*doesn't matter*/);

	auto engine = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	engine->run();

	EXPECT_TRUE(engine->m_errors.empty());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::PV) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::PV), 0.029425221409081936);
}
#pragma endregion
#pragma region Delta
TEST(engine_BS, EuroCallDelta)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Call, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Delta, 69 /*doesn't matter*/);

	auto engine = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	engine->run();

	EXPECT_TRUE(engine->m_errors.empty());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Delta) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Delta), 0.95487770772622005 );
}
TEST(engine_BS, EuroPutDelta)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Put, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Delta, 69 /*doesn't matter*/);

	auto engine = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	engine->run();

	EXPECT_TRUE(engine->m_errors.empty());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Delta) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Delta), -0.045122292273779951);
}
#pragma endregion
#pragma region Gamma
TEST(engine_BS, EuroCallGamma)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Call, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Gamma, 69 /*doesn't matter*/);

	auto engine = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	engine->run();

	EXPECT_TRUE(engine->m_errors.empty());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Gamma) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Gamma), 0.068556080613463410);
}
TEST(engine_BS, EuroPutGamma)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Put, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Gamma, 69 /*doesn't matter*/);

	auto engine = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	engine->run();

	EXPECT_TRUE(engine->m_errors.empty());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Gamma) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Gamma), 0.068556080613463410);
}
#pragma endregion
#pragma region Vega
TEST(engine_BS, EuroCallVega)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Call, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Vega, 69 /*doesn't matter*/);

	auto engine = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	engine->run();

	EXPECT_TRUE(engine->m_errors.empty());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Vega) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Vega), 0.65813837388924867);
}
TEST(engine_BS, EuroPutVega)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Put, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Vega, 69 /*doesn't matter*/);

	auto engine = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	engine->run();

	EXPECT_TRUE(engine->m_errors.empty());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Vega) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Vega), 0.65813837388924867);
}
#pragma endregion
#pragma region Rho
TEST(engine_BS, EuroCallRho)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Call, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Rho, 69 /*doesn't matter*/);

	auto engine = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	engine->run();

	EXPECT_TRUE(engine->m_errors.empty());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Rho) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Rho), 6.4557551196026148);
}
TEST(engine_BS, EuroPutRho)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Put, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Rho, 69 /*doesn't matter*/);

	auto engine = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	engine->run();

	EXPECT_TRUE(engine->m_errors.empty());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Rho) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Rho), -0.62974317151260595);
}
#pragma endregion
#pragma endregion
#pragma endregion

