#pragma once

#include <expected>
#include <memory>
#include <string>
#include <vector>

#include <magic_enum/magic_enum.hpp>

#include "abstract_engine.h"

#include "monte_carlo_path_schemes.h"
#include "monte_carlo_payoff.h"
#include "monte_carlo_exercise.h"

namespace DPP
{
    class MonteCarloEngine : public AbstractEngine
    {
    public:
        virtual ~MonteCarloEngine() = default;

        static EngineCreationResult create(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc)
        {
            if (calc.empty())
                return std::unexpected("No calculation data supplied");

            if (!magic_enum::enum_contains(calc.front().m_pathSchemeType))
                return std::unexpected("Unsupported path scheme type");

            if (!magic_enum::enum_contains(trd.m_optionPayoffType))
                return std::unexpected("Unsupported option payoff type");

            if (!magic_enum::enum_contains(trd.m_optionExerciseType))
                return std::unexpected("Unsupported option exercise type");

            return std::unique_ptr<MonteCarloEngine>(new MonteCarloEngine(mkt, trd, calc));
        }

    protected:
        MonteCarloEngine(const MarketData& mkt, const TradeData& trd, const CalcData& calc);
        MonteCarloEngine(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc);

        CalculationResult calcPV(const CalcData& calc) const override;
        CalculationResult calcDelta(const CalcData& calc) const override;
        CalculationResult calcRho(const CalcData& calc) const override;
        CalculationResult calcRhoParallel(const CalcData& calc) const override;
        CalculationResult calcVega(const CalcData& calc) const override;
        CalculationResult calcGamma(const CalcData& calc) const override;
    };
}
