#pragma once
#include <cmath>
#include <cstddef>
#include <expected>
#include <string>
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
        std::expected<std::vector<double>, std::string> simPaths(const MarketData& mkt, const CalcData& calc,
                                                                 const double dt) const
        {
            const auto sqrt_dt = std::sqrt(dt);

            std::vector<double> zeroRatesByStep(calc.m_steps);
            std::ranges::transform(
                std::views::iota(size_t{0}, calc.m_steps),
                zeroRatesByStep.begin(),
                [&](size_t step) { return mkt.zeroRate(static_cast<double>(step) * dt); });

            std::vector<double> dividendYieldsByStep(calc.m_steps);
            std::ranges::transform(
                std::views::iota(size_t{0}, calc.m_steps),
                dividendYieldsByStep.begin(),
                [&](size_t step) { return mkt.dividendYield(static_cast<double>(step) * dt); });

            // Hoist pillar routing (binary search on expiries) once per calendar time step; inner loop only σ(K).
            // Validate all timesteps before allocating RNG / spawning workers.
            const bool use_local_vol = mkt.hasLocalVolSurface() && calc.m_steps > 1;
            std::vector<size_t> lvPillarByTimeStep;
            if (use_local_vol)
            {
                lvPillarByTimeStep.resize(calc.m_steps - 1);
                for (size_t i = 0; i + 1 < calc.m_steps; ++i)
                {
                    const double t = static_cast<double>(i) * dt;
                    const auto pillarRes = mkt.m_localVolSurface->localVolPillarIndexForTime(t);
                    if (!pillarRes)
                        return std::unexpected(pillarRes.error());
                    lvPillarByTimeStep[i] = *pillarRes;
                }
            }

            std::vector<double> unif_rands(calc.m_sims * (calc.m_steps - 1), 2);
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
                        const double q = dividendYieldsByStep[step_index - 1];
                        double sigma = mkt.m_vol;
                        if (use_local_vol)
                        {
                            const size_t pillarIdx = lvPillarByTimeStep[step_index - 1];
                            sigma = mkt.m_localVolSurface->localVolOnPillar(pillarIdx, s);
                        }
                        self.updatePrice(s, dW, r, q, sigma, dt);
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
        void updatePrice(double& S, double dW, double r, double q, double sigma, double dt) const
        {
            S = S * std::exp((r - q - 0.5 * sigma * sigma) * dt + sigma * dW);
        }
    };

    struct EulerScheme : PathSchemeCRTP<EulerScheme>
    {
        void updatePrice(double& S, double dW, double r, double q, double sigma, double dt) const
        {
            S = S * (1.0 + (r - q) * dt + sigma * dW);
        }
    };

    struct MilsteinScheme : PathSchemeCRTP<MilsteinScheme>
    {
        void updatePrice(double& S, double dW, double r, double q, double sigma, double dt) const
        {
            S = S * (1.0 + (r - q) * dt + sigma * dW + 0.5 * sigma * sigma * (dW * dW - dt));
        }
    };
}
