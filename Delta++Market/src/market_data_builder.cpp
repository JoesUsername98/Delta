#include "Delta++Market/market_data_builder.h"

namespace DPP
{
    std::vector<RateQuote> fredToRateQuotes(
        const std::map<std::string, double>& tenorMap,
        const std::vector<std::pair<std::string, FredSeriesResponse>>& responses,
        const std::string& date)
    {
        std::vector<RateQuote> quotes;

        for (const auto& [seriesId, resp] : responses)
        {
            auto tenorIt = tenorMap.find(seriesId);
            if (tenorIt == tenorMap.end()) continue;

            // Find observation matching the requested date
            for (const auto& obs : resp.observations)
            {
                if (obs.date == date && obs.value.has_value())
                {
                    quotes.push_back({
                        .tenor = tenorIt->second,
                        .rate = obs.value.value(),
                        .date = date,
                        .source = "FRED"
                    });
                    break;
                }
            }
        }

        return quotes;
    }

    std::vector<OptionQuote> avToOptionQuotes(const OptionChainResponse& response)
    {
        std::vector<OptionQuote> quotes;

        for (const auto& entry : response.options)
        {
            quotes.push_back({
                .expiry = entry.expiry,
                .strike = entry.strike,
                .bid = entry.bid,
                .ask = entry.ask,
                .impliedVol = entry.impliedVol,
                .underlying = response.underlying,
                .date = response.date
            });
        }

        return quotes;
    }
}
