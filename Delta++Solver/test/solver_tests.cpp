#include <gtest/gtest.h>
#include <cmath>

#include <Delta++Solver/interpolation.h>
#include <Delta++Solver/bootstrapper.h>

using namespace DPP;

TEST(Solver_Interpolation, LinearBasic)
{
    LinearInterpolator interp({0.0, 1.0, 2.0}, {0.0, 1.0, 0.0});
    EXPECT_DOUBLE_EQ(interp(0.5), 0.5);
    EXPECT_DOUBLE_EQ(interp(1.5), 0.5);
    // Flat extrapolation
    EXPECT_DOUBLE_EQ(interp(-1.0), 0.0);
    EXPECT_DOUBLE_EQ(interp(3.0), 0.0);
}

TEST(Solver_Interpolation, CubicSplineEndpoints)
{
    CubicSplineInterpolator interp({0.0, 1.0, 2.0, 3.0}, {0.0, 1.0, 0.0, 1.0});
    EXPECT_DOUBLE_EQ(interp(0.0), 0.0);
    EXPECT_DOUBLE_EQ(interp(1.0), 1.0);
    EXPECT_DOUBLE_EQ(interp(2.0), 0.0);
    EXPECT_DOUBLE_EQ(interp(3.0), 1.0);
}

TEST(Solver_Bootstrap, BasicDiscountFactors)
{
    std::vector<BootstrapInput> inputs = {
        {1.0, 5.0},   // 1Y at 5%
        {2.0, 5.0},   // 2Y at 5%
        {5.0, 5.0},   // 5Y at 5%
    };

    auto result = bootstrap(inputs);
    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result->tenors.size(), 3);
    EXPECT_NEAR(result->discountFactors[0], 1.0 / 1.05, 1e-12);
    EXPECT_NEAR(result->discountFactors[1], 1.0 / (1.05 * 1.05), 1e-12);
    EXPECT_NEAR(result->discountFactors[2], 1.0 / std::pow(1.05, 5.0), 1e-12);
}

TEST(Solver_Bootstrap, EmptyInputReturnsError)
{
    auto result = bootstrap({});
    EXPECT_FALSE(result.has_value());
}

TEST(Solver_Bootstrap, NegativeTenorReturnsError)
{
    auto result = bootstrap({{-1.0, 5.0}});
    EXPECT_FALSE(result.has_value());
}
