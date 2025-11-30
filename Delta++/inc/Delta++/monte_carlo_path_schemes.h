#pragma once
#include <cmath>
#include <thread>
#include <algorithm>

#include "Delta++/market.h"
#include "Delta++/calc.h"
#include <Delta++Math/distributions.h>

namespace DPP
{
    struct IPathScheme
    {
        virtual void updatePrice(double& S, double dW, double dt, const MarketData& mkt) const = 0;
        virtual ~IPathScheme() = default;

        std::vector<double> simPaths(const MarketData& mkt, const CalcData& calc, const double dt) const
        {
            const auto sqrt_dt = std::sqrt(dt);
            std::vector<double> sims(calc.m_sims * calc.m_steps, mkt.m_underlyingPrice);

            auto worker = [&](size_t start_sim, size_t end_sim, size_t thread_id) {
                static thread_local std::seed_seq seq{ 42 + thread_id };
                static thread_local std::mt19937_64 rng{ seq };
                static thread_local std::uniform_real_distribution<double> unif(0.0, 1.0);

                for (size_t sim_idx = start_sim; sim_idx < end_sim; ++sim_idx)
                {
                    double s = mkt.m_underlyingPrice;
                    for (size_t step_index = 1; step_index < calc.m_steps; ++step_index)
                    {
                        const auto idx = sim_idx * calc.m_steps + step_index;
                        const double u = unif(rng);
                        const double z = DPPMath::invCumDensity(u);
                        const double dW = sqrt_dt * z;
                        updatePrice(s, dW, dt, mkt);
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
                threads.emplace_back(worker, start, end, t);
                start = (std::min)(end, calc.m_sims);
            }
            for (auto& t : threads) { t.join(); };

            return sims;
        }
    };

    struct ExactScheme : IPathScheme
    {
        void updatePrice(double& S, double dW, double dt, const MarketData& mkt) const override
        {
            S = S * std::exp((mkt.m_interestRate - 0.5 * mkt.m_vol * mkt.m_vol) * dt + mkt.m_vol * dW);
        }
    };

    struct EulerScheme : IPathScheme
    {
        void updatePrice(double& S, double dW, double dt, const MarketData& mkt) const override
        {
            S = S * (1 + mkt.m_interestRate * dt + mkt.m_vol * dW);
        }
    };

    struct MilsteinScheme : IPathScheme
    {
        void updatePrice(double& S, double dW, double dt, const MarketData& mkt) const override
        {
            S = S * (1 + mkt.m_interestRate * dt + mkt.m_vol * dW + 0.5 * mkt.m_vol * mkt.m_vol * (dW * dW - dt));
        }
    };
}
