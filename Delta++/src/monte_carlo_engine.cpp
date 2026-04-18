#include <type_traits>
#include <variant>

#include <magic_enum/magic_enum.hpp>

#include "Delta++/monte_carlo_engine.h"
#include "Delta++/monte_carlo_static_dispatch.h"

using namespace std::string_literals;

namespace DPP
{
    MonteCarloEngine::MonteCarloEngine(const MarketData& mkt, const TradeData& trd, const CalcData& calc)
        : AbstractEngine(mkt, trd, calc)
    {
    }

    MonteCarloEngine::MonteCarloEngine(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc)
        : AbstractEngine(mkt, trd, calc)
    {
    }

    CalculationResult MonteCarloEngine::calcPV(const CalcData& calc) const
    {
        const double dt = m_trd.m_maturity / static_cast<double>(calc.m_steps);
        return std::visit(
            [&](const auto& schemeKind, const auto& exerciseKind, const auto& payoffKind) {
                return mc_dispatch::monteCarloPV<std::decay_t<decltype(schemeKind)>, std::decay_t<decltype(exerciseKind)>,
                                            std::decay_t<decltype(payoffKind)>>(m_trd, m_mkt, calc, dt, m_debugResults);
            },
            mc_dispatch::pathSchemeKindFromCalc(calc),
            mc_dispatch::exerciseKindFromTrade(m_trd),
            mc_dispatch::payoffKindFromTrade(m_trd));
    }

    CalculationResult MonteCarloEngine::calcDelta( const CalcData& calc ) const
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;
        pv_only.m_collectDebugPaths = false;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

        std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() ) 
            return std::unexpected{ aggErr };

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        MonteCarloEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();
        
		aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
		if ( !aggErr.empty() ) 
			return std::unexpected{ aggErr };

        const auto pv_up_res = scalarOrError(up_calc.m_results.at(Calculation::PV));
        if (!pv_up_res.has_value())
            return std::unexpected{ pv_up_res.error() };
        const auto pv_down_res = scalarOrError(down_calc.m_results.at(Calculation::PV));
        if (!pv_down_res.has_value())
            return std::unexpected{ pv_down_res.error() };
        return pv_up_res.value() - pv_down_res.value();
    }

    CalculationResult MonteCarloEngine::calcRho( const CalcData& calc ) const
    {
        // Key-rate rho: one entry per curve knot (fallback: single parallel-style entry when no curve).
        constexpr double bump = 0.005;

        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;
        pv_only.m_collectDebugPaths = false;

        const auto& tenors = m_mkt.m_yieldCurve.tenors();
        const double T = m_trd.m_maturity;
        CurveRho rho;
        rho.reserve(tenors.size());
        for (size_t i = 0; i < tenors.size(); ++i)
        {
            if (tenors[i] > T)
                break;
            MarketData bump_up = m_mkt.bumpYieldCurveKeyRate(i, bump);
            MonteCarloEngine up_calc(bump_up, m_trd, pv_only);
            up_calc.run();

            std::string aggErr = up_calc.getAggregatedErrors();
            if (!aggErr.empty())
                return std::unexpected{ aggErr };

            MarketData bump_down = m_mkt.bumpYieldCurveKeyRate(i, -bump);
            MonteCarloEngine down_calc(bump_down, m_trd, pv_only);
            down_calc.run();

            aggErr = down_calc.getAggregatedErrors();
            if (!aggErr.empty())
                return std::unexpected{ aggErr };

            const auto pv_up = scalarOrError(up_calc.m_results.at(Calculation::PV));
            const auto pv_down = scalarOrError(down_calc.m_results.at(Calculation::PV));
            if (!pv_up.has_value())
                return std::unexpected{ pv_up.error() };
            if (!pv_down.has_value())
                return std::unexpected{ pv_down.error() };

            rho.push_back({tenors[i], 100. * (pv_up.value() - pv_down.value())});
        }
        return rho;
    }

    CalculationResult MonteCarloEngine::calcRhoParallel(const CalcData& calc) const
    {
        constexpr double bump = 0.005;
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;
        pv_only.m_collectDebugPaths = false;

        MarketData bump_up = m_mkt.bumpYieldCurveParallel(bump);
        MonteCarloEngine up_calc(bump_up, m_trd, pv_only);
        up_calc.run();

        std::string aggErr = up_calc.getAggregatedErrors();
        if (!aggErr.empty())
            return std::unexpected{ aggErr };

        MarketData bump_down = m_mkt.bumpYieldCurveParallel(-bump);
        MonteCarloEngine down_calc(bump_down, m_trd, pv_only);
        down_calc.run();

        aggErr = down_calc.getAggregatedErrors();
        if (!aggErr.empty())
            return std::unexpected{ aggErr };

        const auto pv_up = scalarOrError(up_calc.m_results.at(Calculation::PV));
        const auto pv_down = scalarOrError(down_calc.m_results.at(Calculation::PV));
        if (!pv_up.has_value())
            return std::unexpected{ pv_up.error() };
        if (!pv_down.has_value())
            return std::unexpected{ pv_down.error() };

        return 100. * (pv_up.value() - pv_down.value());
    }

    CalculationResult MonteCarloEngine::calcVega( const CalcData& calc ) const
    {
        if (!m_mkt.isEssentiallyConstantVolSurface())
            return std::unexpected(
                "Monte Carlo vega bumping requires constant σ(T,K) (e.g. MarketData::withFlatConstantVol); general "
                "local vol surfaces are not supported.");

        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;
        pv_only.m_collectDebugPaths = false;

        MarketData bump_up = m_mkt.bumpVol( 0.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

		std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() )
            return std::unexpected{ aggErr };

        MarketData bump_down = m_mkt.bumpVol( -0.005 );
        MonteCarloEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();

        aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
        if (!aggErr.empty())
            return std::unexpected{ aggErr };

        const auto pv_up_res = scalarOrError(up_calc.m_results.at(Calculation::PV));
        if (!pv_up_res.has_value())
            return std::unexpected{ pv_up_res.error() };
        const auto pv_down_res = scalarOrError(down_calc.m_results.at(Calculation::PV));
        if (!pv_down_res.has_value())
            return std::unexpected{ pv_down_res.error() };
        const double pv_up = pv_up_res.value();
        const double pv_down = pv_down_res.value();
        return ( pv_up - pv_down) * 100. ;
    }

    CalculationResult MonteCarloEngine::calcGamma( const CalcData& calc ) const
    {
        CalcData delta_only = calc;
        delta_only.m_calc = Calculation::Delta;
        delta_only.m_collectDebugPaths = false;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, delta_only );
        up_calc.run();

		std::string aggErr = up_calc.getAggregatedErrors();
        if ( !aggErr.empty() )
            return std::unexpected{ aggErr };

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        MonteCarloEngine down_calc ( bump_down, m_trd, delta_only );
        down_calc.run();

        aggErr.clear();
		aggErr = down_calc.getAggregatedErrors();
        if ( !aggErr.empty() )
            return std::unexpected{ aggErr };

        const auto delta_up_res = scalarOrError(up_calc.m_results.at(Calculation::Delta));
        if (!delta_up_res.has_value())
            return std::unexpected{ delta_up_res.error() };
        const auto delta_down_res = scalarOrError(down_calc.m_results.at(Calculation::Delta));
        if (!delta_down_res.has_value())
            return std::unexpected{ delta_down_res.error() };
        const double delta_up = delta_up_res.value();
        const double delta_down = delta_down_res.value();
        return delta_up - delta_down;
    }
}