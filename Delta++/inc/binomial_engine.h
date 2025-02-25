#pragma once

#include "abstract_engine.h"

namespace DPP
{
    class BinomialEngine : public AbstractEngine
    {
    public:
    //Const ref?
        BinomialEngine( MarketData mkt, TradeData trd, CalcData calc ) :
        AbstractEngine( mkt, trd, calc )
        {}
        BinomialEngine( MarketData mkt, TradeData trd, std::vector<CalcData> calc ) :
        AbstractEngine( mkt, trd, calc )
        {}
        virtual ~BinomialEngine() = default;

    protected:
        void calcPV( const CalcData& calc ) override;
        void calcDelta( const CalcData& calc ) override;
        void calcRho( const CalcData& calc ) override;
        void calcVega( const CalcData& calc ) override;
        void calcGamma( const CalcData& calc ) override;  
    };
}