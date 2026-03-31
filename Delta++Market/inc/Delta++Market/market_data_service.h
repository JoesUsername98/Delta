#pragma once

#include "yield_curve.h"
#include "quotes.h"
#include "market_data_builder.h"
#include <Delta++MarketAPI/massive_client.h>

#include <expected>
#include <memory>
#include <string>

namespace DPP
{
    class MarketDataService
    {
    public:
        explicit MarketDataService(std::shared_ptr<MassiveClient> massive);

        /// Treasury yield curve from Massive /fed/v1/treasury-yields for the given calendar date.
        std::expected<YieldCurve, std::string> buildYieldCurve(const std::string& date) const;

    private:
        std::shared_ptr<MassiveClient> m_massive;
    };
}
