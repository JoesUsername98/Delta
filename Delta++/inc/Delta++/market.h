#pragma once 

#include <optional>

#include <Delta++Market/yield_curve.h>

namespace DPP
{
    struct MarketData
    {
        double m_vol;
        double m_underlyingPrice;
        double m_interestRate;
        std::optional<DPP::YieldCurve> m_yieldCurve;

        MarketData copy() const
        {
            return MarketData{ .m_vol = m_vol, .m_underlyingPrice = m_underlyingPrice, .m_interestRate = m_interestRate, .m_yieldCurve = m_yieldCurve };
		}

        double zeroRate(double t) const
        {
            return m_yieldCurve.has_value() ? m_yieldCurve->zeroRate(t) : m_interestRate;
        }

        double discount(double t) const
        {
            return m_yieldCurve.has_value() ? m_yieldCurve->discount(t) : std::exp(-m_interestRate * t);
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
        
        MarketData bumpInterestRate ( double bump ) const 
        {
            MarketData bumpee = copy();
            bumpee.m_interestRate += bump;
            return bumpee;
        }

        MarketData bumpYieldCurveParallel(double bump) const
        {
            MarketData bumpee = copy();
            if (bumpee.m_yieldCurve.has_value())
                bumpee.m_yieldCurve = bumpee.m_yieldCurve->parallelShift(bump);
            else
                bumpee.m_interestRate += bump;
            return bumpee;
        }

        MarketData bumpYieldCurveKeyRate(std::size_t knotIdx, double bump) const
        {
            MarketData bumpee = copy();
            if (bumpee.m_yieldCurve.has_value())
                bumpee.m_yieldCurve = bumpee.m_yieldCurve->keyRateBump(knotIdx, bump);
            else
                bumpee.m_interestRate += bump;
            return bumpee;
        }
    };
}