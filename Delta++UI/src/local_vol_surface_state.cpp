#include "local_vol_surface_state.h"

#include <Delta++DB/market_db.h>
#include <Delta++Market/andreasen_huge.h>
#include <Delta++Market/implied_vol.h>
#include <Delta++Market/market_data_builder.h>
#include <Delta++Market/dividend_yield_curve.h>
#include <Delta++Market/yield_curve.h>

#include "shared_curve_cache.h"

#include <algorithm>
#include <chrono>
#include <map>
#include <limits>
#include <ranges>
#include <optional>
#include <span>
#include <string>
#include <cmath>

#include <Delta++Solver/interpolation.h>

namespace DPP
{
    namespace
    {
        /// Per-expiry option chain buckets for local-vol bootstrap (AH input).
        struct AhExpiryBucket
        {
            std::string expirationDate;
            double T{};
            double q{};
            std::vector<double> strikesPaired;
            std::vector<double> callsPaired;
            std::vector<double> putsPaired;
            std::vector<double> strikesCalls;
            std::vector<double> callsOnly;
        };

        void tryBootstrapAndreasenHugeFromBuckets(const std::map<std::string, AhExpiryBucket>& buckets,
                                                  double spot,
                                                  const YieldCurve& curve,
                                                  std::string& status,
                                                  std::optional<LocalVolSurface>& surface)
        {
            if (buckets.empty())
                return;

            std::vector<double> Ts;
            std::vector<double> qs;
            std::vector<std::vector<double>> KsByT;
            std::vector<std::vector<double>> CsByT;

            Ts.reserve(buckets.size());
            qs.reserve(buckets.size());
            KsByT.reserve(buckets.size());
            CsByT.reserve(buckets.size());

            for (const auto& [exp, b] : buckets)
            {
                if (!(b.T > 0.0))
                    continue;
                std::vector<double> kAh = b.strikesCalls;
                std::vector<double> cAh = b.callsOnly;
                if (kAh.size() < 3 || cAh.size() != kAh.size())
                    continue;

                Ts.push_back(b.T);
                qs.push_back(b.q);
                KsByT.push_back(std::move(kAh));
                CsByT.push_back(std::move(cAh));
            }

            bool okGrid = Ts.size() >= 2;
            for (size_t i = 0; i < KsByT.size(); ++i)
                okGrid = okGrid && (KsByT[i].size() >= 3);

            if (!okGrid)
            {
                status += "\nAH skipped: need >=2 expiries and >=3 strikes per expiry";
                return;
            }

            AHInput ah{
                .spot = spot,
                .curve = curve,
                .expiries = std::move(Ts),
                .dividendYields = std::move(qs),
                .strikes = std::move(KsByT),
                .callPrices = std::move(CsByT),
            };
            const auto tAhStart = std::chrono::steady_clock::now();
            const auto surf = bootstrapAndreasenHuge(ah);
            const auto tAhEnd = std::chrono::steady_clock::now();
            const auto ahMs = std::chrono::duration_cast<std::chrono::milliseconds>(tAhEnd - tAhStart).count();

            if (!surf.has_value())
            {
                status += "\nAH bootstrap error: " + surf.error();
                return;
            }

            surface = *surf;
            status += "\nAH OK; time " + std::to_string(ahMs) + " ms. ";
        }
    } // namespace

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
        {
            m_underlyingIdx = 0;
            return true;
        }
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
        m_surface.reset();
        m_sliceK.clear();
        m_sliceT.clear();
        m_sliceIv.clear();
        m_sliceLv.clear();
        m_ivGrid3d.reset();
        m_lvGrid3d.reset();
        m_callGrid3d.reset();
        m_parityYields.clear();
        m_parityCurveMs = 0;

        const std::string underlyingTicker = selectedUnderlying();
        if (underlyingTicker.empty())
        {
            m_status = "No underlying selected";
            return false;
        }
        if (!m_lastPrice.has_value() || !(*m_lastPrice > 0.0))
        {
            m_status = "Missing equity last price; run loader with --load equities";
            return false;
        }
        const auto rowRes = DPP::DB::Market::queryTreasuryYieldRow(dbPath(), m_asof);
        if (!rowRes.has_value())
        {
            m_status = rowRes.error();
            return false;
        }
        if (!rowRes.value().has_value())
        {
            m_status = "No treasury_yields row for this AsOf date";
            return false;
        }

        const auto quotes = DPP::massiveTreasuryRowToRateQuotes(*rowRes.value());
        const auto curveRes = DPP::YieldCurve::build(quotes);
        if (!curveRes.has_value())
        {
            m_status = curveRes.error();
            return false;
        }
        DPPUI::g_lastBuiltYieldCurve = curveRes.value();
        const DPP::YieldCurve& curve = *DPPUI::g_lastBuiltYieldCurve;
        
        auto chainRes = DPP::DB::Market::queryPutCallMidsForDateUnderlying(dbPath(), m_asof, underlyingTicker);
        if (!chainRes.has_value())
        {
            m_status = chainRes.error();
            return false;
        }

        const double S = *m_lastPrice;

        std::map<std::string, AhExpiryBucket> buckets;

        int nRows = 0;
        int nPairs = 0;
        for (const auto& row : chainRes.value())
        {
            ++nRows;
            auto& b = buckets[row.expirationDate];
            b.expirationDate = row.expirationDate;
            b.T = row.yearsToExpiry;

            if (row.callMid.has_value() && (*row.callMid > 0.0) && (row.strike > 0.0))
            {
                b.strikesCalls.push_back(row.strike);
                b.callsOnly.push_back(*row.callMid);
            }
            if (row.callMid.has_value() && row.putMid.has_value() &&
                (*row.callMid > 0.0) && (*row.putMid > 0.0) && (row.strike > 0.0))
            {
                ++nPairs;
                b.strikesPaired.push_back(row.strike);
                b.callsPaired.push_back(*row.callMid);
                b.putsPaired.push_back(*row.putMid);
            }
        }

        const auto tParityStart = std::chrono::steady_clock::now();

        // q(T) from put–call parity (interpolated curve) + parity table rows
        std::vector<DPP::ParityExpiryInput> parityInputs;
        parityInputs.reserve(buckets.size());
        for (auto& [exp, b] : buckets)
        {
            if (!(b.T > 0.0))
                continue;
            parityInputs.push_back(DPP::ParityExpiryInput{
                .tYears = b.T,
                .expirationDate = b.expirationDate,
                .strikes = b.strikesPaired,
                .callMids = b.callsPaired,
                .putMids = b.putsPaired,
            });
        }

        if (!parityInputs.empty())
        {
            auto divBuilt =
                DPP::buildDividendYieldCurveFromParity(S, curve, std::span<const DPP::ParityExpiryInput>(parityInputs));
            if (!divBuilt.has_value())
            {
                m_status = divBuilt.error();
                return false;
            }
            for (const DPP::DividendYieldPillarDiag& d : divBuilt->pillars)
            {
                m_parityYields.push_back(ParityYieldRow{
                    .expirationDate = d.expirationDate,
                    .texp_years = d.texp_years,
                    .r = d.r,
                    .q = d.q,
                    .A = d.A,
                    .B = d.B,
                    .forward = d.forward,
                    .nUsed = d.nUsed,
                    .rmse = d.rmse,
                });
            }
            DPPUI::g_lastBuiltDividendYieldCurve = std::move(divBuilt->curve);
        }

        const auto tParityEnd = std::chrono::steady_clock::now();
        m_parityCurveMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(tParityEnd - tParityStart).count();

        int nIvOk = 0;
        int nIvErr = 0;
        int nCallUsed = 0;

        if (!DPPUI::g_lastBuiltDividendYieldCurve.has_value())
        {
            m_status = "Dividend yield curve is not available; cannot compute implied volatility.";
            return false;
        }

        const auto tIvStart = std::chrono::steady_clock::now();
        for (auto& [exp, b] : buckets)
        {
            if (!(b.T > 0.0))
                continue;

            // Consistent strike ordering for downstream splines and AH input.
            if (!b.strikesCalls.empty())
                std::ranges::sort(std::views::zip(b.strikesCalls, b.callsOnly), {},
                                  [](const auto& e) { return std::get<0>(e); });

            const double r = curve.zeroRate(b.T);
            b.q = DPPUI::g_lastBuiltDividendYieldCurve->q(b.T);

            for (size_t i = 0; i < b.strikesCalls.size(); ++i)
            {
                const double K = b.strikesCalls[i];
                const double mid = b.callsOnly[i];
                if (!(K > 0.0) || !(mid > 0.0))
                    continue;

                ++nCallUsed;
                const auto iv = impliedVolCall(mid, S, K, b.T, r, b.q);
                if (!iv.has_value())
                {
                    ++nIvErr;
                    continue;
                }

                ++nIvOk;
                m_data.texp_years.push_back(b.T);
                m_data.strikes.push_back(K);
                m_data.call_mids.push_back(mid);
                m_data.implied_vol.push_back(iv.value());
            }
        }
        const auto tIvEnd = std::chrono::steady_clock::now();
        const auto ivMs = std::chrono::duration_cast<std::chrono::milliseconds>(tIvEnd - tIvStart).count();

        m_status =
            "Fetched " + std::to_string(nRows) +
            " chain row(s); paired " + std::to_string(nPairs) +
            "; q(T) time " + std::to_string(m_parityCurveMs) + " ms" +
            "; calls used " + std::to_string(nCallUsed) +
            "; IV ok " + std::to_string(nIvOk) +
            "; IV failed " + std::to_string(nIvErr) +
            "; IV time " + std::to_string(ivMs) + " ms";

        // Build AH input using raw call knots grouped by expiry (and inferred q(T) per expiry).
        tryBootstrapAndreasenHugeFromBuckets(buckets, S, curve, m_status, m_surface);

        recomputeSliceAndGridsFromBootstrap();

        return !m_data.implied_vol.empty();
    }

    void LocalVolSurfaceState::recomputeSliceAndGridsFromBootstrap()
    {
        // Precompute slice curves once (avoid evaluating IV surface every frame).
        // Strike grid: unique observed strikes from IV bootstrap points only.
        if (m_data.strikes.empty() && !m_surface.has_value())
            return;

        m_sliceK = m_data.strikes;
        if (m_sliceK.empty() && m_surface.has_value())
        {
            m_status +=
                "\nSlice / IV grid: no strike samples from implied-vol bootstrap; "
                "local vol surface alone does not define a strike mesh here.";
        }
        std::sort(m_sliceK.begin(), m_sliceK.end());
        m_sliceK.erase(std::unique(m_sliceK.begin(), m_sliceK.end()), m_sliceK.end());

        // Paper/UI requested slices.
        m_sliceT = {0.25, 0.5, 1.0};

        if (m_sliceK.size() < 3)
            return;

        // Group IV knots by expiry and build per-expiry spline once.
        struct Row
        {
            double T;
            double K;
            double iv;
        };
        std::vector<Row> rows;
        rows.reserve(m_data.texp_years.size());
        for (size_t i = 0; i < m_data.texp_years.size(); ++i)
            rows.push_back({m_data.texp_years[i], m_data.strikes[i], m_data.implied_vol[i]});

        std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) {
            if (a.T != b.T)
                return a.T < b.T;
            return a.K < b.K;
        });

        std::vector<double> Ts;
        std::vector<std::vector<double>> KsByT;
        std::vector<std::vector<double>> IvsByT;
        for (const auto& r : rows)
        {
            if (Ts.empty() || r.T != Ts.back())
            {
                Ts.push_back(r.T);
                KsByT.emplace_back();
                IvsByT.emplace_back();
            }
            KsByT.back().push_back(r.K);
            IvsByT.back().push_back(r.iv);
        }

        auto ivAt = [&](size_t iT, double Kq) -> double {
            const auto& Ks = KsByT[iT];
            const auto& Vs = IvsByT[iT];
            if (Ks.size() < 2)
                return Vs.empty() ? std::numeric_limits<double>::quiet_NaN() : Vs.front();
            CubicSplineInterpolator s(Ks, Vs);
            return s(Kq);
        };

        auto ivSurface = [&](double Tq, double Kq) -> double {
            if (Ts.empty())
                return std::numeric_limits<double>::quiet_NaN();
            if (Tq <= Ts.front())
                return ivAt(0, Kq);
            if (Tq >= Ts.back())
                return ivAt(Ts.size() - 1, Kq);
            const auto it = std::upper_bound(Ts.begin(), Ts.end(), Tq);
            const size_t i1 = static_cast<size_t>(it - Ts.begin());
            const size_t i0 = i1 - 1;
            const double t0 = Ts[i0];
            const double t1 = Ts[i1];
            const double v0 = ivAt(i0, Kq);
            const double v1 = ivAt(i1, Kq);
            const double w = (t1 > t0) ? (Tq - t0) / (t1 - t0) : 0.0;
            return v0 + (v1 - v0) * w;
        };

        m_sliceIv.assign(m_sliceT.size(),
                         std::vector<double>(m_sliceK.size(), std::numeric_limits<double>::quiet_NaN()));
        m_sliceLv.assign(m_sliceT.size(),
                         std::vector<double>(m_sliceK.size(), std::numeric_limits<double>::quiet_NaN()));

        for (size_t it = 0; it < m_sliceT.size(); ++it)
        {
            const double Tsl = m_sliceT[it];
            for (size_t ik = 0; ik < m_sliceK.size(); ++ik)
            {
                const double K = m_sliceK[ik];
                if (!Ts.empty())
                    m_sliceIv[it][ik] = ivSurface(Tsl, K);
                if (m_surface.has_value())
                    m_sliceLv[it][ik] = m_surface->localVol(Tsl, K);
            }
        }

        // Precompute 3D grids once (avoid per-frame evaluation when 3D windows are open).
        auto linspace = [](double a, double b, int n) -> std::vector<double> {
            std::vector<double> out;
            if (n <= 1)
            {
                out.push_back(a);
                return out;
            }
            out.resize(static_cast<size_t>(n));
            for (int i = 0; i < n; ++i)
            {
                const double t = static_cast<double>(i) / static_cast<double>(n - 1);
                out[static_cast<size_t>(i)] = a + (b - a) * t;
            }
            return out;
        };

        auto buildGrid = [&](const std::vector<double>& TsGrid, const std::vector<double>& KsGrid,
                             const auto& f_TK) -> LocalVolGrid3D {
            LocalVolGrid3D g;
            g.xCount = static_cast<int>(KsGrid.size());
            g.yCount = static_cast<int>(TsGrid.size());
            const size_t n = static_cast<size_t>(g.xCount) * static_cast<size_t>(g.yCount);
            g.xs.resize(n);
            g.ys.resize(n);
            g.zs.resize(n);
            size_t idx = 0;
            for (double Tq : TsGrid)
            {
                for (double Kq : KsGrid)
                {
                    g.xs[idx] = static_cast<float>(Kq);
                    g.ys[idx] = static_cast<float>(Tq);
                    g.zs[idx] = static_cast<float>(f_TK(Tq, Kq));
                    ++idx;
                }
            }
            return g;
        };

        // Determine overall [minK,maxK] and [minT,maxT] for grid generation.
        double minK = std::numeric_limits<double>::infinity();
        double maxK = -std::numeric_limits<double>::infinity();
        for (double k : m_sliceK)
        {
            minK = (std::min)(minK, k);
            maxK = (std::max)(maxK, k);
        }
        double minT = std::numeric_limits<double>::infinity();
        double maxT = -std::numeric_limits<double>::infinity();
        for (double t : Ts)
        {
            minT = (std::min)(minT, t);
            maxT = (std::max)(maxT, t);
        }
        if (m_surface.has_value() && !m_surface->expiries().empty())
        {
            const auto& TsKnots = m_surface->expiries();
            minT = (std::min)(minT, TsKnots.front());
            maxT = (std::max)(maxT, TsKnots.back());
        }

        if (!std::isfinite(minK) || !std::isfinite(maxK) || !std::isfinite(minT) || !std::isfinite(maxT))
            return;

        const int nK = 60;
        const int nT = 30;
        const auto KsGrid = linspace(minK, maxK, nK);
        const auto TsGrid = linspace(minT, maxT, nT);

        // IV grid only if we have IV knots.
        if (!Ts.empty())
            m_ivGrid3d = buildGrid(TsGrid, KsGrid, [&](double Tq, double Kq) { return ivSurface(Tq, Kq); });

        if (!m_surface.has_value())
            return;

        const auto& surf = *m_surface;
        m_lvGrid3d = buildGrid(TsGrid, KsGrid, [&](double Tq, double Kq) { return surf.localVol(Tq, Kq); });
        m_callGrid3d = buildGrid(TsGrid, KsGrid, [&](double Tq, double Kq) { return surf.callPrice(Tq, Kq); });
    }
}

