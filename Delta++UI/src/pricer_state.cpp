#include "pricer_state.h"

#include <Delta++DB/market_db.h>
#include <Delta++Market/dividend_yield_curve.h>
#include <Delta++Market/market_data_builder.h>

#include "shared_curve_cache.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <ranges>

namespace
{
    bool sameExpiry(const DPP::DB::Market::PutCallMidPoint& a, const DPP::DB::Market::PutCallMidPoint& b)
    {
        return a.expirationDate == b.expirationDate;
    }
}

const EnumCombo<DPP::OptionPayoffType> PricerState::m_payoffCombo{};
const EnumCombo<DPP::OptionExerciseType> PricerState::m_exerciseCombo{};
const EnumCombo<DPP::PathSchemeType> PricerState::m_pathSchemeCombo{};
const EnumCombo<DPP::CalculationMethod> PricerState::m_calculationMethodCombo{};

YieldCurve PricerState::makeFlatYieldCurve(const double flatZeroRate)
{
    const double ratePct = std::expm1(flatZeroRate) * 100.0;
    std::vector<DPP::RateQuote> quotes = {
        {.tenor = 1.0, .rate = ratePct},
        {.tenor = 30.0, .rate = ratePct},
    };
    auto yc = YieldCurve::build(quotes);
    return yc.value();
}

PricerState::PricerState()
    : m_trd{.m_optionExerciseType = OptionExerciseType::European,
            .m_optionPayoffType = OptionPayoffType::Call,
            .m_strike = 100.,
            .m_maturity = 1.0},
      m_mkt(MarketData::withFlatConstantVol(100.0, 0.2, makeFlatYieldCurve(0.05)).value())
{
    m_yieldCurveTenors = m_mkt.m_yieldCurve.tenors();
    m_yieldCurveZeroRates = m_mkt.m_yieldCurve.zeroRates();
    ++m_yieldCurvePlotEpoch;
    applyFlatDividendCurve();
    refreshEquityTickers();

    m_calcsToDo[(int)Calculation::PV] = true;
    m_calcsToDo[(int)Calculation::Delta] = false;
    m_calcsToDo[(int)Calculation::Gamma] = false;
    m_calcsToDo[(int)Calculation::Vega] = false;
    m_calcsToDo[(int)Calculation::Rho] = false;
    m_calcsToDo[(int)Calculation::RhoParallel] = false;
}

void PricerState::reset()
{
    m_btn_calcPressed = false;
    m_valueChanged = false;
}

bool PricerState::needsRecalc()
{
    m_valueChanged |= m_btn_calcPressed;
    const bool needsRecalc = (m_valueChanged && m_dynamicRecalc) || (!m_dynamicRecalc && m_btn_calcPressed);
    return needsRecalc;
}

bool PricerState::recalcIfRequired()
{
    if (!needsRecalc())
        return false;

    m_calcs.clear();
    for (int i = (int)Calculation::PV; i < (int)Calculation::_SIZE; ++i)
    {
        if (!m_calcsToDo[i])
            continue;

        m_calcs.emplace_back();
        auto& c = m_calcs.back();
        c.m_calc = static_cast<Calculation>(i);
        c.m_pathSchemeType = static_cast<DPP::PathSchemeType>(m_pathSchemeComboIdx);
        c.m_steps = m_steps;
        c.m_sims = m_sims;
        if (m_calculationMethod == DPP::CalculationMethod::MonteCarlo)
            c.m_seed = static_cast<std::uint32_t>(m_seed);
        c.m_collectDebugPaths =
            (m_calculationMethod == DPP::CalculationMethod::MonteCarlo && c.m_calc == Calculation::PV);
    }

    const auto start = std::chrono::high_resolution_clock::now();

    auto engine_res = EngineFactory::getEngine(m_calculationMethod, m_mkt, m_trd, m_calcs);

    if (!engine_res.has_value())
    {
        m_engine.reset();
        m_engineBuildError = engine_res.error();
        return false;
    }

    m_engineBuildError.clear();
    m_engine = std::move(engine_res.value());
    m_engine->run();

    const auto end = std::chrono::high_resolution_clock::now();
    m_timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    return true;
}

void PricerState::refreshEquityTickers()
{
    m_marketError.clear();
    const auto path = DPP::DB::Market::defaultMarketDbPath();
    auto res = DPP::DB::Market::queryDistinctEquityTickers(path);
    if (!res.has_value())
    {
        m_marketError = res.error();
        m_equityTickers.clear();
        m_underlyingIdx = 0;
        m_trd.m_underlyingTicker.clear();
        return;
    }
    m_equityTickers = std::move(res.value());
    if (m_equityTickers.empty())
    {
        m_underlyingIdx = 0;
        m_trd.m_underlyingTicker.clear();
        return;
    }
    m_underlyingIdx = std::clamp(m_underlyingIdx, 0, static_cast<int>(m_equityTickers.size() - 1));
    syncTradeUnderlyingTicker();
}

void PricerState::syncTradeUnderlyingTicker()
{
    if (m_underlyingIdx >= 0 && static_cast<size_t>(m_underlyingIdx) < m_equityTickers.size())
        m_trd.m_underlyingTicker = m_equityTickers[static_cast<size_t>(m_underlyingIdx)];
    else
        m_trd.m_underlyingTicker.clear();
}

void PricerState::applyFlatYieldCurve()
{
    m_mkt.m_yieldCurve = makeFlatYieldCurve(m_flatRateStub);
    m_yieldCurveTenors = m_mkt.m_yieldCurve.tenors();
    m_yieldCurveZeroRates = m_mkt.m_yieldCurve.zeroRates();
    ++m_yieldCurvePlotEpoch;
    rebuildFlatVolSurfaceFromUi();
}

void PricerState::rebuildFlatVolSurfaceFromUi()
{
    if (m_volSource != PricerVolSource::Flat)
        return;
    if (const auto built =
            MarketData::withFlatConstantVol(m_mkt.m_underlyingPrice, m_flatVolSigma, m_mkt.m_yieldCurve,
                                            m_mkt.m_dividendYieldCurve))
        m_mkt = std::move(*built);
}

bool PricerState::tryLoadTreasuryYieldFromDb()
{
    m_marketError.clear();
    m_marketStatus.clear();
    const auto path = DPP::DB::Market::defaultMarketDbPath();
    const auto rowRes = DPP::DB::Market::queryTreasuryYieldRow(path, m_asof);
    if (!rowRes.has_value())
    {
        m_marketError = rowRes.error();
        return false;
    }
    if (!rowRes.value().has_value())
    {
        m_marketError = "No treasury_yields row for as-of date " + std::string(m_asof);
        return false;
    }
    const auto quotes = DPP::massiveTreasuryRowToRateQuotes(*rowRes.value());
    const auto curveRes = YieldCurve::build(quotes);
    if (!curveRes.has_value())
    {
        m_marketError = curveRes.error();
        return false;
    }
    m_mkt.m_yieldCurve = curveRes.value();
    DPPUI::g_lastBuiltYieldCurve = m_mkt.m_yieldCurve;
    m_yieldCurveTenors = m_mkt.m_yieldCurve.tenors();
    m_yieldCurveZeroRates = m_mkt.m_yieldCurve.zeroRates();
    ++m_yieldCurvePlotEpoch;
    m_marketStatus = "Loaded treasury yield curve for " + std::string(m_asof);
    rebuildFlatVolSurfaceFromUi();
    return true;
}

void PricerState::applyFlatDividendCurve()
{
    m_mkt.m_dividendYieldCurve = DividendYieldCurve::flat(m_flatDividendStub);
    ++m_dividendPlotEpoch;
    rebuildFlatVolSurfaceFromUi();
}

bool PricerState::tryLoadImpliedDividendFromDb()
{
    m_marketError.clear();
    m_marketStatus.clear();
    if (m_trd.m_underlyingTicker.empty())
    {
        m_marketError = "Select an underlying before loading implied dividend yield.";
        return false;
    }
    if (!(m_mkt.m_underlyingPrice > 0.0))
    {
        m_marketError = "Underlying value must be positive.";
        return false;
    }

    const auto path = DPP::DB::Market::defaultMarketDbPath();
    auto chainRes =
        DPP::DB::Market::queryPutCallMidsForDateUnderlying(path, m_asof, m_trd.m_underlyingTicker);
    if (!chainRes.has_value())
    {
        m_marketError = chainRes.error();
        return false;
    }
    std::vector<DPP::DB::Market::PutCallMidPoint> chain = std::move(chainRes.value());
    if (chain.empty())
    {
        m_marketError = "No put/call parity inputs for this underlying and as-of date.";
        return false;
    }

    const double S = m_mkt.m_underlyingPrice;
    std::vector<DPP::ParityExpiryPillar> pillars;
    for (auto&& chunk : std::views::all(chain) | std::views::chunk_by(sameExpiry))
    {
        if (std::ranges::empty(chunk))
            continue;
        const auto& r0 = *std::ranges::begin(chunk);
        pillars.push_back(DPP::ParityExpiryPillar{
            .tYears = r0.yearsToExpiry,
            .expirationDate = r0.expirationDate,
            .rows = std::span<const DPP::PutCallMidPoint>(std::ranges::data(chunk), std::ranges::size(chunk)),
        });
    }

    auto divBuilt = DividendYieldCurve::buildFromParity(S, m_mkt.m_yieldCurve, std::move(pillars));
    if (!divBuilt.has_value())
    {
        m_marketError = divBuilt.error();
        return false;
    }
    m_mkt.m_dividendYieldCurve = std::move(divBuilt->curve);
    DPPUI::g_lastBuiltDividendYieldCurve = m_mkt.m_dividendYieldCurve;
    ++m_dividendPlotEpoch;
    m_marketStatus = "Loaded implied dividend yield curve from parity.";
    return true;
}

bool PricerState::tryBootstrapLocalVolSurface()
{
    m_marketError.clear();
    m_marketStatus.clear();
    if (m_trd.m_underlyingTicker.empty())
    {
        m_marketError = "Select an underlying before bootstrapping local volatility.";
        return false;
    }
    if (!(m_mkt.m_underlyingPrice > 0.0))
    {
        m_marketError = "Underlying value must be positive.";
        return false;
    }

    std::strncpy(m_lvState.m_asof, m_asof, sizeof(m_lvState.m_asof));
    m_lvState.m_asof[sizeof(m_lvState.m_asof) - 1] = '\0';

    const DividendYieldCurve* divPtr =
        (m_dividendSource == PricerDividendSource::FlatStub) ? &m_mkt.m_dividendYieldCurve : nullptr;

    const std::optional<double> spotOpt{m_mkt.m_underlyingPrice};
    const bool ok =
        m_lvState.bootstrap(&m_mkt.m_yieldCurve, spotOpt, m_trd.m_underlyingTicker, divPtr);
    m_marketStatus = m_lvState.status();
    if (!ok)
    {
        m_marketError = m_lvState.status().empty() ? "Local vol bootstrap failed." : m_lvState.status();
        m_mkt.m_localVolSurface.reset();
        return false;
    }
    if (!m_lvState.surface().has_value())
    {
        m_marketError = "Bootstrap did not produce a local volatility surface.";
        m_mkt.m_localVolSurface.reset();
        return false;
    }
    m_mkt.m_localVolSurface = *m_lvState.surface();
    if (m_dividendSource == PricerDividendSource::MarketDbImplied && DPPUI::g_lastBuiltDividendYieldCurve.has_value())
        m_mkt.m_dividendYieldCurve = *DPPUI::g_lastBuiltDividendYieldCurve;
    return true;
}
