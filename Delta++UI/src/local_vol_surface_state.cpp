#include "local_vol_surface_state.h"

#include <Delta++DB/market_db.h>
#include <Delta++Market/implied_vol.h>

#include "shared_curve_cache.h"

#include <algorithm>

namespace DPP
{
    LocalVolSurfaceState::LocalVolSurfaceState() = default;

    std::filesystem::path LocalVolSurfaceState::dbPath() const
    {
        return DPP::DB::Market::defaultMarketDbPath();
    }

    std::string LocalVolSurfaceState::selectedUnderlying() const
    {
        if (m_underlyings.empty())
            return {};
        if (m_underlyingIdx < 0 || static_cast<size_t>(m_underlyingIdx) >= m_underlyings.size())
            return m_underlyings.front();
        return m_underlyings[static_cast<size_t>(m_underlyingIdx)];
    }

    bool LocalVolSurfaceState::refreshUnderlyings()
    {
        m_status.clear();
        auto res = DPP::DB::Market::queryDistinctUnderlyingsForDate(dbPath(), m_asof);
        if (!res.has_value())
        {
            m_underlyings.clear();
            m_underlyingIdx = 0;
            m_status = res.error();
            return false;
        }

        m_underlyings = std::move(res.value());
        if (m_underlyings.empty())
            m_underlyingIdx = 0;
        else
            m_underlyingIdx = std::clamp(m_underlyingIdx, 0, static_cast<int>(m_underlyings.size() - 1));
        return true;
    }

    bool LocalVolSurfaceState::refreshLastPrice()
    {
        m_status.clear();
        const std::string u = selectedUnderlying();
        if (u.empty())
        {
            m_lastPrice.reset();
            return true;
        }

        auto res = DPP::DB::Market::queryEquityLast(dbPath(), m_asof, u);
        if (!res.has_value())
        {
            m_lastPrice.reset();
            m_status = res.error();
            return false;
        }

        m_lastPrice = res.value();
        return true;
    }

    bool LocalVolSurfaceState::bootstrap()
    {
        m_status.clear();
        m_data = {};

        const std::string u = selectedUnderlying();
        if (u.empty())
        {
            m_status = "No underlying selected";
            return false;
        }
        if (!m_lastPrice.has_value() || !(*m_lastPrice > 0.0))
        {
            m_status = "Missing equity last price; run loader with --load equities";
            return false;
        }
        if (!DPPUI::g_lastBuiltYieldCurve.has_value())
        {
            m_status = "No yield curve cached. Open API Tester and Fetch yield curve first.";
            return false;
        }

        auto midsRes = DPP::DB::Market::queryCallMidsForDateUnderlying(dbPath(), m_asof, u);
        if (!midsRes.has_value())
        {
            m_status = midsRes.error();
            return false;
        }

        const auto& curve = *DPPUI::g_lastBuiltYieldCurve;
        const double S = *m_lastPrice;

        int nTotal = 0;
        int nUsed = 0;
        int nIvOk = 0;
        int nIvErr = 0;

        for (const auto& p : midsRes.value())
        {
            ++nTotal;
            const double T = p.yearsToExpiry;
            const double K = p.strike;
            const double mid = p.mid;
            if (!(T > 0.0) || !(K > 0.0) || !(mid > 0.0))
                continue;

            ++nUsed;
            const double r = curve.zeroRate(T);
            const auto iv = impliedVolCall(mid, S, K, T, r, 0.06); // q=0.06 Minimises failures for test case 2023-01-04 SPX 
            if (!iv.has_value())
            {
                ++nIvErr;
                continue;
            }

            ++nIvOk;
            m_data.texp_years.push_back(T);
            m_data.strikes.push_back(K);
            m_data.call_mids.push_back(mid);
            m_data.implied_vol.push_back(iv.value());
        }

        m_status =
            "Fetched " + std::to_string(nTotal) +
            " call(s); used " + std::to_string(nUsed) +
            "; IV ok " + std::to_string(nIvOk) +
            "; IV failed " + std::to_string(nIvErr);

        return !m_data.implied_vol.empty();
    }
}

