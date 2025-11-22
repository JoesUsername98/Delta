#pragma once

#include <vector>
#include <ranges>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <limits>

#include "abstract_engine.h"

namespace DPP
{
    class MonteCarloEngine : public AbstractEngine
    {
    public:
        MonteCarloEngine( const MarketData& mkt, const TradeData& trd, const CalcData& calc ) :
        AbstractEngine( mkt, trd, calc )
        {}
        MonteCarloEngine( const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc ) :
        AbstractEngine( mkt, trd, calc )
        {}
        virtual ~MonteCarloEngine() = default;

    protected:
        void calcPV( const CalcData& calc ) override;
        void calcDelta( const CalcData& calc ) override;
        void calcRho( const CalcData& calc ) override;
        void calcVega( const CalcData& calc ) override;
        void calcGamma( const CalcData& calc ) override;  

    private:
		void updatePrice( double& S, double dW, double dt ) const;
        std::vector<double> simPaths(const CalcData& calc, const double dt ) const;
        
        template<typename PayoffFn>
        double calcEuropeanPV(const CalcData& calc, const std::vector<double>& sims, double dt, PayoffFn payoff_fn) const;

        template<typename PayoffFn>
        double calcAmericanPV(const CalcData& calc, const std::vector<double>& sims, double dt, PayoffFn payoff_fn) const;
    };
}

namespace DPP 
{
    template<typename PayoffFn>
    double MonteCarloEngine::calcEuropeanPV(const CalcData& calc, const std::vector<double>& sims, double dt, PayoffFn payoff_fn) const
    {
        const auto maturity_prices =
            sims
            | std::views::drop(calc.m_steps - 1)
            | std::views::stride(calc.m_steps);

        double sum = std::ranges::fold_left(
            maturity_prices
            | std::views::transform(payoff_fn)
            , 0.0, std::plus{});

        const double mean_payoff = sum / std::ranges::distance(maturity_prices);
        return std::exp(-m_mkt.m_interestRate * m_trd.m_maturity) * mean_payoff;
    }

    template<typename PayoffFn>
    double MonteCarloEngine::calcAmericanPV(const CalcData& calc, const std::vector<double>& sims, double dt, PayoffFn payoff_fn) const
    {
        const double mult = std::exp(-m_mkt.m_interestRate * dt);
        std::vector<double> dfs;
        dfs.reserve(calc.m_steps);
        dfs.push_back(1.0);
        for (size_t s = 1; s < calc.m_steps; ++s) { dfs.push_back(dfs[s - 1] * mult); }

        auto final_pvs_view =
            std::views::iota(size_t{0}, calc.m_sims)
            | std::views::transform([&](size_t sim_idx) {
                auto S_t0_sim = sims.begin() + sim_idx * calc.m_steps;

                auto discounted_payoff =
                    std::views::iota(size_t{0}, calc.m_steps)
                    | std::views::transform([&](size_t step) {
                        double S_t_sim = *(S_t0_sim + step);
                        return payoff_fn(S_t_sim) * dfs[step];
                    });

                double sim_max = std::ranges::fold_left(
                    discounted_payoff,
                    std::numeric_limits<double>::lowest(),
                    [](double a, double b) { return std::max(a, b); }
                );

                return sim_max;
            });

        const double sum_path_pvs = std::ranges::fold_left(final_pvs_view, 0.0, std::plus{});
        return sum_path_pvs / calc.m_sims;
    }
}