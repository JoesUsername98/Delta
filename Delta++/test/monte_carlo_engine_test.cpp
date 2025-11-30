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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 7.657011201984667;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 7.4810055016788821;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 13.371611621392097;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 15.924228122126411;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = 0.55022140081792603;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = -0.41929948742677858;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = 0.94204371931060038;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = -0.86825443415032133;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = 0.026616961412988083;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = 0.016611451885695239;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = 0.045806410967347944;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = -0.0086335803749317819;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
    
    const double expected_rho = 47.106188662356715;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
    
    const double expected_rho = -52.355660144722464;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
    
    const double expected_rho = 51.599446462042486;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
    
    const double expected_rho = -44.206806499909845;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
    
    const double expected_vega = 37.32248716204829;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
    
    const double expected_vega = 37.809516891168649;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega) != engine_mc->m_results.end() );
    
    const double expected_vega = 76.516609193764225;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ),
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

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
    
    const double expected_vega = 61.88000253122059;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ),
        expected_vega, 
        TOL );
}
#pragma endregion
#pragma endregion