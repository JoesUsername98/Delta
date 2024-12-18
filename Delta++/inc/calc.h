#pragma once 

#include <stddef.h>
#include "enums.h"

namespace DPP
{
    struct CalcData
    {
        Calculation m_calc;
        size_t m_steps;

        explicit CalcData ( Calculation calc, size_t steps ) :
        m_calc( calc ), m_steps ( steps )
        {}
    };
}