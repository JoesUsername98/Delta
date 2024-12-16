#pragma once 

#include "enums.h"

namespace DPP
{
    struct TradeData
    {
        OptionExerciseType m_optionExerciseType;
        OptionPayoffType m_optionPayoffType;
        double m_strike;
        double m_maturity;

        explicit TradeData( 
            OptionExerciseType ex, OptionPayoffType payoff, double strike, double maturity ) :
            m_optionExerciseType( ex ), m_optionPayoffType( payoff), 
            m_strike( strike ), m_maturity ( maturity)
            {}
    };
}