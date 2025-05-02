#pragma once

#include <memory>

#include "binomial_engine.h"
#include "black_scholes_engine.h"

namespace DPP
{
    class EngineFactory
    {
    public:
        static std::unique_ptr<AbstractEngine> getEngine(CalculationMethod engType, MarketData mkt, TradeData trd, std::vector<CalcData> calc)
        {
            switch (engType)
            {
            case CalculationMethod::Binomial:
                return getEngine<BinomialEngine>(mkt, trd, calc);
            case CalculationMethod::BlackScholes:
                return getEngine<BlackScholesEngine>(mkt, trd, calc);
            default:
                throw std::runtime_error("Undefined engine type requested");
            }
        }
        static std::unique_ptr<AbstractEngine> getEngine(CalculationMethod engType, MarketData mkt, TradeData trd, CalcData calc)
        {
            return getEngine(engType, mkt, trd, std::vector<CalcData>{ calc });
        }

        template <typename EngType>
        static std::unique_ptr<AbstractEngine> getEngine(MarketData mkt, TradeData trd, CalcData calc)
        {
            return getEngine<EngType>(mkt, trd, std::vector<CalcData>{calc});
        }
        template <typename EngType>
        static std::unique_ptr<AbstractEngine> getEngine(MarketData mkt, TradeData trd, std::vector<CalcData> calc)
        {
            static_assert(std::is_convertible<EngType*, AbstractEngine*>::value, "EngType must be derived from Abstract Engine");
            return std::make_unique<EngType>(mkt, trd, calc);
        }
    };
}