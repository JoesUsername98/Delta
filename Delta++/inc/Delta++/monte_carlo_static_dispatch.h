#pragma once

#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "Delta++/abstract_engine.h"
#include "Delta++/monte_carlo_path_schemes.h"
#include "Delta++/monte_carlo_payoff.h"
#include "Delta++/monte_carlo_exercise.h"

namespace DPP::mc_dispatch
{
    // --- Path scheme: enum → visit tag → concrete PathSchemeCRTP type ---

    struct MCExactSchemeKind
    {
    };
    struct MCEulerSchemeKind
    {
    };
    struct MCMilsteinSchemeKind
    {
    };

    using PathSchemeKindVariant = std::variant<MCExactSchemeKind, MCEulerSchemeKind, MCMilsteinSchemeKind>;

    inline PathSchemeKindVariant pathSchemeKindFromCalc(const CalcData& calc)
    {
        switch (calc.m_pathSchemeType)
        {
        case PathSchemeType::Exact:
            return MCExactSchemeKind{};
        case PathSchemeType::Euler:
            return MCEulerSchemeKind{};
        case PathSchemeType::Milstein:
            return MCMilsteinSchemeKind{};
        }
        std::unreachable();
    }

    template <typename SchemeKind>
    struct MonteCarloSchemeFor;

    template <>
    struct MonteCarloSchemeFor<MCExactSchemeKind>
    {
        using type = ExactScheme;
    };

    template <>
    struct MonteCarloSchemeFor<MCEulerSchemeKind>
    {
        using type = EulerScheme;
    };

    template <>
    struct MonteCarloSchemeFor<MCMilsteinSchemeKind>
    {
        using type = MilsteinScheme;
    };

    // --- Exercise style (European / American) ---

    struct MCEuropeanKind
    {
    };
    struct MCAmericanKind
    {
    };

    using ExerciseKindVariant = std::variant<MCEuropeanKind, MCAmericanKind>;

    inline ExerciseKindVariant exerciseKindFromTrade(const TradeData& trd)
    {
        switch (trd.m_optionExerciseType)
        {
        case OptionExerciseType::European:
            return MCEuropeanKind{};
        case OptionExerciseType::American:
            return MCAmericanKind{};
        }
        std::unreachable();
    }

    // --- Payoff kind (Call / Put); strike comes from TradeData inside Exercise ---

    struct MCCallPayoffKind
    {
    };
    struct MCPutPayoffKind
    {
    };

    using PayoffKindVariant = std::variant<MCCallPayoffKind, MCPutPayoffKind>;

    inline PayoffKindVariant payoffKindFromTrade(const TradeData& trd)
    {
        switch (trd.m_optionPayoffType)
        {
        case OptionPayoffType::Call:
            return MCCallPayoffKind{};
        case OptionPayoffType::Put:
            return MCPutPayoffKind{};
        }
        std::unreachable();
    }

    template <typename PayoffKind>
    struct MonteCarloPayoffFor;

    template <>
    struct MonteCarloPayoffFor<MCCallPayoffKind>
    {
        using type = MCCallPayoff;
    };

    template <>
    struct MonteCarloPayoffFor<MCPutPayoffKind>
    {
        using type = MCPutPayoff;
    };

    template <typename ExerciseKind, typename Payoff>
    struct MonteCarloExerciseFor;

    template <typename Payoff>
    struct MonteCarloExerciseFor<MCEuropeanKind, Payoff>
    {
        using type = MCEuropeanExercise<Payoff>;
    };

    template <typename Payoff>
    struct MonteCarloExerciseFor<MCAmericanKind, Payoff>
    {
        using type = MCAmericanExercise<Payoff>;
    };

    /// Static dispatch: no runtime polymorphism on the hot path; `std::visit` selects the 3×2×2 instantiation.
    template <typename SchemeKind, typename ExerciseKind, typename PayoffKind>
    inline double monteCarloPV(const TradeData& trd, const MarketData& mkt, const CalcData& calc, double dt,
                               VectorDebugMap& debugResults)
    {
        using Scheme = typename MonteCarloSchemeFor<std::decay_t<SchemeKind>>::type;
        Scheme scheme{};
        std::vector<double> sims = scheme.simPaths(mkt, calc, dt);

        using Payoff = typename MonteCarloPayoffFor<std::decay_t<PayoffKind>>::type;
        using Exercise = typename MonteCarloExerciseFor<std::decay_t<ExerciseKind>, Payoff>::type;
        Exercise exercise(trd);

        const double pv = exercise.price(trd, mkt, calc, sims, dt);
        if (calc.m_collectDebugPaths)
            debugResults.try_emplace(DebugInfo::MCPaths, std::move(sims));
        return pv;
    }
}
