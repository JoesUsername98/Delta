#pragma once
#include <vector>
#include <memory>

#include "abstract_engine.h"

namespace DPP
{
    class BinomialEngine : public AbstractEngine
    {
    public:
        virtual ~BinomialEngine() = default;

        static EngineCreationResult create(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc)
        {
            return std::unique_ptr<BinomialEngine>(new BinomialEngine(mkt, trd, calc));
        }

    protected:
        BinomialEngine(const MarketData& mkt, const TradeData& trd, const CalcData& calc) :
            AbstractEngine(mkt, trd, calc)
        {}
        BinomialEngine(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc) :
            AbstractEngine(mkt, trd, calc)
        {}

        CalculationResult calcPV( const CalcData& calc ) const override;
        CalculationResult calcDelta( const CalcData& calc ) const override;
        CalculationResult calcRho( const CalcData& calc ) const override;
        CalculationResult calcVega( const CalcData& calc ) const override;
        CalculationResult calcGamma( const CalcData& calc ) const override;
    };
}