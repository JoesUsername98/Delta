#pragma once

#include <vector>
#include "monte_carlo_payoff.h"
#include "abstract_engine.h"

namespace DPP
{
    struct IMCExercise
    {
        virtual double price(const TradeData& trd, const MarketData& mkt, const CalcData& calc,
                             const std::vector<double>& sims, double dt, const IMCPayoff& payoff) const = 0;
        virtual ~IMCExercise() = default;
    };

    struct MCEuropeanExercise : IMCExercise
    {
        double price(const TradeData& trd, const MarketData& mkt, const CalcData& calc,
                     const std::vector<double>& sims, double dt, const IMCPayoff& payoff) const override;
    };

    struct MCAmericanExercise : IMCExercise
    {
        double price(const TradeData& trd, const MarketData& mkt, const CalcData& calc,
                     const std::vector<double>& sims, double dt, const IMCPayoff& payoff) const override;
    };
}
