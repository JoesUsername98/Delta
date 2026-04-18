#include <gtest/gtest.h>

#include <Delta++/engine_factory.h>
#include <Delta++/abstract_engine.h>
#include "test_curve_utils.h"

using namespace DPP;

namespace
{
    double scalar(const CalculationResult& r)
    {
        const auto s = scalarOrError(r);
        EXPECT_TRUE(s.has_value()) << s.error();
        return s.value_or(0.0);
    }
}

//https://www.math.drexel.edu/~pg/fin/VanillaCalculator.html
#pragma region PV
TEST(engine_BS, EuroCallPV)
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 5.0,
        .m_maturity = 3.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(4.0, 0.2, DPPTest::makeFlatCurve(0.25));
    CalcData calc {
        .m_calc = Calculation::PV,
        .m_steps = 69
    };

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::PV) != engine->m_results.end());
	// σ comes from AH flat stub (`localVolAt`), not a raw literal; use near-equality vs Drexel reference PV.
	EXPECT_NEAR(scalar(engine->m_results.at(Calculation::PV)), 1.6675924577040089, 1e-6);
}
TEST(engine_BS, EuroPutPV)
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 5.0,
        .m_maturity = 3.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(4.0, 0.2, DPPTest::makeFlatCurve(0.25));
    CalcData calc {
        .m_calc = Calculation::PV,
        .m_steps = 69
    };

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::PV) != engine->m_results.end());
	EXPECT_NEAR(scalar(engine->m_results.at(Calculation::PV)), 0.029425221409081936, 1e-6);
}
#pragma endregion
#pragma region Delta
TEST(engine_BS, EuroCallDelta)
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 5.0,
        .m_maturity = 3.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(4.0, 0.2, DPPTest::makeFlatCurve(0.25));
    CalcData calc {
        .m_calc = Calculation::Delta,
        .m_steps = 69
    };

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Delta) != engine->m_results.end());
	EXPECT_EQ(scalar(engine->m_results.at(Calculation::Delta)), 0.95487770772622005 );
}
TEST(engine_BS, EuroPutDelta)
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 5.0,
        .m_maturity = 3.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(4.0, 0.2, DPPTest::makeFlatCurve(0.25));
    CalcData calc {
        .m_calc = Calculation::Delta,
        .m_steps = 69
    };

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Delta) != engine->m_results.end());
	EXPECT_EQ(scalar(engine->m_results.at(Calculation::Delta)), -0.045122292273779951);
}
#pragma endregion
#pragma region Gamma
TEST(engine_BS, EuroCallGamma)
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 5.0,
        .m_maturity = 3.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(4.0, 0.2, DPPTest::makeFlatCurve(0.25));
    CalcData calc {
        .m_calc = Calculation::Gamma,
        .m_steps = 69
    };

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Gamma) != engine->m_results.end());
	EXPECT_EQ(scalar(engine->m_results.at(Calculation::Gamma)), 0.068556080613463410);
}
TEST(engine_BS, EuroPutGamma)
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 5.0,
        .m_maturity = 3.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(4.0, 0.2, DPPTest::makeFlatCurve(0.25));
    CalcData calc {
        .m_calc = Calculation::Gamma,
        .m_steps = 69
    };

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Gamma) != engine->m_results.end());
	EXPECT_EQ(scalar(engine->m_results.at(Calculation::Gamma)), 0.068556080613463410);
}
#pragma endregion
#pragma region Vega
TEST(engine_BS, EuroCallVega)
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 5.0,
        .m_maturity = 3.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(4.0, 0.2, DPPTest::makeFlatCurve(0.25));
    CalcData calc {
        .m_calc = Calculation::Vega,
        .m_steps = 69
    };

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Vega) != engine->m_results.end());
	EXPECT_EQ(scalar(engine->m_results.at(Calculation::Vega)), 0.65813837388924867);
}
TEST(engine_BS, EuroPutVega)
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 5.0,
        .m_maturity = 3.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(4.0, 0.2, DPPTest::makeFlatCurve(0.25));
    CalcData calc {
        .m_calc = Calculation::Vega,
        .m_steps = 69
    };

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::Vega) != engine->m_results.end());
	EXPECT_EQ(scalar(engine->m_results.at(Calculation::Vega)), 0.65813837388924867);
}
#pragma endregion
#pragma region Rho
TEST(engine_BS, EuroCallRho)
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 5.0,
        .m_maturity = 3.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(4.0, 0.2, DPPTest::makeFlatCurve(0.25));
    CalcData calc {
        .m_calc = Calculation::RhoParallel,
        .m_steps = 69
    };

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::RhoParallel) != engine->m_results.end());
	EXPECT_NEAR(scalar(engine->m_results.at(Calculation::RhoParallel)), 6.4557551196026148, 1e-3);
}
TEST(engine_BS, EuroPutRho)
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 5.0,
        .m_maturity = 3.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(4.0, 0.2, DPPTest::makeFlatCurve(0.25));
    CalcData calc {
        .m_calc = Calculation::RhoParallel,
        .m_steps = 69
    };

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE(!engine->hasAnyErrors());
	EXPECT_EQ(engine->m_results.size(), 1);
	EXPECT_TRUE(engine->m_results.find(Calculation::RhoParallel) != engine->m_results.end());
	EXPECT_NEAR(scalar(engine->m_results.at(Calculation::RhoParallel)), -0.62974317151260595, 1e-3);
}
#pragma endregion
#pragma region RejectsUnsupported
TEST(engine_BS, RejectsAmericanOptions) {
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::American,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 5.0,
        .m_maturity = 3.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(4.0, 0.2, DPPTest::makeFlatCurve(0.25));
    CalcData calc {
        .m_calc = Calculation::PV,
        .m_steps = 69
    };

	auto engine_res = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
	EXPECT_FALSE(engine_res.has_value());
	EXPECT_EQ(engine_res.error(), "BlackScholes can only handle European Exercise");
}
#pragma endregion
