#include <gtest/gtest.h>

#include "engine_factory.h"

using namespace DPP;

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
