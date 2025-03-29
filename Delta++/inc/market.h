#pragma once 

namespace DPP
{
    struct MarketData
    {
        double m_vol;
        double m_underlyingPrice;
        double m_interestRate;

        explicit MarketData ( double vol, double underlyingPrice, double interestRate ) :
        m_vol( vol ), m_underlyingPrice( underlyingPrice ), m_interestRate( interestRate )
        {}

        MarketData bumpVol ( double bump ) const 
        {
            MarketData bumpee = *this;
            bumpee.m_vol += bump;
            return bumpee;
        }

        MarketData bumpUnderlying ( double bump ) const
        {
            MarketData bumpee = *this;
            bumpee.m_underlyingPrice *= bump;
            return bumpee;
        }
        
        MarketData bumpInterestRate ( double bump ) const
        {
            MarketData bumpee = *this;
            bumpee.m_interestRate += bump;
            return bumpee;
        }
    };
}