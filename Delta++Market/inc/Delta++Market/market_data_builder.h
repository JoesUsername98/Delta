#pragma once

#include "quotes.h"
#include <Delta++MarketAPI/dtos.h>
#include <vector>
#include <map>

namespace DPP
{
    // Converts FRED API DTOs into canonical RateQuotes
    std::vector<RateQuote> fredToRateQuotes(
        const std::map<std::string, double>& tenorMap,
        const std::vector<std::pair<std::string, FredSeriesResponse>>& responses,
        const std::string& date);

    // Converts AlphaVantage option chain DTO into canonical OptionQuotes
    std::vector<OptionQuote> avToOptionQuotes(const OptionChainResponse& response);
}
