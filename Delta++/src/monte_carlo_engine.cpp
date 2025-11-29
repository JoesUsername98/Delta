#include <random>
#include <future>
#include <thread>
#include <cmath>

#include <Delta++Math/distributions.h>
#include "Delta++/monte_carlo_engine.h"


using namespace std::string_literals;
namespace DPP
{
    void MonteCarloEngine::updatePrice( double& S, double dW, double dt ) const
    {
        // exact scheme
        // S = S * std::exp( ( m_mkt.m_interestRate - 0.5 * m_mkt.m_vol * m_mkt.m_vol ) * dt + m_mkt.m_vol * dW );
        // Euler scheme
        // S = S * ( 1 + m_mkt.m_interestRate * dt + m_mkt.m_vol * dW );
        // Milstein scheme
        S = S * ( 1 + m_mkt.m_interestRate * dt + m_mkt.m_vol * dW + 0.5 * m_mkt.m_vol * m_mkt.m_vol * ( dW * dW - dt ) );
    }

    std::vector<double> MonteCarloEngine::simPaths(const CalcData& calc, const double dt) const 
    {
        const auto sqrt_dt = std::sqrt(dt);
        std::vector<double> sims(calc.m_sims * calc.m_steps, m_mkt.m_underlyingPrice);

        auto worker = [&](size_t start_sim, size_t end_sim, size_t thread_id) {
            static thread_local std::seed_seq seq{ 42 + thread_id };
            static thread_local std::mt19937_64 rng{ seq };
            static thread_local std::uniform_real_distribution<double> unif(0.0, 1.0);

            for (size_t sim_idx = start_sim; sim_idx < end_sim; ++sim_idx)
            {
                double s = m_mkt.m_underlyingPrice;
                for (size_t step_index = 1; step_index < calc.m_steps; ++step_index)
                {
                    const auto idx = sim_idx * calc.m_steps + step_index;
                    const double u = unif(rng);
                    const double z = DPPMath::invCumDensity(u);
                    const double dW = sqrt_dt * z;
                    updatePrice(s, dW, dt);
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
            threads.emplace_back(std::thread(worker, start, end, t));
            start = std::min(end, calc.m_sims);
        }
        for (auto& t : threads) { t.join(); };

        return sims;
    }

    void MonteCarloEngine::calcPV( const CalcData& calc )
    {
        const auto dt =  m_trd.m_maturity / static_cast<double>( calc.m_steps );
        std::vector<double> sims = simPaths( calc, dt );

        auto payoff_call = [strike = m_trd.m_strike](double s) { return std::max(s - strike, 0.0); };
        auto payoff_put  = [strike = m_trd.m_strike](double s) { return std::max(strike - s, 0.0); };

        auto european_runner = [&](auto&& payoff_fn) {
            const double pv = calcEuropeanPV(calc, sims, dt, std::forward<decltype(payoff_fn)>(payoff_fn));
            m_results.emplace(calc.m_calc, pv);
        };

        auto american_runner = [&](auto&& payoff_fn) {
            const double pv = calcAmericanPV(calc, sims, dt, std::forward<decltype(payoff_fn)>(payoff_fn));
            m_results.emplace(calc.m_calc, pv);
        };

        if (m_trd.m_optionExerciseType == OptionExerciseType::European) 
        {
            if      ( m_trd.m_optionPayoffType == OptionPayoffType::Call    )    { european_runner(payoff_call);    }
            else if ( m_trd.m_optionPayoffType == OptionPayoffType::Put     )    { european_runner(payoff_put);     } 
            else    { m_errors.emplace(calc.m_calc, "Unsupported option payoff type"); }
        } 
        else if (m_trd.m_optionExerciseType == OptionExerciseType::American) 
        {
            if      ( m_trd.m_optionPayoffType == OptionPayoffType::Call    )   { american_runner(payoff_call);     } 
            else if ( m_trd.m_optionPayoffType == OptionPayoffType::Put     )   { american_runner(payoff_put);      }
            else    { m_errors.emplace(calc.m_calc, "Unsupported option payoff type"); }
        } 
        else 
        {
            m_errors.emplace(calc.m_calc, "Unsupported option exercise type");
        }
    }

    void MonteCarloEngine::calcDelta( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

        if( !up_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : up_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        MonteCarloEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();
        if( !down_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : down_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        const double pv_up = up_calc.m_results.at( Calculation::PV );
        const double pv_down = down_calc.m_results.at( Calculation::PV );
        m_results.emplace( calc.m_calc, pv_up - pv_down );
    }

    void MonteCarloEngine::calcRho( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpInterestRate( 0.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

        if( !up_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : up_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bump_down = m_mkt.bumpInterestRate (-0.005);
        MonteCarloEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();
        if( !down_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : down_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        const double pv_up = up_calc.m_results.at( Calculation::PV );
        const double pv_down = down_calc.m_results.at( Calculation::PV );
        m_results.emplace( calc.m_calc, 100. * ( pv_up - pv_down ) );
    }

    void MonteCarloEngine::calcVega( const CalcData& calc )
    {
        CalcData pv_only = calc;
        pv_only.m_calc = Calculation::PV;

        MarketData bump_up = m_mkt.bumpVol( 0.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, pv_only );
        up_calc.run();

        if( !up_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : up_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bump_down = m_mkt.bumpVol( -0.005 );
        MonteCarloEngine down_calc ( bump_down, m_trd, pv_only );
        down_calc.run();
        if( !down_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : down_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        const double pv_up = up_calc.m_results.at( Calculation::PV );
        const double pv_down = down_calc.m_results.at( Calculation::PV );
        m_results.emplace( calc.m_calc, (pv_up - pv_down)*100 );
    }

    void MonteCarloEngine::calcGamma( const CalcData& calc )
    {
        CalcData delta_only = calc;
        delta_only.m_calc = Calculation::Delta;

        MarketData bump_up = m_mkt.bumpUnderlying( 1.005 );
        MonteCarloEngine up_calc ( bump_up, m_trd, delta_only );
        up_calc.run();

        if( !up_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : up_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        MarketData bump_down = m_mkt.bumpUnderlying( .995 );
        MonteCarloEngine down_calc ( bump_down, m_trd, delta_only );
        down_calc.run();
        if( !down_calc.m_errors.empty() ) 
        {
            for( const auto& [ key , err ] : down_calc.m_errors )
                m_errors[ calc.m_calc ] += " "s + err;
            
            return;
        }

        const double delta_up = up_calc.m_results.at( Calculation::Delta );
        const double delta_down = down_calc.m_results.at( Calculation::Delta );
        m_results.emplace( calc.m_calc, delta_up - delta_down );
    }
}