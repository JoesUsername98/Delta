#pragma once 

#include <stddef.h>
#include "enums.h"

namespace DPP
{
    struct CalcData
    {
        Calculation m_calc;
        size_t m_steps;
        CalculationMethod m_method;

        explicit CalcData ( Calculation calc, size_t steps, CalculationMethod method ) :
        m_calc( calc ), m_steps ( steps ), m_method ( method )
        {}
    };
}