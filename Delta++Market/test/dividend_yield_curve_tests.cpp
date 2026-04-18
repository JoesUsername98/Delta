#include <gtest/gtest.h>

#include <cmath>
#include <span>
#include <vector>

#include <Delta++Market/dividend_yield_curve.h>
#include <Delta++Market/yield_curve.h>

using namespace DPP;

namespace
{
    RateQuote qFlat(const double tenor, const double ratePct)
    {
        return {.tenor = tenor, .rate = ratePct, .date = "2024-01-01", .source = "Test"};
    }

    /// y = C - P = A - B*K with A = S*exp(-qT), B = A/S so F = S; pick C,P > 0 and C-P = y.
    void addSyntheticPillar(std::vector<std::vector<PutCallMidPoint>>& rowBacking,
                            std::vector<ParityExpiryPillar>& pillars,
                            const std::string& exp,
                            const double T,
                            const double S,
                            const double q)
    {
        const double A = S * std::exp(-q * T);
        const double B = A / S;
        std::vector<double> Ks{80.0, 100.0, 120.0};
        std::vector<double> calls;
        std::vector<double> puts;
        calls.reserve(Ks.size());
        puts.reserve(Ks.size());
        for (const double K : Ks)
        {
            const double y = A - B * K;
            double C = 20.0;
            double P = C - y;
            if (P <= 0.0)
            {
                P = 5.0;
                C = P + y;
                if (C <= 0.0)
                {
                    C = std::abs(y) + 10.0;
                    P = C - y;
                }
            }
            calls.push_back(C);
            puts.push_back(P);
        }

        std::vector<PutCallMidPoint> rows;
        rows.reserve(Ks.size());
        for (size_t i = 0; i < Ks.size(); ++i)
        {
            rows.push_back(PutCallMidPoint{
                .expirationDate = exp,
                .yearsToExpiry = T,
                .strike = Ks[i],
                .callMid = calls[i],
                .putMid = puts[i],
            });
        }
        rowBacking.push_back(std::move(rows));
        const auto& r = rowBacking.back();
        pillars.push_back(ParityExpiryPillar{
            .tYears = T,
            .expirationDate = exp,
            .rows = std::span<const PutCallMidPoint>(r.data(), r.size()),
        });
    }
}

TEST(Market_DividendYieldCurve, EmptyPillarsError)
{
    std::vector<RateQuote> quotes = {qFlat(0.25, 4.0), qFlat(1.0, 4.0), qFlat(5.0, 4.5)};
    auto yc = YieldCurve::build(quotes);
    ASSERT_TRUE(yc.has_value());

    std::vector<ParityExpiryPillar> empty;
    auto r = DividendYieldCurve::buildFromParity(100.0, *yc, empty);
    EXPECT_FALSE(r.has_value());
}

TEST(Market_DividendYieldCurve, SinglePillarConstantQ)
{
    std::vector<RateQuote> quotes = {qFlat(0.25, 4.0), qFlat(1.0, 4.0), qFlat(5.0, 4.5)};
    auto yc = YieldCurve::build(quotes);
    ASSERT_TRUE(yc.has_value());

    const double S = 100.0;
    const double qIn = 0.03;
    std::vector<std::vector<PutCallMidPoint>> backing;
    std::vector<ParityExpiryPillar> pillars;
    addSyntheticPillar(backing, pillars, "2025-01-01", 1.0, S, qIn);

    auto r = DividendYieldCurve::buildFromParity(S, *yc, pillars);
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->pillars.size(), 1u);
    EXPECT_NEAR(r->pillars[0].q, qIn, 1e-6);

    const DividendYieldCurve& div = r->curve;
    EXPECT_NEAR(div.q(0.5), r->pillars[0].q, 1e-9);
    EXPECT_NEAR(div.q(2.0), r->pillars[0].q, 1e-9);
}

TEST(Market_DividendYieldCurve, TwoPillarsSplineInterpolates)
{
    std::vector<RateQuote> quotes = {qFlat(0.25, 4.0), qFlat(1.0, 4.0), qFlat(5.0, 4.5)};
    auto yc = YieldCurve::build(quotes);
    ASSERT_TRUE(yc.has_value());

    const double S = 100.0;
    const double qShort = 0.01;
    const double qLong = 0.04;
    std::vector<std::vector<PutCallMidPoint>> backing;
    std::vector<ParityExpiryPillar> pillars;
    addSyntheticPillar(backing, pillars, "2025-01-01", 0.5, S, qShort);
    addSyntheticPillar(backing, pillars, "2026-01-01", 2.0, S, qLong);

    auto r = DividendYieldCurve::buildFromParity(S, *yc, pillars);
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->pillars.size(), 2u);
    EXPECT_NEAR(r->pillars[0].q, qShort, 1e-5);
    EXPECT_NEAR(r->pillars[1].q, qLong, 1e-5);

    const DividendYieldCurve& div = r->curve;
    EXPECT_NEAR(div.q(0.5), qShort, 1e-5);
    EXPECT_NEAR(div.q(2.0), qLong, 1e-5);

    const double qMid = div.q(1.25);
    EXPECT_GT(qMid, (std::min)(qShort, qLong) - 1e-4);
    EXPECT_LT(qMid, (std::max)(qShort, qLong) + 1e-4);
}
