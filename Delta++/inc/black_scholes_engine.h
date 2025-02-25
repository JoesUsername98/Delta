#pragma once

#include "abstract_engine.h"

namespace DPP
{
    class BlackScholesEngine : public AbstractEngine
    {
    public:
        //Const ref?
        BlackScholesEngine(MarketData mkt, TradeData trd, CalcData calc) :
            AbstractEngine(mkt, trd, calc)
        {}
        BlackScholesEngine(MarketData mkt, TradeData trd, std::vector<CalcData> calc) :
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

        //TODO abstract out to a Math library
        double cumDensity(double z) const;
        double probDensity(double z) const;
    };

}