#include <gtest/gtest.h>

#include <Delta++/engine_factory.h>

using namespace DPP;

constexpr int STEPS = 252;
constexpr int SIMS = 1000;
const double  TOL = 1e-11;
const double VOL = 0.2;

#pragma region Binomial
#pragma region PV
TEST( engine_mc, EuroCallPV )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::PV, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 7.6452181227433922;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ).value(),
        expected_pv,
        TOL );
}
TEST( engine_mc, EuroPutPV )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::PV, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 7.3935839856863295;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ).value() ,
        expected_pv, 
        TOL );
}
TEST( engine_mc, AmerCallPV )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::PV, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 13.285859566122195;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ).value(),
        expected_pv, 
        TOL );
}
TEST( engine_mc, AmerPutPV )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::PV, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 15.916122098965991;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ).value(),
        expected_pv, 
        TOL );
}
#pragma endregion
#pragma region Delta
TEST( engine_mc, EuroCallDelta )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Delta, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = 0.54737887910056138;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ).value(),
        expected_delta, 
        TOL );
}
TEST( engine_mc, EuroPutDelta )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Delta, STEPS, 100);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = -0.42705641267555094;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ).value(),
        expected_delta, 
        TOL );
}
TEST( engine_mc, AmerCallDelta )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Delta, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = 0.95036855524625352;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ).value(),
        expected_delta, 
        TOL );
}
TEST( engine_mc, AmerPutDelta )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Delta, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = -0.86839139105919472;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ).value(),
        expected_delta, 
        TOL );
}
#pragma endregion
#pragma region Gamma
TEST( engine_mc, EuroCallGamma )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Gamma, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = 0.019382314570957071;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ).value(),
        expected_gamma, 
        TOL );
}
TEST( engine_mc, EuroPutGamma )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Gamma, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = 0.0093692421999884701;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ).value(),
        expected_gamma, 
        TOL );
}
TEST( engine_mc, AmerCallGamma )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Gamma, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = 0.048928977300072063;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ).value(),
        expected_gamma, 
        TOL );
}
TEST( engine_mc, AmerPutGamma )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Gamma, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = -0.0086576513939071731;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ).value(),
        expected_gamma, 
        TOL );
}
#pragma endregion
#pragma region Rho
TEST( engine_mc, EuroCallRho )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Rho, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
    
    const double expected_rho = 46.834431815156563;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ).value(),
        expected_rho, 
        TOL );
}
TEST( engine_mc, EuroPutRho )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Rho, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
    
    const double expected_rho = -52.627033596496453;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ).value(),
        expected_rho, 
        TOL );
}
TEST( engine_mc, AmerCallRho )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Rho, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
    
    const double expected_rho = 52.194860913964547;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ).value(),
        expected_rho, 
        TOL);
}
TEST( engine_mc, AmerPutRho )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Rho, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
    
    const double expected_rho = -44.049588054060251;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ).value(),
        expected_rho, 
        TOL );
}
#pragma endregion
#pragma region Vega
TEST( engine_mc, EuroCallVega )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Vega, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
    
    const double expected_vega = 37.131002628629517;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ).value(),
        expected_vega, 
        TOL );
}
TEST( engine_mc, EuroPutVega )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Vega, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
    
    const double expected_vega = 37.505354914658007;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ).value(),
        expected_vega, 
        TOL );
}
TEST( engine_mc, AmerCallVega )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Vega, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega) != engine_mc->m_results.end() );
    
    const double expected_vega = 75.911396454998581;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ).value(),
        expected_vega, 
        TOL );
}
TEST( engine_mc, AmerPutVega )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Vega, STEPS, SIMS);

    auto engine_res_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    EXPECT_TRUE(engine_res_mc.has_value());
    auto& engine_mc = engine_res_mc.value();
    engine_mc->run();

    EXPECT_TRUE( !engine_mc->hasAnyErrors() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
    
    const double expected_vega = 61.955894105174991;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ).value(),
        expected_vega, 
        TOL );
}
#pragma endregion
#pragma endregion