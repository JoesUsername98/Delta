#include <gtest/gtest.h>

#include <Delta++/engine_factory.h>

using namespace DPP;

#pragma region Binomial
#pragma region PV
TEST( engine, EuroCallPV )
{
	TradeData trd {
    .m_optionExerciseType = OptionExerciseType::European,
    .m_optionPayoffType = OptionPayoffType::Call,
    .m_strike = 105.,
    .m_maturity = 1.
};
	MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.,
    .m_interestRate = 0.05
};
	CalcData calc {
    .m_calc = Calculation::PV,
    .m_steps= 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::PV ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::PV ).value(), 48.170795535239122 );
}
TEST( engine, EuroPutPV )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::European,
    .m_optionPayoffType = OptionPayoffType::Put,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::PV,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::PV ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::PV ).value(), 48.049738737522595 );
}
TEST( engine, AmerCallPV )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::American,
    .m_optionPayoffType = OptionPayoffType::Call,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::PV,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::PV ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::PV ).value(), 48.170795535239122 );
}
TEST( engine, AmerPutPV )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::American,
    .m_optionPayoffType = OptionPayoffType::Put,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::PV,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::PV ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::PV ).value(), 48.758203318346808 );
}
#pragma endregion
#pragma region Delta
TEST( engine, EuroCallDelta )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::European,
    .m_optionPayoffType = OptionPayoffType::Call,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Delta,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Delta ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Delta ).value(), 0.75613560127931123 );
}
TEST( engine, EuroPutDelta )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::European,
    .m_optionPayoffType = OptionPayoffType::Put,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Delta,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Delta ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Delta ).value(), -0.24507496669782114 );
}
TEST( engine, AmerCallDelta )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::American,
    .m_optionPayoffType = OptionPayoffType::Call,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Delta,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Delta ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Delta ).value(), 0.75613560127931123 );
}
TEST( engine, AmerPutDelta )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::American,
    .m_optionPayoffType = OptionPayoffType::Put,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Delta,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Delta ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Delta ).value(), -0.24503301188877202 );
}
#pragma endregion
#pragma region Gamma
TEST( engine, EuroCallGamma )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::European,
    .m_optionPayoffType = OptionPayoffType::Call,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Gamma,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Gamma ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Gamma ).value(), 0.0075613560128005020 );
}
TEST( engine, EuroPutGamma )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::European,
    .m_optionPayoffType = OptionPayoffType::Put,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Gamma,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Gamma ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Gamma ).value(), -0.0024507496669698980 );
}
TEST( engine, AmerCallGamma )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::American,
    .m_optionPayoffType = OptionPayoffType::Call,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Gamma,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Gamma ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Gamma ).value(), 0.0075613560128005020 );
}
TEST( engine, AmerPutGamma )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::American,
    .m_optionPayoffType = OptionPayoffType::Put,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Gamma,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Gamma ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Gamma ).value(), -0.0024503301188687487 );
}
#pragma endregion
#pragma region Rho
TEST( engine, EuroCallRho )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::European,
    .m_optionPayoffType = OptionPayoffType::Call,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Rho,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Rho ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Rho ).value(), 27.795392260804164 );
}
TEST( engine, EuroPutRho )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::European,
    .m_optionPayoffType = OptionPayoffType::Put,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Rho,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Rho ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Rho ).value(), -72.211865338645254 );
}
TEST( engine, AmerCallRho )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::American,
    .m_optionPayoffType = OptionPayoffType::Call,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Rho,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Rho ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Rho ).value(), 27.795392260804164 );
}
TEST( engine, AmerPutRho )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::American,
    .m_optionPayoffType = OptionPayoffType::Put,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Rho,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Rho ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Rho ).value(), -59.348811877772789 );
}
#pragma endregion
#pragma region Vega
TEST( engine, EuroCallVega )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::European,
    .m_optionPayoffType = OptionPayoffType::Call,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Vega,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Vega ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Vega ).value(), 34.296496709994528 );
}
TEST( engine, EuroPutVega )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::European,
    .m_optionPayoffType = OptionPayoffType::Put,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Vega,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Vega ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Vega ).value(), 34.296496709992397 );
}
TEST( engine, AmerCallVega )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::American,
    .m_optionPayoffType = OptionPayoffType::Call,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Vega,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Vega ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Vega ).value(), 34.296496709994528 );
}
TEST( engine, AmerPutVega )
{
    TradeData trd {
    .m_optionExerciseType = OptionExerciseType::American,
    .m_optionPayoffType = OptionPayoffType::Put,
    .m_strike = 105.0,
    .m_maturity = 1.0
};
    MarketData mkt {
    .m_vol = 1.2,
    .m_underlyingPrice = 100.0,
    .m_interestRate = 0.05
};
    CalcData calc {
    .m_calc = Calculation::Vega,
    .m_steps = 3
};

	auto engine_res = EngineFactory::getEngine<BinomialEngine>( mkt, trd, calc );
	EXPECT_TRUE(engine_res.has_value());
	auto& engine = engine_res.value();
	engine->run();

	EXPECT_TRUE( !engine->hasAnyErrors() );
	EXPECT_EQ( engine->m_results.size() , 1 );
	EXPECT_TRUE( engine->m_results.find( Calculation::Vega ) != engine->m_results.end() );
	EXPECT_EQ( engine->m_results.at( Calculation::Vega ).value(), 34.592394876462151 );
}
#pragma endregion
#pragma endregion
