#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <ranges>
#include <vector>

#include "Delta++/calc.h"
#include "Delta++/market.h"
#include "Delta++/trade.h"

namespace DPP
{
    template <typename Payoff>
    struct MCEuropeanExercise
    {
        explicit MCEuropeanExercise(const TradeData& trd) : m_payoff(trd.m_strike) {}

        double price(const TradeData& trd, const MarketData& mkt, const CalcData& calc,
                     const std::vector<double>& sims, double /*dt*/) const
        {
            const auto maturity_prices =
                sims | std::views::drop(calc.m_steps - 1) | std::views::stride(calc.m_steps);

            double sum = std::ranges::fold_left(
                maturity_prices | std::views::transform([this](double s) { return m_payoff(s); }),
                0.0,
                std::plus<>());

            const double mean_payoff = sum / std::ranges::distance(maturity_prices);
            return mkt.discount(trd.m_maturity) * mean_payoff;
        }

    private:
        Payoff m_payoff;
    };

    template <typename Payoff>
    struct MCAmericanExercise
    {
        explicit MCAmericanExercise(const TradeData& trd) : m_payoff(trd.m_strike) {}

        double price(const TradeData& /*trd*/, const MarketData& mkt, const CalcData& calc,
                     const std::vector<double>& sims, double dt) const
        {
            std::vector<double> dfs;
            dfs.reserve(calc.m_steps);
            for (size_t step = 0; step < calc.m_steps; ++step)
                dfs.push_back(mkt.discount(static_cast<double>(step) * dt));

            auto final_pvs_view =
                std::views::iota(size_t{0}, calc.m_sims) | std::views::transform([&](size_t sim_idx) {
                    auto S_t0_sim = sims.begin() + sim_idx * calc.m_steps;

                    auto discounted_payoff =
                        std::views::iota(size_t{0}, calc.m_steps) | std::views::transform([&](size_t step) {
                            double S_t_sim = *(S_t0_sim + step);
                            return m_payoff(S_t_sim) * dfs[step];
                        });

                    double sim_max = std::ranges::fold_left(
                        discounted_payoff,
                        std::numeric_limits<double>::lowest(),
                        [](double a, double b) { return (std::max)(a, b); });

                    return sim_max;
                });

            const double sum_path_pvs = std::ranges::fold_left(final_pvs_view, 0.0, std::plus<>());
            return sum_path_pvs / calc.m_sims;
        }

    private:
        Payoff m_payoff;
    };
}
