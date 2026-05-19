#include <gtest/gtest.h>

#include <Delta++/observable.h>

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <thread>
#include <vector>

#include "observable_test_helpers.h"
#include <random>
#include <Delta++Math/distributions.h>
#include <Delta++/monte_carlo_path_schemes.h>

using DPP::ObservableMarketData;
using DPP::Observable;
using DPP::RecordingObserver;
using DPP::Tick;

TEST(IObservable_Basic, SubscribeAndPublishCallsObserver)
{
    Observable<int> src;
    auto obs = std::make_shared<RecordingObserver<int>>();

    auto tok = src.subscribe(obs);
    src.publish(7);
    src.publish(42);

    ASSERT_EQ(obs->count(), 2u);
    EXPECT_EQ(obs->values()[0], 7);
    EXPECT_EQ(obs->values()[1], 42);

    src.unsubscribe(tok);
    src.publish(100);
    EXPECT_EQ(obs->count(), 2u);
}

TEST(IObservable_WeakPtr, ExpiredObserverIsPruned)
{
    Observable<int> src;

    {
        auto obs = std::make_shared<RecordingObserver<int>>();
        (void)src.subscribe(obs);
        src.publish(1);
        EXPECT_EQ(obs->count(), 1u);
        EXPECT_EQ(src.subscriberCount(), 1u);
    }

    src.publish(2);
    EXPECT_EQ(src.subscriberCount(), 0u);
}

TEST(IObservable_Threading, ConcurrentPublishAndSubscribeIsRaceFree)
{
    Observable<int> src;
    auto stable = std::make_shared<RecordingObserver<int>>();
    (void)src.subscribe(stable);

    constexpr int kPublishers = 4;
    constexpr int kPerThread = 1000;
    constexpr int kChurners = 4;

    std::atomic<bool> stopChurn{false};

    std::vector<std::thread> publishers;
    publishers.reserve(kPublishers);
    for (int t = 0; t < kPublishers; ++t)
    {
        publishers.emplace_back(
            [&, t]
            {
                for (int i = 0; i < kPerThread; ++i)
                    src.publish(t * kPerThread + i);
            });
    }

    std::vector<std::thread> churners;
    churners.reserve(kChurners);
    for (int t = 0; t < kChurners; ++t)
    {
        churners.emplace_back(
            [&]
            {
                while (!stopChurn.load(std::memory_order_relaxed))
                {
                    auto local = std::make_shared<RecordingObserver<int>>();
                    auto tok = src.subscribe(local);
                    std::this_thread::yield();
                    src.unsubscribe(tok);
                }
            });
    }

    for (auto& th : publishers)
        th.join();
    stopChurn.store(true, std::memory_order_relaxed);
    for (auto& th : churners)
        th.join();

    EXPECT_EQ(stable->count(), static_cast<std::size_t>(kPublishers * kPerThread));
    EXPECT_EQ(stable->errors(), 0);
}

TEST(IObservable_Async, PublishAsyncReturnsFutureAndCompletes)
{
    Observable<int> src;
    auto obs = std::make_shared<RecordingObserver<int>>();
    (void)src.subscribe(obs);

    std::future<void> f1 = src.publishAsync(11);
    std::future<void> f2 = src.publishAsync(22);

    ASSERT_EQ(f1.wait_for(std::chrono::seconds(2)), std::future_status::ready);
    ASSERT_EQ(f2.wait_for(std::chrono::seconds(2)), std::future_status::ready);
    f1.get();
    f2.get();

    EXPECT_EQ(obs->count(), 2u);
}

TEST(IObservable_Async, NextValueResolvesPromise)
{
    Observable<int> src;

    std::future<int> fut = src.nextValue();

    std::future<void> bg =
        std::async(std::launch::async, [&] { src.publish(99); });
    bg.get();

    ASSERT_EQ(fut.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    EXPECT_EQ(fut.get(), 99);

    EXPECT_EQ(src.subscriberCount(), 0u);
}

TEST(MarketData_Demo, PriceUpdatesFanOutToMultipleObservers)
{
    ObservableMarketData md;
    auto a = std::make_shared<RecordingObserver<Tick>>();
    auto b = std::make_shared<RecordingObserver<Tick>>();
    auto c = std::make_shared<RecordingObserver<Tick>>();

    (void)md.subscribe(a);
    (void)md.subscribe(b);
    (void)md.subscribe(c);

    md.setPrice(101.0);
    md.setPrice(102.5);
    md.setPrice(103.25);

    for (const auto& obs : {a, b, c})
    {
        ASSERT_EQ(obs->count(), 3u);
        const auto v = obs->values();
        EXPECT_EQ(v[0].seq, 0u);
        EXPECT_DOUBLE_EQ(v[0].price, 101.0);
        EXPECT_EQ(v[1].seq, 1u);
        EXPECT_DOUBLE_EQ(v[1].price, 102.5);
        EXPECT_EQ(v[2].seq, 2u);
        EXPECT_DOUBLE_EQ(v[2].price, 103.25);
    }
}

TEST(MarketData_Demo, SetPriceAsyncWaitableViaFuture)
{
    ObservableMarketData md;
    auto obs = std::make_shared<RecordingObserver<Tick>>();
    (void)md.subscribe(obs);

    std::future<void> fut = md.setPriceAsync(123.45);
    ASSERT_EQ(fut.wait_for(std::chrono::seconds(2)), std::future_status::ready);
    fut.get();

    ASSERT_EQ(obs->count(), 1u);
    EXPECT_DOUBLE_EQ(obs->values()[0].price, 123.45);
    EXPECT_EQ(obs->values()[0].seq, 0u);
}

// Takes ticks, averages them and after 10, retransmits the averages out.
class AverageObserver final : public DPP::IObserver<DPP::Tick>, public DPP::Observable<DPP::Tick>
{
public:

    //IObserver
    void onNext(const DPP::Tick& tick) override
    {
        m_sum.fetch_add(tick.price, std::memory_order_relaxed);
        if (m_count.fetch_add(1, std::memory_order_relaxed) > 10)
        {
            const auto seq = m_seq.fetch_add(1, std::memory_order_relaxed);
            std::lock_guard<std::mutex> lk(m_mtx);
            publish( 
                Tick{ 
                    seq,
                    m_sum.load(std::memory_order_relaxed) / m_count.load(std::memory_order_relaxed) 
                }
            );
        }
    }

    void onError(std::exception_ptr) override
    {
        m_errors.fetch_add(1, std::memory_order_relaxed);
    }

    void onComplete() override
    {
        m_completes.fetch_add(1, std::memory_order_relaxed);
    }

    // Domain 
    double average() const
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_sum.load(std::memory_order_relaxed) / m_count.load(std::memory_order_relaxed);
    }

    unsigned long long count() const
    {
        return m_count.load(std::memory_order_relaxed);
    }

private:
    mutable std::mutex m_mtx;
    std::atomic<std::uint64_t> m_seq{ 0 };
    std::atomic<int> m_errors{ 0 };
    std::atomic<int> m_completes{ 0 };
    std::atomic<double> m_sum{ 0. };
    std::atomic<unsigned long long> m_count{ 0 };
};

TEST(MarketData_Demo, Playground)
{
    ObservableMarketData md;
    auto obs = std::make_shared<AverageObserver>();
    auto avgRecorder = std::make_shared<RecordingObserver<Tick>>();
    (void)md.subscribe(obs);
    (void)obs->subscribe(avgRecorder);
    DPP::EulerScheme es;
    double s = 100;

    std::seed_seq seq{ 42 };
    std::mt19937_64 rng{ seq };
    std::uniform_real_distribution<double> unif(0.0, 1.0);

    for (size_t i = 0; i < 999; ++i)
    {
        auto z = DPPMath::invCumDensity(unif(rng));
        es.updatePrice(s, z, 0.02, 0., 0.2, 1.);
        md.setPrice(s);

        // ObservableMarketData sends data to the AverageObserver
        // The AverageObserver calculates the average price ticked by ObservableMarketData and retransmitts it
        // the avgRecorder simply takes the retransmitted AverageObserver 
    }

    ASSERT_EQ(obs->count(), 999);
    const auto average = obs->average();
    ASSERT_EQ(average, 103.33415018848105);
    ASSERT_EQ(avgRecorder->count(), 988);
    ASSERT_EQ(avgRecorder->values().back().price, average);
}