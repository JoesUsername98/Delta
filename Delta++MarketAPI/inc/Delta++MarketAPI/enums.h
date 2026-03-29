#pragma once

namespace DPP
{
    /// How the API Tester obtains treasury yield curve inputs (offline stub vs live Massive).
    enum class YieldCurveSource
    {
        Stub,
        Massive,
    };
}
