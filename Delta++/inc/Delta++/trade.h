#pragma once

#include "enums.h"

#include <string>

namespace DPP
{
    struct TradeData
    {
        OptionExerciseType m_optionExerciseType;
        OptionPayoffType m_optionPayoffType;
        double m_strike;
        double m_maturity;
        /// Underlying equity ticker (UI / market DB); optional for unit tests.
        std::string m_underlyingTicker{};
    };
}