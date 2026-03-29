#pragma once

#include <Delta++Market/yield_curve.h>
#include <Delta++MarketAPI/dtos.h>
#include <Delta++MarketAPI/enums.h>

#include <optional>
#include <string>
#include <vector>

struct ApiTesterState
{
    DPP::YieldCurveSource m_yieldCurveSource = DPP::YieldCurveSource::Stub;

    char m_buildDate[11] = "2024-03-01";
    double m_tYears = 1.0;

    char m_optionSymbol[32] = "AAPL";
    double m_avExpiryYears = 0.5;
    double m_avStrike = 100.0;

    std::string m_status;

    bool m_hasCurve = false;
    std::optional<DPP::YieldCurve> m_curve;
    double m_curveZeroRateAtT = 0.0;
    double m_curveDiscountAtT = 1.0;
    std::vector<double> m_curveTenors;
    std::vector<double> m_curveZeroRates;

    bool m_hasVol = false;
    double m_volAtPoint = 0.0;

    // Massive GET /v3/reference/options/contracts
    char m_ocUnderlying[32] = "AAPL";
    int m_ocLimit = 10;
    char m_ocExpiration[16] = "";

    // Massive GET /v2/aggs/ticker/.../range/...
    char m_oaOptionsTicker[96] = "O:AAPL211119C00085000";
    int m_oaMultiplier = 1;
    char m_oaTimespan[32] = "day";
    char m_oaFrom[32] = "2021-01-01";
    char m_oaTo[32] = "2021-12-31";
    bool m_oaSendAdjusted = true;
    bool m_oaAdjusted = true;
    char m_oaSort[16] = "";
    int m_oaLimitQuery = 0;

    bool m_hasOptionsContracts = false;
    bool m_hasOptionsAggs = false;
    std::optional<DPP::OptionsContractsEnvelope> m_optionsContractsResult;
    std::optional<DPP::OptionsAggregatesEnvelope> m_optionsAggsResult;
    std::string m_optionsContractsMsg;
    std::string m_optionsAggsMsg;

    void refreshCurveAtT();
    void fetchYieldCurve();
    void fetchVolSurfaceFromAv();
    void fetchOptionsContracts();
    void fetchOptionsAggregates();
};
