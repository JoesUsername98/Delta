#pragma once

#include <Delta++DB/market_db.h>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace DPP
{
    struct LocalVolBootstrapInput
    {
        std::vector<double> texp_years;
        std::vector<double> strikes;
        std::vector<double> call_mids;
        std::vector<double> implied_vol;
    };

    class LocalVolSurfaceState
    {
    public:
        LocalVolSurfaceState();

        const std::vector<std::string>& underlyings() const { return m_underlyings; }
        const std::optional<double>& lastPrice() const { return m_lastPrice; }
        const LocalVolBootstrapInput& data() const { return m_data; }
        const std::string& status() const { return m_status; }

        bool refreshUnderlyings();
        bool refreshLastPrice();
        bool bootstrap();

        char m_asof[11] = "2023-01-04";
        int m_underlyingIdx = 0;

    private:
        std::filesystem::path dbPath() const;
        std::string selectedUnderlying() const;

        std::vector<std::string> m_underlyings;
        std::optional<double> m_lastPrice;
        LocalVolBootstrapInput m_data;
        std::string m_status;
    };
}

