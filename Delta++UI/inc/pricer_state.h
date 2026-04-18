#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include <Delta++/engine_factory.h>
#include <Delta++Market/yield_curve.h>

#include "enum_combo.h"
#include "local_vol_surface_state.h"

using namespace std::string_literals;
using namespace DPP;

enum class PricerRateSource
{
    FlatStub = 0,
    MarketDb = 1,
};

enum class PricerDividendSource
{
    FlatStub = 0,
    MarketDbImplied = 1,
};

enum class PricerVolSource
{
    Flat = 0,
    LocalVolBootstrap = 1,
};

struct PricerState
{
    // MODEL
    TradeData m_trd;
    MarketData m_mkt;
    int m_steps = 1'000;
    int m_sims = 1'000;
    int m_seed = 42;
    CalculationMethod m_calculationMethod = CalculationMethod::MonteCarlo;
    std::vector<CalcData> m_calcs;
    std::string m_engineBuildError;
    std::unique_ptr<AbstractEngine> m_engine;
    std::array<bool, (int)Calculation::_SIZE> m_calcsToDo;
    std::optional<int> m_timeTaken = std::nullopt;

    int m_optionPayoffIdx = 0;
    int m_exerciseTypeIdx = 0;
    int m_calculationMethodIdx = 0;
    int m_pathSchemeComboIdx = 0;
    static const EnumCombo<OptionPayoffType> m_payoffCombo;
    static const EnumCombo<OptionExerciseType> m_exerciseCombo;
    static const EnumCombo<PathSchemeType> m_pathSchemeCombo;
    static const EnumCombo<CalculationMethod> m_calculationMethodCombo;

    // VIEW_MODEL
    bool m_valueChanged = false;
    bool m_dynamicRecalc = false;
    bool m_btn_calcPressed = false;

    char m_asof[11] = "2023-01-04";
    std::vector<std::string> m_equityTickers;
    int m_underlyingIdx = 0;

    std::string m_marketStatus;
    std::string m_marketError;

    PricerRateSource m_rateSource = PricerRateSource::FlatStub;
    double m_flatRateStub = 0.05;
    std::vector<double> m_yieldCurveTenors;
    std::vector<double> m_yieldCurveZeroRates;
    /// Bumped when `m_mkt.m_yieldCurve` changes so plots can refit axes (ImPlot keeps limits until then).
    uint32_t m_yieldCurvePlotEpoch = 0;

    PricerDividendSource m_dividendSource = PricerDividendSource::FlatStub;
    double m_flatDividendStub = 0.0;
    uint32_t m_dividendPlotEpoch = 0;

    PricerVolSource m_volSource = PricerVolSource::Flat;
    /// Flat-vol UI input; applied via `rebuildFlatVolSurfaceFromUi` (`AHInterpolator::buildFlatConstant`).
    double m_flatVolSigma{0.2};
    /// Scratch state for last local-vol bootstrap (slices, 3D grids); surface copy lives on `m_mkt`.
    LocalVolSurfaceState m_lvState;

    void reset();
    bool needsRecalc();
    bool recalcIfRequired();

    void refreshEquityTickers();
    void syncTradeUnderlyingTicker();
    void applyFlatYieldCurve();
    bool tryLoadTreasuryYieldFromDb();
    /// Rebuilds `m_mkt` with `MarketData::withFlatConstantVol` when `m_volSource == Flat` (no-op in bootstrap mode).
    void rebuildFlatVolSurfaceFromUi();
    void applyFlatDividendCurve();
    bool tryLoadImpliedDividendFromDb();
    bool tryBootstrapLocalVolSurface();

    PricerState();

private:
    static YieldCurve makeFlatYieldCurve(double flatZeroRate);
};
