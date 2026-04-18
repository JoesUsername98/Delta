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

constexpr int STEPS = 252;
constexpr int SIMS = 1000;
const double  TOL = 1e-11;
const double VOL = 0.2;

#pragma region Binomial
#pragma region PV
TEST( engine_mc, EuroCallPV )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::PV,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 7.6452181227433922;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::PV )),
        expected_pv,
        TOL );
}
TEST( engine_mc, EuroPutPV )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::PV,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 7.3935839856863295;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::PV )) ,
        expected_pv, 
        TOL );
}
TEST( engine_mc, AmerCallPV )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::American,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::PV,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 13.285859566122195;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::PV )),
        expected_pv, 
        TOL );
}
TEST( engine_mc, AmerPutPV )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::American,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::PV,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 15.916122098965991;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::PV )),
        expected_pv, 
        TOL );
}
#pragma endregion
#pragma region Delta
TEST( engine_mc, EuroCallDelta )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::Delta,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = 0.54737887910056138;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::Delta )),
        expected_delta, 
        TOL );
}
TEST( engine_mc, EuroPutDelta )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::Delta,
        .m_steps = STEPS,
        .m_sims = 100
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = -0.42705641267555094;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::Delta )),
        expected_delta, 
        TOL );
}
TEST( engine_mc, AmerCallDelta )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::American,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::Delta,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = 0.95036855524625352;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::Delta )),
        expected_delta, 
        TOL );
}
TEST( engine_mc, AmerPutDelta )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::American,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::Delta,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = -0.86839139105919472;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::Delta )),
        expected_delta, 
        TOL );
}
#pragma endregion
#pragma region Gamma
TEST( engine_mc, EuroCallGamma )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::Gamma,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = 0.019382314570957071;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::Gamma )),
        expected_gamma, 
        TOL );
}
TEST( engine_mc, EuroPutGamma )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::Gamma,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = 0.0093692421999884701;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::Gamma )),
        expected_gamma, 
        TOL );
}
TEST( engine_mc, AmerCallGamma )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::American,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::Gamma,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = 0.048928977300072063;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::Gamma )),
        expected_gamma, 
        TOL );
}
TEST( engine_mc, AmerPutGamma )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::American,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::Gamma,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = -0.0086576513939071731;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::Gamma )),
        expected_gamma, 
        TOL );
}
#pragma endregion
#pragma region Rho
TEST( engine_mc, EuroCallRho )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::RhoParallel,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::RhoParallel ) != engine_mc->m_results.end() );
    
    const double expected_rho = 46.834431815156563;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::RhoParallel )),
        expected_rho, 
        1e-6 );
}
TEST( engine_mc, EuroPutRho )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::RhoParallel,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::RhoParallel ) != engine_mc->m_results.end() );
    
    const double expected_rho = -52.627033596496453;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::RhoParallel )),
        expected_rho, 
        1e-6 );
}
TEST( engine_mc, AmerCallRho )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::American,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::RhoParallel,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::RhoParallel ) != engine_mc->m_results.end() );
    
    const double expected_rho = 52.194860913964547;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::RhoParallel )),
        expected_rho, 
        1e-6);
}
TEST( engine_mc, AmerPutRho )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::American,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::RhoParallel,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::RhoParallel ) != engine_mc->m_results.end() );
    
    const double expected_rho = -44.049588054060251;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::RhoParallel )),
        expected_rho, 
        1e-6 );
}
#pragma endregion
#pragma region Vega
TEST( engine_mc, EuroCallVega )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::Vega,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
    
    const double expected_vega = 37.131002628629517;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::Vega )),
        expected_vega, 
        TOL );
}
TEST( engine_mc, EuroPutVega )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::European,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::Vega,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
    
    const double expected_vega = 37.505354914658007;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::Vega )),
        expected_vega, 
        TOL );
}
TEST( engine_mc, AmerCallVega )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::American,
        .m_optionPayoffType = OptionPayoffType::Call,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::Vega,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega) != engine_mc->m_results.end() );
    
    const double expected_vega = 75.911396454998581;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::Vega )),
        expected_vega, 
        TOL );
}
TEST( engine_mc, AmerPutVega )
{
    TradeData trd {
        .m_optionExerciseType = OptionExerciseType::American,
        .m_optionPayoffType = OptionPayoffType::Put,
        .m_strike = 105.0,
        .m_maturity = 1.0
    };
    MarketData mkt = DPPTest::makeFlatMarket(100.0, VOL, DPPTest::makeFlatCurve(0.05));
    CalcData calc {
        .m_calc = Calculation::Vega,
        .m_steps = STEPS,
        .m_sims = SIMS
    };

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
    
    const double expected_vega = 61.955894105174991;
      
    EXPECT_NEAR( scalar(engine_mc->m_results.at( Calculation::Vega )),
        expected_vega, 
        TOL );
}
#pragma endregion
#pragma endregion