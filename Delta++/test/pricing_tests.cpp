#include <gtest/gtest.h>

#include "engine_factory.h"

using namespace DPP;

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
	EXPECT_EQ( mat[0].m_data.m_optionValue, 48.170795535239122 );
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
	EXPECT_EQ(mat[0].m_data.m_optionValue, 48.049738737522595);
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
	EXPECT_EQ(mat[0].m_data.m_optionValue, 48.170795535239122);
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
	EXPECT_EQ(mat[0].m_data.m_optionValue, 48.758203318346808);
}


