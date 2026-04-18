#include <Delta++Market/dividend_yield_curve.h>

namespace DPP
{
    DividendYieldCurve DividendYieldCurve::flat(const double q)
    {
        DividendYieldCurve c;
        c.m_constantOnly = true;
        c.m_constantQ = q;
        c.m_tenors.clear();
        c.m_qKnots.clear();
        c.m_spline.reset();
        return c;
    }

    double DividendYieldCurve::q(const double tYears) const
    {
        if (m_constantOnly)
            return m_constantQ;
        if (m_spline.has_value())
            return (*m_spline)(tYears);
        return m_constantQ;
    }
}
