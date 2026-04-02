#pragma once

#include <Delta++Solver/interpolation.h>

#include <expected>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace DPP
{
    class YieldCurve;

    /// One expiry’s paired put/call mids used for parity q(T).
    struct ParityExpiryInput
    {
        double tYears{};
        std::string expirationDate;
        std::vector<double> strikes;
        std::vector<double> callMids;
        std::vector<double> putMids;
    };

    /// Per-expiry parity diagnostics (maps to UI parity table).
    struct DividendYieldPillarDiag
    {
        std::string expirationDate;
        double texp_years{};
        double r{};
        double q{};
        double A{};
        double B{};
        double forward{};
        int nUsed{};
        double rmse{};
    };

    /// Interpolated dividend yield q(T) from put–call parity pillars (cubic spline on knots; flat if one knot).
    class DividendYieldCurve
    {
    public:
        double q(double tYears) const;

        const std::vector<double>& tenors() const { return m_tenors; }
        const std::vector<double>& qKnots() const { return m_qKnots; }

    private:
        struct Internal
        {
        };

        friend std::expected<struct DividendYieldCurveBuildResult, std::string> buildDividendYieldCurveFromParity(
            double spot,
            const YieldCurve& rates,
            std::span<const ParityExpiryInput> pillars);

        explicit DividendYieldCurve(Internal) {}

        std::vector<double> m_tenors;
        std::vector<double> m_qKnots;
        bool m_constantOnly = false;
        double m_constantQ = 0.0;
        std::optional<CubicSplineInterpolator> m_spline;
    };

    struct DividendYieldCurveBuildResult
    {
        DividendYieldCurve curve;
        std::vector<DividendYieldPillarDiag> pillars;
    };

    /// For each pillar: sort paired quotes by strike, fit parity, then build q(T) on sorted unique tenors.
    /// Fails if `pillars` is empty (no inputs to process).
    std::expected<DividendYieldCurveBuildResult, std::string> buildDividendYieldCurveFromParity(
        double spot,
        const YieldCurve& rates,
        std::span<const ParityExpiryInput> pillars);
}
