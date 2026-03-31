#include "Delta++Market/market_data_builder.h"

namespace DPP
{
    std::vector<RateQuote> massiveTreasuryRowToRateQuotes(const TreasuryYieldRow& row)
    {
        std::vector<RateQuote> quotes;
        const std::string& d = row.date;

        auto push = [&](double tenor, const std::optional<double>& y)
        {
            if (y.has_value())
                quotes.push_back({tenor, y.value(), d, "Massive"});
        };

        push(1.0 / 12.0, row.yield_1_month);
        push(0.25, row.yield_3_month);
        push(0.5, row.yield_6_month);
        push(1.0, row.yield_1_year);
        push(2.0, row.yield_2_year);
        push(3.0, row.yield_3_year);
        push(5.0, row.yield_5_year);
        push(7.0, row.yield_7_year);
        push(10.0, row.yield_10_year);
        push(20.0, row.yield_20_year);
        push(30.0, row.yield_30_year);

        return quotes;
    }
}
