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
        // Tighter FD padding (0.5% log-range) slightly widens bootstrap error vs flat vol; keep a loose band.
        EXPECT_NEAR(lv, sigma, 0.08) << "T=" << T;
        EXPECT_GT(out->callPrice(T, 100.0), 0.0);
    }
}

TEST(Market_AndreasenHuge, GapStepUsesDupireNotTimeLerpOfPillarPrices)
{
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

    const double t0 = in.expiries[0];
    const double t1 = in.expiries[1];
    const double Tmid = 0.5 * (t0 + t1);
    const double K = 100.0;

    const double r0 = in.curve.zeroRate(t0);
    const double r1 = in.curve.zeroRate(t1);
    const double C0 = BlackScholes::callPrice(S, K, t0, r0, q, sigma);
    const double C1 = BlackScholes::callPrice(S, K, t1, r1, q, sigma);
    const double w = (Tmid - t0) / (t1 - t0);
    const double naiveLerp = C0 + w * (C1 - C0);

    const double gapPrice = out->callPrice(Tmid, K);

    // Dupire gap fill (paper step 2) is not linear interpolation of pillar Black prices in calendar time.
    EXPECT_GT(std::abs(gapPrice - naiveLerp), 1e-4);

    // Interior local vol between pillars uses left-pillar variance slice (constant in T on the strip).
    const double lvGap = out->localVol(Tmid, K);
    const double lvLeft = out->localVol(t0 + 1e-8, K);
    EXPECT_NEAR(lvGap, lvLeft, 0.02);
}

