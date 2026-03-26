#include "Delta++Market/market_data_service.h"

namespace DPP
{
    MarketDataService::MarketDataService(std::shared_ptr<FredClient> fred,
                                          std::shared_ptr<AlphaVantageClient> av)
        : m_fred(std::move(fred)), m_av(std::move(av))
    {}

    std::expected<YieldCurve, std::string>
    MarketDataService::buildYieldCurve(const std::string& date) const
    {
        static const std::vector<std::string> kDefaultIds = []
        {
            std::vector<std::string> v;
            v.reserve(kFredDefaultSeriesIds.size());
            for (std::string_view sv : kFredDefaultSeriesIds)
                v.emplace_back(sv);
            return v;
        }();
        return buildYieldCurve(date, kDefaultIds);
    }

    std::expected<YieldCurve, std::string>
    MarketDataService::buildYieldCurve(const std::string& date,
                                        const std::vector<std::string>& seriesIds) const
    {
        if (seriesIds.empty())
            return std::unexpected("No FRED series requested");

        std::vector<std::pair<std::string, FredSeriesResponse>> responses;
        responses.reserve(seriesIds.size());

        for (const auto& sid : seriesIds)
        {
            auto resp = m_fred->getSeriesObservations(sid, date, date);
            if (!resp.has_value())
                return std::unexpected("Failed to fetch " + sid + ": " + resp.error());
            responses.emplace_back(sid, std::move(resp.value()));
        }

        // Convert to canonical quotes
        auto quotes = fredToRateQuotes(kFredTenorMap, responses, date);
        if (quotes.empty())
            return std::unexpected("No rate quotes found for date " + date);

        return YieldCurve::build(quotes);
    }

    std::expected<VolSurface, std::string>
    MarketDataService::buildVolSurface(const std::string& symbol) const
    {
        auto chainResult = m_av->getOptionChain(symbol);
        if (!chainResult.has_value())
            return std::unexpected(chainResult.error());

        auto quotes = avToOptionQuotes(chainResult.value());
        if (quotes.empty())
            return std::unexpected("No option quotes for " + symbol);

        return VolSurface::build(quotes);
    }
}
