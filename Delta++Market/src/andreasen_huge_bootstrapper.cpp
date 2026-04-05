// Andreasen–Huge step-1 bootstrap: sequential Dupire forward solve per expiry on the FD grid.

#include <Delta++Market/andreasen_huge.h>

#include <Delta++Solver/interpolation.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace DPP
{
    namespace
    {
        class AHBootstrapper
        {
        public:
            explicit AHBootstrapper(const AHInput& in) : m_in(in) {}

            std::expected<AHInterpolator, std::string> bootstrap();

        private:
            static constexpr int kMaxIter = 100;
            static constexpr double kTol = 1e-7;

            static bool isStrictlyIncreasing(const std::vector<double>& xs);

            std::expected<void, std::string> validateInput() const;
            std::expected<void, std::string> buildStrikeGrid();
            std::expected<void, std::string> forwardCallSlices();
            void initialGuessW(const std::vector<double>& cTarget, double dT);
            std::expected<void, std::string> solveSlice(const std::vector<double>& cTarget, double dT);
            std::expected<void, std::string> runExpirySlices();

            const AHInput& m_in;
            std::vector<double> m_Kgrid;
            size_t m_nK{};
            size_t m_nT{};
            std::vector<std::vector<double>> m_cMarket;
            std::vector<std::vector<double>> m_localVarGrid;
            std::vector<std::vector<double>> m_cFwdOnGrid;
            std::vector<double> m_dupOpBuf;
            std::vector<double> m_cOld;
            std::vector<double> m_cNew;
            std::vector<double> m_w;
            std::vector<double> m_cBdry;
        };

        bool AHBootstrapper::isStrictlyIncreasing(const std::vector<double>& xs)
        {
            for (size_t i = 1; i < xs.size(); ++i)
                if (!(xs[i] > xs[i - 1]))
                    return false;
            return true;
        }

        std::expected<AHInterpolator, std::string> AHBootstrapper::bootstrap()
        {
            if (auto v = validateInput(); !v)
                return std::unexpected(std::move(v.error()));
            if (auto g = buildStrikeGrid(); !g)
                return std::unexpected(std::move(g.error()));
            if (auto m = forwardCallSlices(); !m)
                return std::unexpected(std::move(m.error()));

            m_localVarGrid.assign(m_nT, std::vector<double>(m_nK, AhDupireFd::kVarMin));
            m_cFwdOnGrid.assign(m_nT, std::vector<double>(m_nK));
            m_dupOpBuf.resize(m_nK);
            m_cOld.resize(m_nK);
            m_cNew.resize(m_nK);
            m_w.assign(m_nK, AhDupireFd::kVarMin);
            m_cBdry.resize(m_nK);

            if (auto s = runExpirySlices(); !s)
                return std::unexpected(std::move(s.error()));

            AhForwardSurfaceData ahData{
                .curve = m_in.curve,
                .kGrid = std::move(m_Kgrid),
                .cFwd = std::move(m_cFwdOnGrid),
                .localVariance = std::move(m_localVarGrid),
            };
            return AHInterpolator::build(m_in.expiries, m_in.strikes, m_in.callPrices, std::move(ahData));
        }

        std::expected<void, std::string> AHBootstrapper::validateInput() const
        {
            if (!(m_in.spot > 0.0))
                return std::unexpected("spot must be > 0");
            if (m_in.expiries.size() < 2)
                return std::unexpected("Need >=2 expiries");
            if (!isStrictlyIncreasing(m_in.expiries))
                return std::unexpected("expiries must be strictly increasing");
            if (m_in.strikes.size() != m_in.expiries.size() || m_in.callPrices.size() != m_in.expiries.size())
                return std::unexpected("strikes/callPrices must match expiries size");
            if (!m_in.dividendYields.empty() && m_in.dividendYields.size() != m_in.expiries.size())
                return std::unexpected("dividendYields must be empty or match expiries size");

            for (size_t i = 0; i < m_in.expiries.size(); ++i)
            {
                const auto& Ks = m_in.strikes[i];
                const auto& Cs = m_in.callPrices[i];
                if (Ks.size() < 3 || Cs.size() != Ks.size())
                    return std::unexpected("Each expiry must have >=3 strikes and matching callPrices");
                for (size_t j = 1; j < Ks.size(); ++j)
                    if (!(Ks[j] > Ks[j - 1]))
                        return std::unexpected("strikes must be strictly increasing within each expiry");
            }
            return {};
        }

        std::expected<void, std::string> AHBootstrapper::buildStrikeGrid()
        {
            m_Kgrid = AhDupireFd::buildLogStrikeGrid(m_in.expiries, m_in.strikes);
            if (m_Kgrid.size() < 3)
                return std::unexpected("Could not build strike grid");
            m_nK = m_Kgrid.size();
            m_nT = m_in.expiries.size();
            m_cMarket.assign(m_nT, std::vector<double>(m_nK));
            return {};
        }

        std::expected<void, std::string> AHBootstrapper::forwardCallSlices()
        {
            for (size_t t_idx = 0; t_idx < m_nT; ++t_idx)
            {
                const double T = m_in.expiries[t_idx];
                const double df = m_in.curve.discount(T);
                if (!(df > 0.0))
                    return std::unexpected("invalid discount factor");

                std::vector<double> cfwd(m_in.strikes[t_idx].size());
                for (size_t k_idx = 0; k_idx < m_in.strikes[t_idx].size(); ++k_idx)
                    cfwd[k_idx] = m_in.callPrices[t_idx][k_idx] / df;

                LinearInterpolator li(m_in.strikes[t_idx], cfwd);
                for (size_t k_idx = 0; k_idx < m_nK; ++k_idx)
                    m_cMarket[t_idx][k_idx] = li(m_Kgrid[k_idx]);
            }
            return {};
        }

        void AHBootstrapper::initialGuessW(const std::vector<double>& cTarget, double dT)
        {
            AhDupireFd::dupireXOperatorTimes(m_Kgrid, cTarget, m_dupOpBuf);
            for (size_t k_idx = 0; k_idx < m_nK; ++k_idx)
            {
                if (k_idx == 0 || k_idx + 1 == m_nK)
                {
                    m_w[k_idx] = AhDupireFd::kVarMin;
                    continue;
                }
                const double denom = dT * std::max(m_dupOpBuf[k_idx], AhDupireFd::kDupireEps);
                const double num = 2.0 * (cTarget[k_idx] - m_cOld[k_idx]);
                if (denom > AhDupireFd::kDupireEps)
                    m_w[k_idx] = std::clamp(num / denom, AhDupireFd::kVarMin, AhDupireFd::kVarMax);
                else
                    m_w[k_idx] = AhDupireFd::kVarMin;
            }
        }

        std::expected<void, std::string> AHBootstrapper::solveSlice(const std::vector<double>& cTarget, double dT)
        {
            double err = 1.0;
            for (int it = 0; it < kMaxIter && err > kTol; ++it)
            {
                if (!AhDupireFd::implicitStep(m_Kgrid, m_cOld, m_cBdry, dT, m_w, m_cNew))
                    return std::unexpected("implicit PDE solve failed");

                AhDupireFd::dupireXOperatorTimes(m_Kgrid, m_cNew, m_dupOpBuf);

                for (size_t k_idx = 1; k_idx + 1 < m_nK; ++k_idx)
                {
                    const double denom = dT * std::max(m_dupOpBuf[k_idx], AhDupireFd::kDupireEps);
                    const double num = 2.0 * (cTarget[k_idx] - m_cOld[k_idx]);
                    if (denom > AhDupireFd::kDupireEps)
                        m_w[k_idx] = std::clamp(num / denom, AhDupireFd::kVarMin, AhDupireFd::kVarMax);
                }
                m_w[0] = m_w[1];
                m_w[m_nK - 1] = m_w[m_nK - 2];

                err = 0.0;
                for (size_t k_idx = 1; k_idx + 1 < m_nK; ++k_idx)
                {
                    const double scale = std::max(1.0, std::abs(cTarget[k_idx]));
                    err = std::max(err, std::abs(m_cNew[k_idx] - cTarget[k_idx]) / scale);
                }
            }
            return {};
        }

        std::expected<void, std::string> AHBootstrapper::runExpirySlices()
        {
            for (size_t t_idx = 0; t_idx < m_nT; ++t_idx)
            {
                const double Tprev = (t_idx == 0) ? 0.0 : m_in.expiries[t_idx - 1];
                const double Tcur = m_in.expiries[t_idx];
                const double dT = Tcur - Tprev;
                if (!(dT > 0.0))
                    return std::unexpected("non-positive time step");

                const std::vector<double>& cTarget = m_cMarket[t_idx];

                if (t_idx == 0)
                    std::transform(m_Kgrid.begin(), m_Kgrid.end(), m_cOld.begin(),
                                   [spot = m_in.spot](double K) { return std::max(spot - K, 0.0); });
                else
                    std::copy(m_cMarket[t_idx - 1].begin(), m_cMarket[t_idx - 1].end(), m_cOld.begin());

                m_cBdry = cTarget;

                initialGuessW(cTarget, dT);

                if (auto solved = solveSlice(cTarget, dT); !solved)
                    return std::unexpected(std::move(solved.error()));

                for (size_t k_idx = 0; k_idx < m_nK; ++k_idx)
                    m_localVarGrid[t_idx][k_idx] = m_w[k_idx];
                m_cFwdOnGrid[t_idx] = m_cNew;
            }
            return {};
        }
    }

    std::expected<AHInterpolator, std::string> bootstrapAndreasenHuge(const AHInput& in)
    {
        return AHBootstrapper(in).bootstrap();
    }
}
