#include <random>
#include <future>
#include <thread>
#include <cmath>
#include <stdexcept>

#include <Delta++Math/distributions.h>
#include "Delta++/monte_carlo_engine.h"


using namespace std::string_literals;
namespace DPP
{
    MonteCarloEngine::MonteCarloEngine(const MarketData& mkt, const TradeData& trd, const CalcData& calc)
		: AbstractEngine(mkt, trd, calc), m_pathSchemeType(PathSchemeType::Milstein)
    {
        initStrategies();
    }

    MonteCarloEngine::MonteCarloEngine(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc)
        : AbstractEngine(mkt, trd, calc), m_pathSchemeType(PathSchemeType::Milstein)
    {
        initStrategies();
    }

    void MonteCarloEngine::initStrategies()
    {
        switch (m_pathSchemeType)
        {
        case PathSchemeType::Exact:
            m_scheme = std::make_unique<ExactScheme>();
            break;
        case PathSchemeType::Euler:
            m_scheme = std::make_unique<EulerScheme>();
            break;
        case PathSchemeType::Milstein:
            m_scheme = std::make_unique<MilsteinScheme>();
            break;
        default:
            throw std::invalid_argument("Unsupported path scheme type");
        }

        switch (m_trd.m_optionPayoffType)
        {
        case OptionPayoffType::Call:
            m_payoff = std::make_unique<MCCallPayoff>(m_trd.m_strike);
            break;
        case OptionPayoffType::Put:
            m_payoff = std::make_unique<MCPutPayoff>(m_trd.m_strike);
            break;
        default:
            throw std::invalid_argument("Unsupported option payoff type");
        }

        switch (m_trd.m_optionExerciseType)
        {
        case OptionExerciseType::European:
            m_exercise = std::make_unique<MCEuropeanExercise>();
            break;
        case OptionExerciseType::American:
            m_exercise = std::make_unique<MCAmericanExercise>();
            break;
        default:
            throw std::invalid_argument("Unsupported option exercise type");
        }
    }

    double MCEuropeanExercise::price(const MonteCarloEngine& engine, const CalcData& calc,
                                   const std::vector<double>& sims, double dt, const IMCPayoff& payoff) const
    {
        const auto maturity_prices =
            sims
            | std::views::drop(calc.m_steps - 1)
            | std::views::stride(calc.m_steps);

        double sum = std::ranges::fold_left(
            maturity_prices
            | std::views::transform([&](double s) { return payoff(s); })
            , 0.0, std::plus<>());

        const double mean_payoff = sum / std::ranges::distance(maturity_prices);
        return std::exp(-engine.m_mkt.m_interestRate * engine.m_trd.m_maturity) * mean_payoff;
    }

    double MCAmericanExercise::price(const MonteCarloEngine& engine, const CalcData& calc,
                                   const std::vector<double>& sims, double dt, const IMCPayoff& payoff) const
    {
        const double mult = std::exp(-engine.m_mkt.m_interestRate * dt);
        std::vector<double> dfs;
        dfs.reserve(calc.m_steps);
        dfs.push_back(1.0);
        for (size_t s = 1; s < calc.m_steps; ++s) { dfs.push_back(dfs[s - 1] * mult); }

        auto final_pvs_view =
            std::views::iota(size_t{0}, calc.m_sims)
            | std::views::transform( [&](size_t sim_idx) {
                auto S_t0_sim = sims.begin() + sim_idx * calc.m_steps;

                auto discounted_payoff =
                    std::views::iota(size_t{0}, calc.m_steps)
                    | std::views::transform([&](size_t step) {
                        double S_t_sim = *(S_t0_sim + step);
                        return payoff(S_t_sim) * dfs[step];
                    });

                double sim_max = std::ranges::fold_left(
                    discounted_payoff,
                    std::numeric_limits<double>::lowest(),
                    [](double a, double b) { return (std::max)(a, b); }
                );

                return sim_max;
            });

        const double sum_path_pvs = std::ranges::fold_left(final_pvs_view, 0.0, std::plus<>());
        return sum_path_pvs / calc.m_sims;
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
                    m_scheme->updatePrice(s, dW, dt, m_mkt);
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
            start = std::min(end, calc.m_sims);
        }
        for (auto& t : threads) { t.join(); };

        return sims;
    }

    void MonteCarloEngine::calcPV( const CalcData& calc )
    {
        const auto dt =  m_trd.m_maturity / static_cast<double>( calc.m_steps );
        std::vector<double> sims = simPaths( calc, dt );

        try 
        {
            const double pv = m_exercise->price(*this, calc, sims, dt, *m_payoff);
            m_results.emplace(calc.m_calc, pv);
        }
        catch (const std::exception& e)
        {
            m_errors.emplace(calc.m_calc, e.what());
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