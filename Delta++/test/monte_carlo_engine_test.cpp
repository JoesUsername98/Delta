#include <gtest/gtest.h>

#include "engine_factory.h"

using namespace DPP;

constexpr int STEPS = 252;
constexpr int SIMS = 1000;
const double  TOL = 1e-12;
const double VOL = 0.2;

#pragma region Binomial
#pragma region PV
TEST( engine_mc, EuroCallPV )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::PV, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 8.8579609930122007;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ),
        expected_pv,
        TOL );
}
TEST( engine_mc, EuroPutPV )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::PV, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 8.0262996692140085;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ),
        expected_pv, 
        TOL );
}
TEST( engine_mc, AmerCallPV )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::PV, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 8.8579609930122007;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::PV ),
        expected_pv, 
        TOL );
}
TEST( engine_mc, AmerPutPV )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::PV, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::PV ) != engine_mc->m_results.end() );
    
    const double expected_pv = 8.0262996692140085;
      
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

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = 0.5766342164062781;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ),
        expected_delta, 
        TOL );
}
TEST( engine_mc, EuroPutDelta )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Delta, STEPS, 100);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = -0.44448965144675423;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ),
        expected_delta, 
        TOL );
}
TEST( engine_mc, AmerCallDelta )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Delta, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = 0.5766342164062781;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Delta ),
        expected_delta, 
        TOL );
}
TEST( engine_mc, AmerPutDelta )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Delta, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Delta ) != engine_mc->m_results.end() );
    
    const double expected_delta = -0.43047329255745304;
      
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

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = 0.021468585153925801;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ),
        expected_gamma, 
        TOL );
}
TEST( engine_mc, EuroPutGamma )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Gamma, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = 0.011397510064285576;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ),
        expected_gamma, 
        TOL );
}
TEST( engine_mc, AmerCallGamma )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Gamma, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = 0.021468585153925801;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Gamma ),
        expected_gamma, 
        TOL );
}
TEST( engine_mc, AmerPutGamma )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Gamma, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Gamma ) != engine_mc->m_results.end() );
    
    const double expected_gamma = 0.011397510064285576;
      
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

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
    
    const double expected_rho = 48.530374106512042;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ),
        expected_rho, 
        TOL );
}
TEST( engine_mc, EuroPutRho )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Rho, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
    
    const double expected_rho = -50.925872202166289;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ),
        expected_rho, 
        TOL );
}
TEST( engine_mc, AmerCallRho )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Rho, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
    
    const double expected_rho = 48.530374106512042;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Rho ),
        expected_rho, 
        TOL );
}
TEST( engine_mc, AmerPutRho )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Rho, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Rho ) != engine_mc->m_results.end() );
    
    const double expected_rho = -50.925872202166289;
      
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

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
    
    const double expected_vega = 44.227996466966246;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ),
        expected_vega, 
        TOL );
}
TEST( engine_mc, EuroPutVega )
{
    TradeData trd ( OptionExerciseType::European, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Vega, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
    
    const double expected_vega = 39.561856778366966;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ),
        expected_vega, 
        TOL );
}
TEST( engine_mc, AmerCallVega )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Call, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Vega, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega) != engine_mc->m_results.end() );
    
    const double expected_vega = 44.227996466966246;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ),
        expected_vega, 
        TOL );
}
TEST( engine_mc, AmerPutVega )
{
    TradeData trd ( OptionExerciseType::American, OptionPayoffType::Put, 105., 1. );
    MarketData mkt ( VOL, 100., 0.05 );
    CalcData calc ( Calculation::Vega, STEPS, SIMS);

    auto engine_mc = EngineFactory::getEngine<MonteCarloEngine>( mkt, trd, calc );
    engine_mc->run();

    EXPECT_TRUE( engine_mc->m_errors.empty() );
    EXPECT_EQ( engine_mc->m_results.size() , 1 );
    EXPECT_TRUE( engine_mc->m_results.find( Calculation::Vega ) != engine_mc->m_results.end() );
    
    const double expected_vega = 39.561856778366966;
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ),
        expected_vega, 
        TOL );
}
#pragma endregion
#pragma endregion