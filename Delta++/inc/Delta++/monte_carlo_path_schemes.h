#pragma once
#include <cmath>
#include <thread>
#include <algorithm>
#include <ranges>
#include <vector>

#include "Delta++/market.h"
#include "Delta++/calc.h"
#include <Delta++Math/distributions.h>

namespace DPP
{
    template <typename Derived>
    struct PathSchemeCRTP
    {
        std::vector<double> simPaths(const MarketData& mkt, const CalcData& calc, const double dt) const
        {
            const auto sqrt_dt = std::sqrt(dt);

            std::vector<double> zeroRatesByStep(calc.m_steps);
            std::ranges::transform(
                std::views::iota(size_t{0}, calc.m_steps),
                zeroRatesByStep.begin(),
                [&](size_t step) { return mkt.zeroRate(static_cast<double>(step) * dt); });

			std::vector<double> unif_rands(calc.m_sims * ( calc.m_steps - 1 ), 2);
            std::seed_seq seq{ calc.m_seed };
            std::mt19937_64 rng{ seq };
            std::uniform_real_distribution<double> unif(0.0, 1.0);
            std::ranges::generate(unif_rands, [&]() { return unif(rng); });

            std::vector<double> sims(calc.m_sims * calc.m_steps, mkt.m_underlyingPrice);
            const Derived& self = *static_cast<const Derived*>(this);
            auto worker = [&](size_t start_sim, size_t end_sim) {
                for (size_t sim_idx = start_sim; sim_idx < end_sim; ++sim_idx)
                {
                    double s = mkt.m_underlyingPrice;
                    for (size_t step_index = 1; step_index < calc.m_steps; ++step_index)
                    {
                        const auto idx = sim_idx * calc.m_steps + step_index;
                        const auto& u = unif_rands[(sim_idx * (calc.m_steps - 1)) + (step_index - 1)];
                        const double z = DPPMath::invCumDensity(u);
                        const double dW = sqrt_dt * z;
                        const double r = zeroRatesByStep[step_index - 1];
                        self.updatePrice(s, dW, r, dt, mkt);
                        sims[idx] = s;
                    }
                }
            };

            const size_t n_threads = std::thread::hardware_concurrency();
            const size_t sims_per_thread = calc.m_sims / n_threads;
            std::vector<std::thread> threads;
            threads.reserve(n_threads);
            size_t start = 0;
            for (size_t t = 0; t < n_threads; ++t)
            {
                size_t end = (t == n_threads - 1) ? calc.m_sims : start + sims_per_thread;
                threads.emplace_back(worker, start, end);
                start = (std::min)(end, calc.m_sims);
            }
            for (auto& t : threads) { t.join(); };

            return sims;
        }
    };

    struct ExactScheme : PathSchemeCRTP<ExactScheme>
    {
        void updatePrice(double& S, double dW, double r, double dt, const MarketData& mkt) const
        {
            S = S * std::exp((r - 0.5 * mkt.m_vol * mkt.m_vol) * dt + mkt.m_vol * dW);
        }
    };

    struct EulerScheme : PathSchemeCRTP<EulerScheme>
    {
        void updatePrice(double& S, double dW, double r, double dt, const MarketData& mkt) const
        {
            S = S * (1 + r * dt + mkt.m_vol * dW);
        }
    };

    struct MilsteinScheme : PathSchemeCRTP<MilsteinScheme>
    {
        void updatePrice(double& S, double dW, double r, double dt, const MarketData& mkt) const
        {
            S = S * (1 + r * dt + mkt.m_vol * dW + 0.5 * mkt.m_vol * mkt.m_vol * (dW * dW - dt));
        }
    };
}
