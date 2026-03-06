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
    };
}