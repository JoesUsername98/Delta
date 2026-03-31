#include "Delta++Market/market_data_service.h"

namespace DPP
{
    MarketDataService::MarketDataService(std::shared_ptr<MassiveClient> massive)
        : m_massive(std::move(massive))
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
}
