#include "Delta++Market/market_data_service.h"

namespace DPP
{
    MarketDataService::MarketDataService(std::shared_ptr<AlphaVantageClient> av,
                                          std::shared_ptr<MassiveClient> massive)
        : m_av(std::move(av)), m_massive(std::move(massive))
    {}

    std::expected<YieldCurve, std::string> MarketDataService::buildYieldCurve(const std::string& date) const
    {
        if (!m_massive)
            return std::unexpected("MassiveClient not configured");

        auto row = m_massive->getTreasuryYieldsForDate(date);
        if (!row.has_value())
            return std::unexpected(row.error());

        auto quotes = massiveTreasuryRowToRateQuotes(*row);
        if (quotes.empty())
            return std::unexpected("No treasury yield columns present for date " + date);

        return YieldCurve::build(quotes);
    }

    std::expected<VolSurface, std::string> MarketDataService::buildVolSurface(const std::string& symbol) const
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
