#include "local_vol_surface_state.h"

#include <Delta++DB/market_db.h>
#include <Delta++Market/andreasen_huge.h>
#include <Delta++Market/implied_vol.h>
#include <Delta++Market/market_data_builder.h>
#include <Delta++Market/yield_curve.h>
#include <Delta++Math/numeric.h>

#include "shared_curve_cache.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <ranges>
#include <optional>
#include <string>
#include <string_view>
#include <cmath>

#include <Delta++Solver/interpolation.h>

namespace
{
    bool sameExpiry(const DPP::DB::Market::PutCallMidPoint& a, const DPP::DB::Market::PutCallMidPoint& b)
    {
        return a.expirationDate == b.expirationDate;
    }
}

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

    void LocalVolSurfaceState::resetState()
    {
        m_status.clear();
        m_LocalVolBootstrapInput = {};
        m_surface.reset();
        m_sliceK.clear();
        m_sliceT.clear();
        m_sliceIv.clear();
        m_sliceLv.clear();
        m_ivGrid3d.reset();
        m_lvGrid3d.reset();
        m_callGrid3d.reset();
        m_parityCurveMs = 0;
    }

    auto LocalVolSurfaceState::bucketOptionChain(const std::vector<DPP::DB::Market::PutCallMidPoint>& chain) const
    {
        return std::views::all(chain) | std::views::chunk_by(sameExpiry);
    }

    LocalVolSurfaceState::ImpliedVolBuildStats LocalVolSurfaceState::buildImpliedVolData(
        const std::vector<DPP::DB::Market::PutCallMidPoint>& chain,
        double spot,
        const YieldCurve& curve)
    {
        ImpliedVolBuildStats stats;
        const auto tIvStart = std::chrono::steady_clock::now();
        for (auto&& chunk : bucketOptionChain(chain))
        {
            if (std::ranges::empty(chunk))
                continue;
            const double tYears = std::ranges::begin(chunk)->yearsToExpiry;
            if (!(tYears > 0.0))
                continue;

            const double r = curve.zeroRate(tYears);
            const double qDiv = DPPUI::g_lastBuiltDividendYieldCurve->q(tYears);

            for (const auto& row : chunk)
            {
                const double K = row.strike;
                const double mid = *row.callMid;

                ++stats.nCallUsed;
                const auto iv = impliedVolCall(mid, spot, K, tYears, r, qDiv);
                if (!iv.has_value())
                {
                    ++stats.nIvErr;
                    continue;
                }

                ++stats.nIvOk;
                m_LocalVolBootstrapInput.texp_years.push_back(tYears);
                m_LocalVolBootstrapInput.strikes.push_back(K);
                m_LocalVolBootstrapInput.call_mids.push_back(mid);
                m_LocalVolBootstrapInput.implied_vol.push_back(iv.value());
            }
        }
        const auto tIvEnd = std::chrono::steady_clock::now();
        stats.ivMs = std::chrono::duration_cast<std::chrono::milliseconds>(tIvEnd - tIvStart).count();
        return stats;
    }

    bool LocalVolSurfaceState::bootstrap(const YieldCurve* yieldCurveOverride,
                                         std::optional<double> spotOverride,
                                         const std::string_view underlyingTickerOverride,
                                         const DividendYieldCurve* dividendForIvOverride)
    {
        resetState();

        const std::string underlyingTicker = underlyingTickerOverride.empty()
                                                 ? selectedUnderlying()
                                                 : std::string(underlyingTickerOverride);
        if (underlyingTicker.empty())
        {
            m_status = "No underlying selected";
            return false;
        }

        const std::optional<double> spotUsed = spotOverride.has_value() ? spotOverride : m_lastPrice;
        if (!spotUsed.has_value() || !(*spotUsed > 0.0))
        {
            m_status = "Missing equity last price; run loader with --load equities or set underlying value";
            return false;
        }

        if (yieldCurveOverride != nullptr)
        {
            DPPUI::g_lastBuiltYieldCurve = *yieldCurveOverride;
        }
        else
        {
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
        }

        const DPP::YieldCurve& curve = *DPPUI::g_lastBuiltYieldCurve;
        
        auto chainRes = DPP::DB::Market::queryPutCallMidsForDateUnderlying(dbPath(), m_asof, underlyingTicker);
        if (!chainRes.has_value())
        {
            m_status = chainRes.error();
            return false;
        }

        std::vector<DPP::DB::Market::PutCallMidPoint> chain = std::move(chainRes.value());
        if (chain.empty())
        {
            m_status = "No parity inputs found";
            return false;
        }
        const double S = *spotUsed;

        if (dividendForIvOverride != nullptr)
        {
            DPPUI::g_lastBuiltDividendYieldCurve = *dividendForIvOverride;
            m_parityCurveMs = 0;
        }
        else
        {
            const auto tParityStart = std::chrono::steady_clock::now();
            // q(T) from put–call parity (interpolated curve) + parity table rows
            auto pillars = bucketOptionChain(chain) | std::views::transform([](auto&& chunk) {
                const auto& r0 = *std::ranges::begin(chunk);
                return ParityExpiryPillar{
                    .tYears = r0.yearsToExpiry,
                    .expirationDate = r0.expirationDate,
                    .rows = std::span<const PutCallMidPoint>(std::ranges::data(chunk), std::ranges::size(chunk)),
                };
            });

            auto divBuilt = DPP::DividendYieldCurve::buildFromParity(S, curve, std::move(pillars));
            if (!divBuilt.has_value())
            {
                m_status = divBuilt.error();
                return false;
            }
            DPPUI::g_lastBuiltDividendYieldCurve = std::move(divBuilt->curve);
            const auto tParityEnd = std::chrono::steady_clock::now();
            m_parityCurveMs =
                std::chrono::duration_cast<std::chrono::milliseconds>(tParityEnd - tParityStart).count();
        }

        const auto ivStats = buildImpliedVolData(chain, S, curve);

        m_status =
            " q(T) time " + std::to_string(m_parityCurveMs) + " ms" +
            "; calls used " + std::to_string(ivStats.nCallUsed) +
            "; IV ok " + std::to_string(ivStats.nIvOk) +
            "; IV failed " + std::to_string(ivStats.nIvErr) +
            "; IV time " + std::to_string(ivStats.ivMs) + " ms";

        // Build AH input using raw call knots grouped by expiry (and inferred q(T) per expiry).
        tryBootstrapAndreasenHugeFromChain(chain, S, curve);

        recomputeSliceAndGridsFromBootstrap();

        return !m_LocalVolBootstrapInput.implied_vol.empty();
    }

    void LocalVolSurfaceState::tryBootstrapAndreasenHugeFromChain(const std::vector<DPP::DB::Market::PutCallMidPoint>& chain,
                                                                  double spot,
                                                                  const YieldCurve& curve)
    {
        if (chain.empty())
            return;

        std::vector<double> Ts;
        std::vector<double> qs;
        std::vector<std::vector<double>> KsByT;
        std::vector<std::vector<double>> CsByT;

        for (auto&& chunk : bucketOptionChain(chain))
        {
            if (std::ranges::size(chunk) < 3)
                continue;
            std::vector<double> kAh;
            std::vector<double> cAh;
            kAh.reserve(std::ranges::size(chunk));
            cAh.reserve(std::ranges::size(chunk));
            for (const auto& row : chunk)
            {
                kAh.push_back(row.strike);
                cAh.push_back(*row.callMid);
            }

            const double tYears = std::ranges::begin(chunk)->yearsToExpiry;
            Ts.push_back(tYears);
            qs.push_back(DPPUI::g_lastBuiltDividendYieldCurve->q(tYears));
            KsByT.push_back(std::move(kAh));
            CsByT.push_back(std::move(cAh));
        }

        bool okGrid = Ts.size() >= 2;
        for (size_t i = 0; i < KsByT.size(); ++i)
            okGrid = okGrid && (KsByT[i].size() >= 3);

        if (!okGrid)
        {
            m_status += "\nAH skipped: need >=2 expiries and >=3 strikes per expiry";
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
            m_status += "\nAH bootstrap error: " + surf.error();
            return;
        }

        m_surface = *surf;
        m_status += "\nAH OK; time " + std::to_string(ahMs) + " ms. ";
    }

    void LocalVolSurfaceState::recomputeSliceAndGridsFromBootstrap()
    {
        // Precompute slice curves once (avoid evaluating IV surface every frame).
        // Strike grid: unique observed strikes from IV bootstrap points only.
        if (m_LocalVolBootstrapInput.strikes.empty() && !m_surface.has_value())
            return;

        m_sliceK = m_LocalVolBootstrapInput.strikes;
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
        rows.reserve(m_LocalVolBootstrapInput.texp_years.size());
        for (size_t i = 0; i < m_LocalVolBootstrapInput.texp_years.size(); ++i)
            rows.push_back({m_LocalVolBootstrapInput.texp_years[i], m_LocalVolBootstrapInput.strikes[i], m_LocalVolBootstrapInput.implied_vol[i]});

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

        for (auto [iT, iK] :
             std::views::cartesian_product(std::views::iota(0uz, m_sliceT.size()),
                                           std::views::iota(0uz, m_sliceK.size())))
        {
            const double Tsl = m_sliceT[iT];
            const double K = m_sliceK[iK];
            if (!Ts.empty())
                m_sliceIv[iT][iK] = ivSurface(Tsl, K);
            if (m_surface.has_value())
                m_sliceLv[iT][iK] = m_surface->localVol(Tsl, K);
        }

        // Precompute 3D grids once (avoid per-frame evaluation when 3D windows are open).
        auto buildGrid = [&](const std::vector<double>& TsGrid, const std::vector<double>& KsGrid,
                             const auto& f_TK) -> LocalVolGrid3D {
            LocalVolGrid3D g;
            g.xCount = static_cast<int>(KsGrid.size());
            g.yCount = static_cast<int>(TsGrid.size());
            const size_t n = static_cast<size_t>(g.xCount) * static_cast<size_t>(g.yCount);
            g.xs.resize(n);
            g.ys.resize(n);
            g.zs.resize(n);
            for (auto [i, tk] :
                 std::views::enumerate(std::views::cartesian_product(TsGrid, KsGrid)))
            {
                const auto& [Tq, Kq] = tk;
                g.xs[static_cast<size_t>(i)] = static_cast<float>(Kq);
                g.ys[static_cast<size_t>(i)] = static_cast<float>(Tq);
                g.zs[static_cast<size_t>(i)] = static_cast<float>(f_TK(Tq, Kq));
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

        const int nK = 30;
        const int nT = 30;
        const auto KsGrid = DPPMath::linspace(minK, maxK, nK);
        const auto TsGrid = DPPMath::linspace(minT, maxT, nT);

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

