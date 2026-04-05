#pragma once

#include <Delta++Market/andreasen_huge_common.h>
#include <Delta++Market/andreasen_huge_interpolator.h>

#include <expected>
#include <string>

namespace DPP
{
    /// Full Andreasen–Huge step-1 bootstrap; returns an `AHInterpolator` for prices and local vol.
    std::expected<AHInterpolator, std::string> bootstrapAndreasenHuge(const AHInput& in);
}
