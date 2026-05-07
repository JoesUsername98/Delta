// observable_practice_tests.cpp
//
// Self-contained practice file: a thread-safe abstract IObservable<T> +
// concrete Observable<T> using shared_ptr / weak_ptr, a demo MarketData
// class, and GoogleTest cases that exercise std::future, std::promise,
// std::packaged_task and std::async.
//
// Everything (interfaces, implementation, demo, tests) lives in this one
// translation unit so the pattern can be studied in isolation.

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace DPP
{
    namespace Practice
    {
        // ---------------------------------------------------------------------
        // IObserver<T>
        //
        // Pure-virtual receiver of events. Modeled loosely on Rx's IObserver:
        // onNext for values, onError for terminal error, onComplete for
        // terminal completion. Owners hold shared_ptr<IObserver<T>>; the
        // observable side stores weak_ptr only.
        // ---------------------------------------------------------------------
        template <class T>
        class IObserver
        {
        public:
            virtual ~IObserver() = default;

            virtual void onNext(const T& value) = 0;
            virtual void onError(std::exception_ptr error) = 0;
            virtual void onComplete() = 0;
        };

        // ---------------------------------------------------------------------
        // IObservable<T>
        //
        // The abstract observable C++ doesn't ship with. All operations are
        // pure virtual so concrete strategies (sync, async, thread-pool, ...)
        // can be swapped without changing call sites.
        // ---------------------------------------------------------------------
        template <class T>
        class IObservable
        {
        public:
            struct Token
            {
                std::uint64_t id{};
            };

            virtual ~IObservable() = default;

            // Subscribers are passed in as weak_ptr so the observable never
            // extends observer lifetime past its owner.
            [[nodiscard]] virtual Token subscribe(std::weak_ptr<IObserver<T>> observer) = 0;
            virtual void unsubscribe(Token token) = 0;

            virtual void publish(const T& value) = 0;

            // Fan-out off the caller's thread; returned future signals when
            // every live observer has been notified.
            [[nodiscard]] virtual std::future<void> publishAsync(T value) = 0;

            // Promise-backed one-shot: resolves with the next published value.
            [[nodiscard]] virtual std::future<T> nextValue() = 0;
        };

        // ---------------------------------------------------------------------
        // Observable<T>
        //
        // Thread-safe implementation:
        //   * subscribers held as weak_ptr; expired entries are pruned on each
        //     publish() so dropped observers are reclaimed automatically.
        //   * publish() snapshots subscribers under the lock and dispatches
        //     outside the lock - subscribers may safely call back into
        //     subscribe()/unsubscribe() during a notification.
        //   * publishAsync() wraps publish() in a std::packaged_task and
        //     launches it through std::async(std::launch::async, ...). The
        //     async-launched future is parked on the observable for the
        //     lifetime of the work to avoid std::async's destructor blocking
        //     surprise; the packaged_task's future is what we return.
        // ---------------------------------------------------------------------
        template <class T>
        class Observable : public IObservable<T>
        {
        public:
            using Token = typename IObservable<T>::Token;

            ~Observable() override
            {
                // m_pending is destroyed in declaration order below; its
                // destructor blocks on every in-flight async fan-out, ensuring
                // no detached task outlives this object.
            }

            [[nodiscard]] Token subscribe(std::weak_ptr<IObserver<T>> observer) override
            {
                const auto id = m_nextId.fetch_add(1, std::memory_order_relaxed);
                std::lock_guard<std::mutex> lk(m_mtx);
                m_subs.push_back(Entry{Token{id}, std::move(observer)});
                return Token{id};
            }

            void unsubscribe(Token token) override
            {
                std::lock_guard<std::mutex> lk(m_mtx);
                std::erase_if(m_subs,
                              [&](const Entry& e) { return e.tok.id == token.id; });
            }

            void publish(const T& value) override
            {
                std::vector<std::shared_ptr<IObserver<T>>> live;
                {
                    std::lock_guard<std::mutex> lk(m_mtx);
                    std::erase_if(m_subs,
                                  [](const Entry& e) { return e.obs.expired(); });
                    live.reserve(m_subs.size());
                    for (auto& e : m_subs)
                    {
                        if (auto sp = e.obs.lock())
                            live.push_back(std::move(sp));
                    }
                }

                for (auto& sp : live)
                {
                    try
                    {
                        sp->onNext(value);
                    }
                    catch (...)
                    {
                        // Per-observer isolation: a misbehaving subscriber
                        // must not break fan-out for the rest.
                        try
                        {
                            sp->onError(std::current_exception());
                        }
                        catch (...)
                        {
                        }
                    }
                }
            }

            [[nodiscard]] std::future<void> publishAsync(T value) override
            {
                std::packaged_task<void()> task(
                    [this, v = std::move(value)]() mutable { this->publish(v); });
                std::future<void> caller = task.get_future();

                std::lock_guard<std::mutex> lk(m_pendingMtx);
                trimReadyLocked();
                m_pending.push_back(std::async(
                    std::launch::async,
                    [t = std::move(task)]() mutable { t(); }));

                return caller;
            }

            [[nodiscard]] std::future<T> nextValue() override
            {
                auto promise = std::make_shared<std::promise<T>>();
                std::future<T> fut = promise->get_future();

                // One-shot helper observer: fulfils the promise on first
                // onNext / onError, then asks the parent to drop it.
                class OneShot final : public IObserver<T>
                {
                public:
                    std::shared_ptr<std::promise<T>> p;
                    Observable<T>* owner{};
                    Token tok{};
                    std::atomic<bool> fired{false};

                    void onNext(const T& v) override
                    {
                        bool expected = false;
                        if (!fired.compare_exchange_strong(expected, true))
                            return;
                        try
                        {
                            p->set_value(v);
                        }
                        catch (...)
                        {
                        }
                        owner->dropOneShot(tok);
                    }

                    void onError(std::exception_ptr e) override
                    {
                        bool expected = false;
                        if (!fired.compare_exchange_strong(expected, true))
                            return;
                        try
                        {
                            p->set_exception(e);
                        }
                        catch (...)
                        {
                        }
                        owner->dropOneShot(tok);
                    }

                    void onComplete() override {}
                };

                auto obs = std::make_shared<OneShot>();
                obs->p = promise;
                obs->owner = this;

                Token tok = this->subscribe(std::weak_ptr<IObserver<T>>(obs));
                obs->tok = tok;

                {
                    std::lock_guard<std::mutex> lk(m_oneShotMtx);
                    m_oneShots[tok.id] = obs;
                }

                return fut;
            }

            [[nodiscard]] std::size_t subscriberCount() const
            {
                std::lock_guard<std::mutex> lk(m_mtx);
                return m_subs.size();
            }

        private:
            struct Entry
            {
                Token tok;
                std::weak_ptr<IObserver<T>> obs;
            };

            void dropOneShot(Token token)
            {
                this->unsubscribe(token);
                std::lock_guard<std::mutex> lk(m_oneShotMtx);
                m_oneShots.erase(token.id);
            }

            // Drop already-ready async fan-outs so m_pending doesn't grow
            // without bound. Caller must hold m_pendingMtx.
            void trimReadyLocked()
            {
                std::erase_if(m_pending,
                              [](std::future<void>& f)
                              {
                                  return f.valid() &&
                                         f.wait_for(std::chrono::seconds(0)) ==
                                             std::future_status::ready;
                              });
            }

            mutable std::mutex m_mtx;
            std::vector<Entry> m_subs;
            std::atomic<std::uint64_t> m_nextId{1};

            // Owning storage for one-shot observers (lifetime managed here so
            // the weak_ptr in m_subs stays lockable until the value lands).
            std::mutex m_oneShotMtx;
            std::unordered_map<std::uint64_t, std::shared_ptr<IObserver<T>>> m_oneShots;

            // Async work parked here so std::async's blocking destructor only
            // fires when this Observable is destroyed - making destruction a
            // clean barrier for any in-flight fan-out.
            std::mutex m_pendingMtx;
            std::vector<std::future<void>> m_pending;
        };

        // ---------------------------------------------------------------------
        // Demo: MarketData
        //
        // Concrete user of the abstract IObservable: produces ticks at a
        // monotonically increasing sequence number when the price is set.
        // ---------------------------------------------------------------------
        struct Tick
        {
            std::uint64_t seq{};
            double price{};
        };

        class MarketData : public Observable<Tick>
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

        // ---------------------------------------------------------------------
        // Test helper: an observer that records every value it sees.
        // ---------------------------------------------------------------------
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
    } // namespace Practice
} // namespace DPP

using DPP::Practice::MarketData;
using DPP::Practice::Observable;
using DPP::Practice::RecordingObserver;
using DPP::Practice::Tick;

// =====================================================================
// Tests
// =====================================================================

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
    } // observer's shared_ptr dies here; the weak_ptr inside src is now expired.

    src.publish(2); // must not crash; expired weak_ptr should be pruned
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

    EXPECT_EQ(stable->count(),
              static_cast<std::size_t>(kPublishers * kPerThread));
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

    // Drive the publish from another thread using std::async to demonstrate
    // promise/future rendezvous across threads.
    std::future<void> bg =
        std::async(std::launch::async, [&] { src.publish(99); });
    bg.get();

    ASSERT_EQ(fut.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    EXPECT_EQ(fut.get(), 99);

    // After firing, the one-shot observer should have removed itself.
    EXPECT_EQ(src.subscriberCount(), 0u);
}

TEST(MarketData_Demo, PriceUpdatesFanOutToMultipleObservers)
{
    MarketData md;
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
    MarketData md;
    auto obs = std::make_shared<RecordingObserver<Tick>>();
    (void)md.subscribe(obs);

    std::future<void> fut = md.setPriceAsync(123.45);
    ASSERT_EQ(fut.wait_for(std::chrono::seconds(2)), std::future_status::ready);
    fut.get();

    ASSERT_EQ(obs->count(), 1u);
    EXPECT_DOUBLE_EQ(obs->values()[0].price, 123.45);
    EXPECT_EQ(obs->values()[0].seq, 0u);
}
