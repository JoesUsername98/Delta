#pragma once 

#include <stddef.h>
#include "enums.h"

namespace DPP
{
    struct CalcData
    {
        Calculation m_calc;
        size_t m_steps;
        size_t m_sims;

        explicit CalcData ( Calculation calc, size_t steps, size_t m_sims = 1'000 ) :
        m_calc( calc ), m_steps ( steps ), m_sims ( m_sims )
        {}
    };
}