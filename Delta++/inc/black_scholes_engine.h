#pragma once

#include "abstract_engine.h"

namespace DPP
{
    class BlackScholesEngine : public AbstractEngine
    {
    public:
        BlackScholesEngine(const MarketData& mkt, const TradeData& trd, const CalcData& calc) :
            AbstractEngine(mkt, trd, calc)
        {}
        BlackScholesEngine(const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc) :
            AbstractEngine(mkt, trd, calc)
        {}
        virtual ~BlackScholesEngine() = default;

    protected:
        void calcPV(const CalcData& calc) override;
        void calcDelta(const CalcData& calc) override;
        void calcRho(const CalcData& calc) override;
        void calcVega(const CalcData& calc) override;
        void calcGamma(const CalcData& calc) override;

    private:
        double getD1() const;
        double getD2() const;

        double callPrice() const;
        double putPrice() const;
    };

}