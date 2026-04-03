#include <Delta++Market/dividend_yield_curve.h>

namespace DPP
{
    double DividendYieldCurve::q(const double tYears) const
    {
        if (m_constantOnly)
            return m_constantQ;
        if (m_spline.has_value())
            return (*m_spline)(tYears);
        return m_constantQ;
    }
}
