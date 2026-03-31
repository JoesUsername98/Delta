#pragma once

#include <Delta++DB/market_db.h>
#include <Delta++Market/local_vol_surface.h>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace DPP
{
    enum class LocalVolYieldCurveSource
    {
        ApiCache = 0,
        MarketDb = 1,
    };

    struct LocalVolBootstrapInput
    {
        std::vector<double> texp_years;
        std::vector<double> strikes;
        std::vector<double> call_mids;
        std::vector<double> implied_vol;
    };

    struct LocalVolGrid3D
    {
        int xCount = 0;
        int yCount = 0;
        std::vector<float> xs; // size = xCount*yCount
        std::vector<float> ys; // size = xCount*yCount
        std::vector<float> zs; // size = xCount*yCount
        bool empty() const { return xCount <= 0 || yCount <= 0 || zs.empty(); }
    };

    class LocalVolSurfaceState
    {
    public:
        LocalVolSurfaceState();

        const std::vector<std::string>& underlyings() const { return m_underlyings; }
        const std::optional<double>& lastPrice() const { return m_lastPrice; }
        const LocalVolBootstrapInput& data() const { return m_data; }
        const std::optional<LocalVolSurface>& surface() const { return m_surface; }
        const std::vector<double>& sliceK() const { return m_sliceK; }
        const std::vector<double>& sliceT() const { return m_sliceT; }
        const std::vector<std::vector<double>>& sliceIv() const { return m_sliceIv; } // [iT][iK]
        const std::vector<std::vector<double>>& sliceLv() const { return m_sliceLv; } // [iT][iK]
        const std::optional<LocalVolGrid3D>& ivGrid3d() const { return m_ivGrid3d; }
        const std::optional<LocalVolGrid3D>& lvGrid3d() const { return m_lvGrid3d; }
        const std::optional<LocalVolGrid3D>& callGrid3d() const { return m_callGrid3d; }
        const std::string& status() const { return m_status; }

        bool refreshUnderlyings();
        bool refreshLastPrice();
        bool bootstrap();

        char m_asof[11] = "2023-01-04";
        int m_underlyingIdx = 0;
        LocalVolYieldCurveSource m_yieldCurveSource = LocalVolYieldCurveSource::ApiCache;

    private:
        std::filesystem::path dbPath() const;
        std::string selectedUnderlying() const;

        std::vector<std::string> m_underlyings;
        std::optional<double> m_lastPrice;
        LocalVolBootstrapInput m_data;
        std::optional<LocalVolSurface> m_surface;
        std::vector<double> m_sliceK;
        std::vector<double> m_sliceT;
        std::vector<std::vector<double>> m_sliceIv;
        std::vector<std::vector<double>> m_sliceLv;
        std::optional<LocalVolGrid3D> m_ivGrid3d;
        std::optional<LocalVolGrid3D> m_lvGrid3d;
        std::optional<LocalVolGrid3D> m_callGrid3d;
        std::string m_status;
    };
}

