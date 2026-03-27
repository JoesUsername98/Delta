#pragma once 
#include <Delta++Market/yield_curve.h>

namespace DPP
{
    struct MarketData
    {
        double m_vol;
        double m_underlyingPrice;
        DPP::YieldCurve m_yieldCurve;

        MarketData copy() const
        {
            return MarketData{ .m_vol = m_vol, .m_underlyingPrice = m_underlyingPrice, .m_yieldCurve = m_yieldCurve };
		}

        double zeroRate(double t) const
        {
            return m_yieldCurve.zeroRate(t);
        }

        double discount(double t) const
        {
            return m_yieldCurve.discount(t);
        }
        
        MarketData bumpVol ( double bump ) const 
        {
            MarketData bumpee = copy();
            bumpee.m_vol += bump;
            return bumpee;
        }

        MarketData bumpUnderlying ( double bump ) const 
        {
            MarketData bumpee = copy();
            bumpee.m_underlyingPrice *= bump;
            return bumpee;
        }
        
        MarketData bumpYieldCurveParallel(double bump) const
        {
            MarketData bumpee = copy();
            bumpee.m_yieldCurve = bumpee.m_yieldCurve.parallelShift(bump);
            return bumpee;
        }

        MarketData bumpYieldCurveKeyRate(std::size_t knotIdx, double bump) const
        {
            MarketData bumpee = copy();
            bumpee.m_yieldCurve = bumpee.m_yieldCurve.keyRateBump(knotIdx, bump);
            return bumpee;
        }
    };
}