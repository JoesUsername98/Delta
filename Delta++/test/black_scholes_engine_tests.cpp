#include <gtest/gtest.h>

#include <Delta++/engine_factory.h>

using namespace DPP;

//https://www.math.drexel.edu/~pg/fin/VanillaCalculator.html
#pragma region PV
TEST(engine_BS, EuroCallPV)
{
	TradeData trd( OptionExerciseType::European, OptionPayoffType::Call, 5., 3. );
	MarketData mkt(.2, 4., .25);
	CalcData calc( Calculation::PV, 69 /*doesn't matter*/ );

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::PV) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::PV).value(), 1.6675924577040089);
}
TEST(engine_BS, EuroPutPV)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Put, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::PV, 69 /*doesn't matter*/);

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::PV) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::PV).value(), 0.029425221409081936);
}
#pragma endregion
#pragma region Delta
TEST(engine_BS, EuroCallDelta)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Call, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Delta, 69 /*doesn't matter*/);

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Delta) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Delta).value(), 0.95487770772622005 );
}
TEST(engine_BS, EuroPutDelta)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Put, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Delta, 69 /*doesn't matter*/);

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Delta) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Delta).value(), -0.045122292273779951);
}
#pragma endregion
#pragma region Gamma
TEST(engine_BS, EuroCallGamma)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Call, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Gamma, 69 /*doesn't matter*/);

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Gamma) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Gamma).value(), 0.068556080613463410);
}
TEST(engine_BS, EuroPutGamma)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Put, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Gamma, 69 /*doesn't matter*/);

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Gamma) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Gamma).value(), 0.068556080613463410);
}
#pragma endregion
#pragma region Vega
TEST(engine_BS, EuroCallVega)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Call, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Vega, 69 /*doesn't matter*/);

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Vega) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Vega).value(), 0.65813837388924867);
}
TEST(engine_BS, EuroPutVega)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Put, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Vega, 69 /*doesn't matter*/);

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Vega) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Vega).value(), 0.65813837388924867);
}
#pragma endregion
#pragma region Rho
TEST(engine_BS, EuroCallRho)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Call, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Rho, 69 /*doesn't matter*/);

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Rho) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Rho).value(), 6.4557551196026148);
}
TEST(engine_BS, EuroPutRho)
{
	TradeData trd(OptionExerciseType::European, OptionPayoffType::Put, 5., 3.);
	MarketData mkt(.2, 4., .25);
	CalcData calc(Calculation::Rho, 69 /*doesn't matter*/);

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Rho) != engine->m_results.end());
	EXPECT_EQ(engine->m_results.at(Calculation::Rho).value(), -0.62974317151260595);
}
#pragma endregion
