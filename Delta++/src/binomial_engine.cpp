#include "Delta++/tri_matrix_builder.h"
#include "Delta++/binomial_engine.h"
#include <algorithm>
#include <cmath>
#include <ranges>

using namespace std::string_literals;
namespace DPP
{
    CalculationResult BinomialEngine::calcPV( const CalcData& calc ) const
    {
        const double dt = m_trd.m_maturity / calc.m_steps;
        const double sigmaTree =
            m_mkt.hasLocalVolSurface()
                ? m_mkt.localVolAt(m_trd.m_maturity, m_trd.m_strike)
                : m_mkt.m_vol;

        TriMatrixBuilder build_result = TriMatrixBuilder::create(calc.m_steps, dt)
            .withUnderlyingValueAndVolatility(m_mkt.m_underlyingPrice, sigmaTree);

        const double u = std::exp(sigmaTree * std::sqrt(dt));
        const double d = 1.0 / u;

        std::vector<double> discountRatesByStep;
        std::vector<double> probabilityHeadsByStep;
        discountRatesByStep.reserve(calc.m_steps);
        probabilityHeadsByStep.reserve(calc.m_steps);
        for (size_t step = 0; step < static_cast<size_t>(calc.m_steps); ++step)
        {
            const double t0 = static_cast<double>(step) * dt;
            const double r = m_mkt.zeroRate(t0);
            const double q = m_mkt.dividendYield(t0);
            const double interest_rate = std::pow(1. + r, dt) - 1.;
            const double discountRate = 1. / (1. + interest_rate);
            const double growthFactor = std::exp((r - q) * dt);
            const double p = (growthFactor - d) / (u - d);

            if (!std::isfinite(discountRate) || discountRate <= 0.0)
                return std::unexpected("Invalid discount factor step (non-finite or non-positive)");
            if (!std::isfinite(p) || p < 0.0 || p > 1.0)
                return std::unexpected("Invalid risk-neutral probability (arbitrage condition failed)");

            discountRatesByStep.push_back(discountRate);
            probabilityHeadsByStep.push_back(p);
        }
        build_result.withDiscountRatesAndProbabilities(std::move(discountRatesByStep), std::move(probabilityHeadsByStep));

        build_result
            .withPayoff(m_trd.m_optionPayoffType, m_trd.m_strike)
            .withPremium(m_trd.m_optionExerciseType);
        
        if( build_result.m_hasError )
            return std::unexpected( build_result.getErrorMsg() );
        else
            return build_result.build().getMatrix()[ 0 ].m_data.m_optionValue;
    }

    CalculationResult BinomialEngine::calcDelta( const CalcData& calc ) const
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        BinomialEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

		std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected( aggErr );

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        BinomialEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();

        aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected(aggErr);

        const auto pv_up_res = scalarOrError(up_calc.m_results.at(Calculation::PV));
        if (!pv_up_res.has_value())
            return std::unexpected(pv_up_res.error());
        const auto pv_down_res = scalarOrError(down_calc.m_results.at(Calculation::PV));
        if (!pv_down_res.has_value())
            return std::unexpected(pv_down_res.error());
        return pv_up_res.value() - pv_down_res.value();
    }

    CalculationResult BinomialEngine::calcRho( const CalcData& calc ) const
    {
        // Key-rate rho: one entry per curve knot (fallback: single parallel-style entry when no curve).
        constexpr double bump = 0.005;

        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        const auto& tenors = m_mkt.m_yieldCurve.tenors();
        CurveRho rho;
        rho.reserve(tenors.size());
        for (size_t i = 0; i < tenors.size(); ++i)
        {
            if (tenors[i] > m_trd.m_maturity)
                continue;
            MarketData bump_up = m_mkt.bumpYieldCurveKeyRate(i, bump);
            BinomialEngine up_calc(bump_up, m_trd, pv_only);
            up_calc.run();
            std::string aggErr = up_calc.getAggregatedErrors();
            if (!aggErr.empty())
                return std::unexpected(aggErr);

            MarketData bump_down = m_mkt.bumpYieldCurveKeyRate(i, -bump);
            BinomialEngine down_calc(bump_down, m_trd, pv_only);
            down_calc.run();
            aggErr = down_calc.getAggregatedErrors();
            if (!aggErr.empty())
                return std::unexpected(aggErr);

            const auto pv_up = scalarOrError(up_calc.m_results.at(Calculation::PV));
            const auto pv_down = scalarOrError(down_calc.m_results.at(Calculation::PV));
            if (!pv_up.has_value())
                return std::unexpected(pv_up.error());
            if (!pv_down.has_value())
                return std::unexpected(pv_down.error());

            rho.push_back({tenors[i], 100. * (pv_up.value() - pv_down.value())});
        }
        std::ranges::sort(rho, {}, &CurveRhoPoint::tenor);
        return rho;
    }

    CalculationResult BinomialEngine::calcRhoParallel(const CalcData& calc) const
    {
        constexpr double bump = 0.005;
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpYieldCurveParallel(bump);
        BinomialEngine up_calc(bump_up, m_trd, pv_only);
        up_calc.run();
        std::string aggErr = up_calc.getAggregatedErrors();
        if (!aggErr.empty())
            return std::unexpected(aggErr);

        MarketData bump_down = m_mkt.bumpYieldCurveParallel(-bump);
        BinomialEngine down_calc(bump_down, m_trd, pv_only);
        down_calc.run();
        aggErr = down_calc.getAggregatedErrors();
        if (!aggErr.empty())
            return std::unexpected(aggErr);

        const auto pv_up = scalarOrError(up_calc.m_results.at(Calculation::PV));
        const auto pv_down = scalarOrError(down_calc.m_results.at(Calculation::PV));
        if (!pv_up.has_value())
            return std::unexpected(pv_up.error());
        if (!pv_down.has_value())
            return std::unexpected(pv_down.error());
        return 100. * (pv_up.value() - pv_down.value());
    }

    CalculationResult BinomialEngine::calcVega( const CalcData& calc ) const
    {
        if (m_mkt.hasLocalVolSurface())
            return std::unexpected("Vega is not supported when a bootstrapped local volatility surface is attached.");

        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpVol( 0.005 );
        BinomialEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

		std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected( aggErr );

        MarketData bump_down = m_mkt.bumpVol( -0.005 );
        BinomialEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();

        aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected( aggErr );

        const auto pv_up_res = scalarOrError(up_calc.m_results.at(Calculation::PV));
        if (!pv_up_res.has_value())
            return std::unexpected(pv_up_res.error());
        const auto pv_down_res = scalarOrError(down_calc.m_results.at(Calculation::PV));
        if (!pv_down_res.has_value())
            return std::unexpected(pv_down_res.error());
        return (pv_up_res.value() - pv_down_res.value()) * 100;
    }

    CalculationResult BinomialEngine::calcGamma( const CalcData& calc ) const
    {
        CalcData delta_only = calc;
        delta_only.m_calc = Calculation::Delta;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        BinomialEngine up_calc ( bump_up, m_trd, delta_only );
        up_calc.run();

		std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected( aggErr );

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        BinomialEngine down_calc ( bump_down, m_trd, delta_only );
        down_calc.run();

        aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected( aggErr );

        const auto delta_up_res = scalarOrError(up_calc.m_results.at(Calculation::Delta));
        if (!delta_up_res.has_value())
            return std::unexpected(delta_up_res.error());
        const auto delta_down_res = scalarOrError(down_calc.m_results.at(Calculation::Delta));
        if (!delta_down_res.has_value())
            return std::unexpected(delta_down_res.error());
        return delta_up_res.value() - delta_down_res.value();
    }
}