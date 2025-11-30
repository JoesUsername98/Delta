#pragma once

#include <expected>
#include <string>
#include <type_traits>
#include <vector>
#include <utility>

#include "binomial_engine.h"
#include "black_scholes_engine.h"
#include "monte_carlo_engine.h"

namespace DPP
{
    class EngineFactory
    {
    public:

        static EngineCreationResult getEngine(CalculationMethod engType, const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc)
        {
            switch (engType)
            {
            case CalculationMethod::Binomial:
                return getEngine<BinomialEngine>(mkt, trd, calc);
            case CalculationMethod::BlackScholes:
                return getEngine<BlackScholesEngine>(mkt, trd, calc);
            case CalculationMethod::MonteCarlo:
                return getEngine<MonteCarloEngine>(mkt, trd, calc);
            default:
                return std::unexpected(std::string("Undefined engine type requested"));
            }
        }

        static EngineCreationResult getEngine(CalculationMethod engType, const MarketData& mkt, const TradeData& trd, const CalcData& calc)
        {
            return getEngine(engType, mkt, trd, std::vector<CalcData>{ calc });
        }

        template <typename EngType>
        static EngineCreationResult getEngine(const MarketData& mkt, const TradeData& trd, const CalcData& calc)
        {
            return getEngine<EngType>(mkt, trd, std::vector<CalcData>{calc});
        }

        template <typename EngType>
        static EngineCreationResult getEngine(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc)
        {
            static_assert(std::is_convertible<EngType*, AbstractEngine*>::value, "EngType must be derived from Abstract Engine");

            using create_expr_t = decltype(EngType::create(std::declval<const MarketData&>(), std::declval<const TradeData&>(), std::declval<const std::vector<CalcData>&>()));
            static_assert(std::is_convertible_v<create_expr_t, EngineCreationResult>, "EngType must provide a static create(mkt, trd, calc) returning EngineCreationResult");

            return EngType::create(mkt, trd, calc);
        }
    };
}