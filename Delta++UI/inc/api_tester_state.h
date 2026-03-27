#pragma once

#include <Delta++Market/yield_curve.h>

#include <optional>
#include <string>
#include <vector>

struct ApiTesterState
{
    bool m_useStub = true;

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

    void refreshCurveAtT();
    void fetchYieldCurveFromFred();
    void fetchVolSurfaceFromAv();
};
