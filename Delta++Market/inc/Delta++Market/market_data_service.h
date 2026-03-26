#pragma once

#include "yield_curve.h"
#include "vol_surface.h"
#include "quotes.h"
#include "market_data_builder.h"
#include <Delta++MarketAPI/fred_client.h>
#include <Delta++MarketAPI/alpha_vantage_client.h>

#include <expected>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <array>
#include <string_view>

namespace DPP
{
    // Standard FRED series IDs and their corresponding tenors in years
    inline const std::map<std::string, double> kFredTenorMap = {
        {"DGS1MO", 1.0 / 12.0},
        {"DGS3MO", 0.25},
        {"DGS6MO", 0.5},
        {"DGS1",   1.0},
        {"DGS2",   2.0},
        {"DGS5",   5.0},
        {"DGS10",  10.0},
        {"DGS30",  30.0}
    };

    // Keys of kFredTenorMap in std::map iteration order (lexicographic). Must stay in sync with kFredTenorMap.
    inline constexpr std::array<std::string_view, 8> kFredDefaultSeriesIds = {
        "DGS1", "DGS10", "DGS1MO", "DGS2", "DGS30", "DGS3MO", "DGS5", "DGS6MO"};

    class MarketDataService
    {
    public:
        MarketDataService(std::shared_ptr<FredClient> fred,
                          std::shared_ptr<AlphaVantageClient> av);

        // Fetch FRED treasury rates for a date and build a yield curve (standard DGS* set in kFredTenorMap).
        std::expected<YieldCurve, std::string> buildYieldCurve(const std::string& date) const;

        // Same as above but only the requested series (must be non-empty).
        std::expected<YieldCurve, std::string>
        buildYieldCurve(const std::string& date, const std::vector<std::string>& seriesIds) const;

        // Fetch AlphaVantage option chain and build a vol surface
        std::expected<VolSurface, std::string>
        buildVolSurface(const std::string& symbol) const;

    private:
        std::shared_ptr<FredClient> m_fred;
        std::shared_ptr<AlphaVantageClient> m_av;
    };
}
