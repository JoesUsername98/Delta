// Andreasen–Huge step-1 bootstrap: sequential Dupire forward solve per expiry on the FD grid.

#include <Delta++Market/andreasen_huge.h>

#include <Delta++Solver/interpolation.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <ranges>
#include <span>
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
            return std::ranges::is_sorted(xs, std::ranges::less{});
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
            if (!(m_in.expiries.front() > 0.0))
                return std::unexpected("first expiry must be > 0");
            if (!isStrictlyIncreasing(m_in.expiries))
                return std::unexpected("expiries must be strictly increasing");
            if (m_in.strikes.size() != m_in.expiries.size() || m_in.callPrices.size() != m_in.expiries.size())
                return std::unexpected("strikes/callPrices must match expiries size");
            if (!m_in.dividendYields.empty() && m_in.dividendYields.size() != m_in.expiries.size())
                return std::unexpected("dividendYields must be empty or match expiries size");

            for (const auto& [Ks, Cs] : std::views::zip(m_in.strikes, m_in.callPrices))
            {
                if (Ks.size() < 3 || Cs.size() != Ks.size())
                    return std::unexpected("Each expiry must have >=3 strikes and matching callPrices");
                if (!isStrictlyIncreasing(Ks))
                    return std::unexpected("strikes must be strictly increasing within each expiry");
            }
            return {};
        }

        std::expected<void, std::string> AHBootstrapper::buildStrikeGrid()
        {
            m_Kgrid = AhDupireFd::buildLogStrikeGrid(m_in.expiries, m_in.strikes);
            if (m_Kgrid.empty())
                return std::unexpected("Could not build strike grid");
            m_nK = m_Kgrid.size();
            m_nT = m_in.expiries.size();
            m_cMarket.assign(m_nT, std::vector<double>(m_nK));
            return {};
        }

        std::expected<void, std::string> AHBootstrapper::forwardCallSlices()
        {
            for (size_t t_idx : std::views::iota(0uz, m_nT))
            {
                const double T = m_in.expiries[t_idx];
                const double df = m_in.curve.discount(T);
                if (!(df > 0.0))
                    return std::unexpected("invalid discount factor");

                const auto& prices = m_in.callPrices[t_idx];
                std::vector<double> cfwd(prices.size());
                std::ranges::transform(prices, cfwd.begin(), [df](double c) { return c / df; });

                LinearInterpolator li(m_in.strikes[t_idx], cfwd);
                std::ranges::transform(m_Kgrid, m_cMarket[t_idx].begin(), [&](double K) { return li(K); });
            }
            return {};
        }

        void AHBootstrapper::initialGuessW(const std::vector<double>& cTarget, double dT)
        {
            AhDupireFd::dupireXOperatorTimes(m_Kgrid, cTarget, m_dupOpBuf);
            std::ranges::transform(
                std::views::iota(0uz, m_nK),
                m_w.begin(),
                [&](size_t k_idx) -> double {
                    if (k_idx == 0 || k_idx + 1 == m_nK)
                        return AhDupireFd::kVarMin;
                    const double denom = dT * std::max(m_dupOpBuf[k_idx], AhDupireFd::kDupireEps);
                    const double num = 2.0 * (cTarget[k_idx] - m_cOld[k_idx]);
                    if (denom > AhDupireFd::kDupireEps)
                        return std::clamp(num / denom, AhDupireFd::kVarMin, AhDupireFd::kVarMax);
                    return AhDupireFd::kVarMin;
                });
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
                if (m_nK > 2)
                {
                    const auto ctIn = std::span<const double>(cTarget).subspan(1, m_nK - 2);
                    const auto cnIn = std::span<const double>(m_cNew).subspan(1, m_nK - 2);
                    err = std::ranges::max(
                        std::views::zip(ctIn, cnIn)
                        | std::views::transform([](const auto& z) {
                              const auto& [ct, cn] = z;
                              const double scale = std::max(1.0, std::abs(ct));
                              return std::abs(cn - ct) / scale;
                          }));
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

                const std::vector<double>& cTarget = m_cMarket[t_idx];

                if (t_idx == 0)
                    std::ranges::transform(m_Kgrid, m_cOld.begin(),
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
