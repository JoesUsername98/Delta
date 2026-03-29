#pragma once

#include "quotes.h"
#include <Delta++MarketAPI/dtos.h>
#include <vector>

namespace DPP
{
    // Converts AlphaVantage option chain DTO into canonical OptionQuotes
    std::vector<OptionQuote> avToOptionQuotes(const OptionChainResponse& response);

    // Massive treasury-yields row -> RateQuotes (only populated optional yields become knots).
    std::vector<RateQuote> massiveTreasuryRowToRateQuotes(const TreasuryYieldRow& row);
}
