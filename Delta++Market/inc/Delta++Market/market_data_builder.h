#pragma once

#include "quotes.h"
#include <Delta++MarketAPI/dtos.h>
#include <vector>

namespace DPP
{
    // Massive treasury-yields row -> RateQuotes (only populated optional yields become knots).
    std::vector<RateQuote> massiveTreasuryRowToRateQuotes(const TreasuryYieldRow& row);
}
