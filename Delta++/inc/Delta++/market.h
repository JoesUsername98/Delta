#pragma once 

namespace DPP
{
    struct MarketData
    {
        double m_vol;
        double m_underlyingPrice;
        double m_interestRate;

        MarketData copy() const
        {
            return MarketData{ .m_vol = m_vol, .m_underlyingPrice = m_underlyingPrice, .m_interestRate = m_interestRate };
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
    };
}