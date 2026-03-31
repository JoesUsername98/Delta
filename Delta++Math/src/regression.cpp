#include <Delta++Math/regression.h>

#include <cmath>
#include <limits>

namespace
{
    bool finite(double x) { return std::isfinite(x); }
}

namespace DPPMath
{
    std::expected<OlsLineFit, std::string> olsFitLine(
        const std::vector<double>& xs,
        const std::vector<double>& ys,
        const int minN)
    {
        if (xs.size() != ys.size())
            return std::unexpected("xs and ys must have same size");
        if (minN < 2)
            return std::unexpected("minN must be >= 2");

        double sumX = 0.0, sumY = 0.0, sumXX = 0.0, sumXY = 0.0;
        int n = 0;
        for (size_t i = 0; i < xs.size(); ++i)
        {
            const double x = xs[i];
            const double y = ys[i];
            if (!finite(x) || !finite(y))
                continue;
            ++n;
            sumX += x;
            sumY += y;
            sumXX += x * x;
            sumXY += x * y;
        }

        if (n < minN)
            return std::unexpected("not enough finite samples for OLS fit");

        const double denom = n * sumXX - sumX * sumX;
        if (!(std::abs(denom) > 1e-14))
            return std::unexpected("degenerate x set for OLS fit");

        const double slope = (n * sumXY - sumX * sumY) / denom;
        const double intercept = (sumY - slope * sumX) / static_cast<double>(n);

        double sse = 0.0;
        int nRmse = 0;
        for (size_t i = 0; i < xs.size(); ++i)
        {
            const double x = xs[i];
            const double y = ys[i];
            if (!finite(x) || !finite(y))
                continue;
            const double yhat = intercept + slope * x;
            const double e = y - yhat;
            sse += e * e;
            ++nRmse;
        }

        const double rmse = (nRmse > 0) ? std::sqrt(sse / static_cast<double>(nRmse))
                                        : std::numeric_limits<double>::quiet_NaN();

        OlsLineFit out;
        out.intercept = intercept;
        out.slope = slope;
        out.nUsed = n;
        out.rmse = rmse;
        return out;
    }
}

