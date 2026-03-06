#pragma once 

#include <stddef.h>
#include "enums.h"

namespace DPP
{
    struct CalcData
    {
        Calculation m_calc;
		PathSchemeType m_pathSchemeType = PathSchemeType::Milstein;
        size_t m_steps;
        size_t m_sims = 1'000;
    };
}