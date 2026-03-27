#include <gtest/gtest.h>

#include <cmath>

#include <Delta++/abstract_engine.h>
#include <Delta++/engine_factory.h>
#include <Delta++Market/yield_curve.h>

using namespace DPP;

namespace
{
    YieldCurve makeFlatCurve(double flatZeroRate)
    {
        const double ratePct = (std::exp(flatZeroRate) - 1.0) * 100.0;
        std::vector<RateQuote> quotes = {
            {.tenor = 1.0, .rate = ratePct},
            {.tenor = 30.0, .rate = ratePct},
        };
        auto yc = YieldCurve::build(quotes);
        EXPECT_TRUE(yc.has_value()) << yc.error();
        return yc.value();
    }

    double scalar(const CalculationResult& r)
    {
        const auto s = scalarOrError(r);
        EXPECT_TRUE(s.has_value()) << s.error();
        return s.value_or(0.0);
    }
}

TEST(YieldCurvePricers, FlatCurveMatchesConstantRate_BlackScholesPV)
{
    TradeData trd{.m_optionExerciseType = OptionExerciseType::European,
                  .m_optionPayoffType = OptionPayoffType::Call,
                  .m_strike = 105.0,
                  .m_maturity = 1.0};

    MarketData mkt{.m_vol = 0.2, .m_underlyingPrice = 100.0, .m_interestRate = 0.05};
    MarketData mktCurve = mkt;
    mktCurve.m_yieldCurve = makeFlatCurve(mkt.m_interestRate);

    CalcData calc{.m_calc = Calculation::PV, .m_steps = 10};

    auto e0 = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
    auto e1 = EngineFactory::getEngine<BlackScholesEngine>(mktCurve, trd, calc);
    ASSERT_TRUE(e0.has_value());
    ASSERT_TRUE(e1.has_value());
    e0.value()->run();
    e1.value()->run();

    const double pv0 = scalar(e0.value()->m_results.at(Calculation::PV));
    const double pv1 = scalar(e1.value()->m_results.at(Calculation::PV));
    EXPECT_NEAR(pv0, pv1, 1e-12);
}

TEST(YieldCurvePricers, FlatCurveMatchesConstantRate_BinomialPV)
{
    TradeData trd{.m_optionExerciseType = OptionExerciseType::European,
                  .m_optionPayoffType = OptionPayoffType::Call,
                  .m_strike = 105.0,
                  .m_maturity = 1.0};

    MarketData mkt{.m_vol = 1.2, .m_underlyingPrice = 100.0, .m_interestRate = 0.05};
    MarketData mktCurve = mkt;
    mktCurve.m_yieldCurve = makeFlatCurve(mkt.m_interestRate);

    CalcData calc{.m_calc = Calculation::PV, .m_steps = 25};

    auto e0 = EngineFactory::getEngine<BinomialEngine>(mkt, trd, calc);
    auto e1 = EngineFactory::getEngine<BinomialEngine>(mktCurve, trd, calc);
    ASSERT_TRUE(e0.has_value());
    ASSERT_TRUE(e1.has_value());
    e0.value()->run();
    e1.value()->run();

    const double pv0 = scalar(e0.value()->m_results.at(Calculation::PV));
    const double pv1 = scalar(e1.value()->m_results.at(Calculation::PV));
    EXPECT_NEAR(pv0, pv1, 1e-10);
}

TEST(YieldCurvePricers, FlatCurveMatchesConstantRate_MonteCarloPV)
{
    TradeData trd{.m_optionExerciseType = OptionExerciseType::European,
                  .m_optionPayoffType = OptionPayoffType::Call,
                  .m_strike = 105.0,
                  .m_maturity = 1.0};

    MarketData mkt{.m_vol = 0.2, .m_underlyingPrice = 100.0, .m_interestRate = 0.05};
    MarketData mktCurve = mkt;
    mktCurve.m_yieldCurve = makeFlatCurve(mkt.m_interestRate);

    CalcData calc{.m_calc = Calculation::PV, .m_pathSchemeType = PathSchemeType::Exact, .m_steps = 500, .m_sims = 20'000, .m_seed = 7};

    auto e0 = EngineFactory::getEngine<MonteCarloEngine>(mkt, trd, calc);
    auto e1 = EngineFactory::getEngine<MonteCarloEngine>(mktCurve, trd, calc);
    ASSERT_TRUE(e0.has_value());
    ASSERT_TRUE(e1.has_value());
    e0.value()->run();
    e1.value()->run();

    const double pv0 = scalar(e0.value()->m_results.at(Calculation::PV));
    const double pv1 = scalar(e1.value()->m_results.at(Calculation::PV));
    EXPECT_NEAR(pv0, pv1, 1e-8);
}

TEST(YieldCurvePricers, KeyRateRhoReturnsVector_BlackScholes)
{
    TradeData trd{.m_optionExerciseType = OptionExerciseType::European,
                  .m_optionPayoffType = OptionPayoffType::Call,
                  .m_strike = 105.0,
                  .m_maturity = 1.0};

    MarketData mkt{.m_vol = 0.2, .m_underlyingPrice = 100.0, .m_interestRate = 0.05};
    mkt.m_yieldCurve = makeFlatCurve(mkt.m_interestRate);

    CalcData calc{.m_calc = Calculation::Rho, .m_steps = 10};

    auto e = EngineFactory::getEngine<BlackScholesEngine>(mkt, trd, calc);
    ASSERT_TRUE(e.has_value());
    e.value()->run();

    const auto& res = e.value()->m_results.at(Calculation::Rho);
    ASSERT_TRUE(res.has_value()) << res.error();
    const auto* curve = std::get_if<CurveRho>(&res.value());
    ASSERT_NE(curve, nullptr);
    const auto& tenors = mkt.m_yieldCurve->tenors();
    const double T = trd.m_maturity;
    const auto n_applicable = static_cast<std::size_t>(std::count_if(tenors.begin(), tenors.end(), [&](double x) { return x <= T; }));
    EXPECT_EQ(curve->size(), n_applicable);
}

