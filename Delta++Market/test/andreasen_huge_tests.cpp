#include <gtest/gtest.h>

#include <Delta++Market/andreasen_huge.h>
#include <Delta++BlackScholes/black_scholes.h>

using namespace DPP;

TEST(Market_AndreasenHuge, SyntheticConstantVolRecoversRoughlyConstantLocalVol)
{
    // Build a near-flat yield curve.
    std::vector<RateQuote> quotes = {
        {.tenor = 0.25, .rate = 4.0, .date = "2024-01-01", .source = "Test"},
        {.tenor = 1.0,  .rate = 4.0, .date = "2024-01-01", .source = "Test"},
        {.tenor = 5.0,  .rate = 4.0, .date = "2024-01-01", .source = "Test"},
        {.tenor = 10.0, .rate = 4.0, .date = "2024-01-01", .source = "Test"},
    };
    auto yc = YieldCurve::build(quotes);
    ASSERT_TRUE(yc.has_value()) << yc.error();

    constexpr double S = 100.0;
    constexpr double sigma = 0.25;
    constexpr double q = 0.0;

    AHInput in{
        .spot = S,
        .curve = *yc,
        .expiries = {0.25, 0.5, 1.0},
    };

    // Same strike grid per expiry to make finite differences meaningful.
    std::vector<double> Ks = {60, 70, 80, 90, 100, 110, 120, 130, 140};
    for (double T : in.expiries)
    {
        const double r = in.curve.zeroRate(T);
        std::vector<double> Cs;
        Cs.reserve(Ks.size());
        for (double K : Ks)
            Cs.push_back(BlackScholes::callPrice(S, K, T, r, q, sigma));
        in.strikes.push_back(Ks);
        in.callPrices.push_back(std::move(Cs));
    }

    auto out = bootstrapAndreasenHuge(in);
    ASSERT_TRUE(out.has_value()) << out.error();

    // Check ATM local vol is close-ish to sigma at each expiry.
    for (double T : in.expiries)
    {
        const double lv = out->localVol(T, 100.0);
        EXPECT_NEAR(lv, sigma, 0.05) << "T=" << T;
        EXPECT_GT(out->callPrice(T, 100.0), 0.0);
    }
}

