#pragma once

#include "abstract_engine.h"

namespace DPP
{
    class MonteCarloEngine : public AbstractEngine
    {
    public:
        MonteCarloEngine( const MarketData& mkt, const TradeData& trd, const CalcData& calc ) :
        AbstractEngine( mkt, trd, calc )
        {}
        MonteCarloEngine( const MarketData& mkt, const TradeData& trd, const std::vector<CalcData>& calc ) :
        AbstractEngine( mkt, trd, calc )
        {}
        virtual ~MonteCarloEngine() = default;

    protected:
        void calcPV( const CalcData& calc ) override;
        void calcDelta( const CalcData& calc ) override;
        void calcRho( const CalcData& calc ) override;
        void calcVega( const CalcData& calc ) override;
        void calcGamma( const CalcData& calc ) override;  

    private:
		void updatePrice( double& S, double dW, double dt ) const;
    };
}