#pragma once

#include <Delta++/observable.h>

#include <atomic>
#include <cstdint>
#include <future>
#include <mutex>
#include <vector>

namespace DPP
{
    struct Tick
    {
        std::uint64_t seq{};
        double price{};
    };

    class ObservableMarketData : public Observable<Tick>
    {
    public:
        void setPrice(double price)
        {
            const auto seq = m_seq.fetch_add(1, std::memory_order_relaxed);
            this->publish(Tick{seq, price});
        }

        [[nodiscard]] std::future<void> setPriceAsync(double price)
        {
            const auto seq = m_seq.fetch_add(1, std::memory_order_relaxed);
            return this->publishAsync(Tick{seq, price});
        }

    private:
        std::atomic<std::uint64_t> m_seq{0};
    };

    template <class T>
    class RecordingObserver final : public IObserver<T>
    {
    public:
        void onNext(const T& v) override
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_values.push_back(v);
        }

        void onError(std::exception_ptr) override
        {
            m_errors.fetch_add(1, std::memory_order_relaxed);
        }

        void onComplete() override
        {
            m_completes.fetch_add(1, std::memory_order_relaxed);
        }

        [[nodiscard]] std::vector<T> values() const
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            return m_values;
        }

        [[nodiscard]] std::size_t count() const
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            return m_values.size();
        }

        [[nodiscard]] int errors() const
        {
            return m_errors.load(std::memory_order_relaxed);
        }

    private:
        mutable std::mutex m_mtx;
        std::vector<T> m_values;
        std::atomic<int> m_errors{0};
        std::atomic<int> m_completes{0};
    };
} // namespace DPP
