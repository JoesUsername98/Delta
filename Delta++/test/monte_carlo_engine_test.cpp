#include <gtest/gtest.h>

#include "engine_factory.h"

using namespace DPP;

#if ((defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)))
    #define WINDOWS
#endif
#if (defined(__unix__) || defined(__unix))
    #define UNIX
#endif

constexpr int STEPS = 252;
constexpr int SIMS = 1000;
const double  TOL = 1e-4;
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
    
    const double expected_pv = 
      #ifdef UNIX
        7.8695509840142934;
      #endif
      #ifdef WINDOWS
        7.9017204341837317;
      #endif
      
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
    
    const double expected_pv =
      #ifdef UNIX
        7.7434294974051436;
      #endif 
      #ifdef WINDOWS
        7.7694460325428851;
      #endif 
      
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
    
    const double expected_pv =
      #ifdef UNIX
        7.8695509840142934;
      #endif 
      #ifdef WINDOWS
        7.9017204341837317;
      #endif 
      
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
    
    const double expected_pv =
      #ifdef UNIX
        7.7434294974051436;
      #endif 
      #ifdef WINDOWS
        7.7694460325428851;
      #endif 
      
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
    
    const double expected_delta =
      #ifdef UNIX
        0.53445975242509203;
      #endif 
      #ifdef WINDOWS
        0.54181477865509198;
      #endif 
      
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
    
    const double expected_delta =
      #ifdef UNIX
        -0.44059505103840824;
      #endif 
      #ifdef WINDOWS
        -0.43000837838543227;
      #endif 
      
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
    
    const double expected_delta =
      #ifdef UNIX
        0.53445975242509203;
      #endif 
      #ifdef WINDOWS
        0.54181477865509198;
      #endif 
      
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
    
    const double expected_delta =
      #ifdef UNIX
        -0.46559235816675049;
      #endif 
      #ifdef WINDOWS
        -0.45829886108704621;
      #endif 
      
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
    
    const double expected_gamma =
      #ifdef UNIX
        0.027401089922215682;
      #endif 
      #ifdef WINDOWS
        0.033162112117489428;
      #endif 
      
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
    
    const double expected_gamma =
      #ifdef UNIX
        0.017400568816297479;
      #endif 
      #ifdef WINDOWS
        0.023160975720081467;
      #endif 
      
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
    
    const double expected_gamma =
      #ifdef UNIX
        0.027401089922215682;
      #endif 
      #ifdef WINDOWS
        0.033162112117489428;
      #endif 
      
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
    
    const double expected_gamma =
      #ifdef UNIX
        0.017400568816297479;
      #endif 
      #ifdef WINDOWS
        0.023160975720081467;
      #endif 
      
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
    
    const double expected_rho =
      #ifdef UNIX
        45.322161482612522;
      #endif 
      #ifdef WINDOWS
        46.021589934129906;
      #endif 
      
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
    
    const double expected_rho =
      #ifdef UNIX
        -54.140736310080406;
      #endif 
      #ifdef WINDOWS
        -53.441237624738761;
      #endif 
      
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
    
    const double expected_rho =
      #ifdef UNIX
        45.322161482612522;
      #endif 
      #ifdef WINDOWS
        46.021589934129906;
      #endif 
      
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
    
    const double expected_rho =
      #ifdef UNIX
        -54.140736310080406;
      #endif 
      #ifdef WINDOWS
        -53.441237624738761;
      #endif 
      
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
    
    const double expected_vega =
      #ifdef UNIX
        39.090166256124981;
      #endif 
      #ifdef WINDOWS
        39.188275070183479;
      #endif 
      
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
    
    const double expected_vega =
      #ifdef UNIX
        39.08280766552403;
      #endif 
      #ifdef WINDOWS
        39.125087578375606;
      #endif 
      
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
    
    const double expected_vega =
      #ifdef UNIX
        39.090166256124981;
      #endif 
      #ifdef WINDOWS
        39.188275070183479;
      #endif 
      
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
    
    const double expected_vega =
      #ifdef UNIX
        39.08280766552403;
      #endif 
      #ifdef WINDOWS
        39.125087578375606;
      #endif 
      
    EXPECT_NEAR( engine_mc->m_results.at( Calculation::Vega ),
        expected_vega, 
        TOL );
}
#pragma endregion
#pragma endregion