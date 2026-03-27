#pragma once 

#include <stddef.h>
#include <cstdint>
#include "enums.h"

namespace DPP
{
    struct CalcData
    {
        Calculation m_calc;
		PathSchemeType m_pathSchemeType = PathSchemeType::Milstein;
        size_t m_steps;
        size_t m_sims = 1'000;
        std::uint32_t m_seed = 42;
        bool m_collectDebugPaths = false;
    };
}