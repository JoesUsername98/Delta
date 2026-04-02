#pragma once

#include <optional>

#include <Delta++Market/dividend_yield_curve.h>
#include <Delta++Market/yield_curve.h>

//A placeholder for something smarter...
namespace DPPUI
{
    inline std::optional<DPP::YieldCurve> g_lastBuiltYieldCurve;
    /// Last dividend yield curve from local-vol bootstrap (put–call parity pillars); empty if none built.
    inline std::optional<DPP::DividendYieldCurve> g_lastBuiltDividendYieldCurve;
}

