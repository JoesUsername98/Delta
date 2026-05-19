#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <future>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "iobservable.h"

namespace DPP
{
    // Thread-safe implementation:
    //   * subscribers held as weak_ptr; expired entries are pruned on each
    //     publish() so dropped observers are reclaimed automatically.
    //   * publish() snapshots subscribers under the lock and dispatches
    //     outside the lock - subscribers may safely call back into
    //     subscribe()/unsubscribe() during a notification.
    //   * publishAsync() wraps publish() in a std::packaged_task and launches
    //     it through std::async(std::launch::async, ...). The async-launched
    //     future is parked on the observable for the lifetime of the work to
    //     avoid std::async's destructor blocking surprise; the packaged_task's
    //     future is what we return.
    template <class T>
    class Observable : public IObservable<T>
    {
    public:
        using Token = typename IObservable<T>::Token;

        ~Observable() override;

        [[nodiscard]] Token subscribe(std::weak_ptr<IObserver<T>> observer) override;
        void unsubscribe(Token token) override;

        void publish(const T& value) override;

        [[nodiscard]] std::future<void> publishAsync(T value) override;

        [[nodiscard]] std::future<T> nextValue() override;

        [[nodiscard]] std::size_t subscriberCount() const;

    private:
        struct Entry
        {
            Token tok;
            std::weak_ptr<IObserver<T>> obs;
        };

        void dropOneShot(Token token);

        // Drop already-ready async fan-outs so m_pending doesn't grow without
        // bound. Caller must hold m_pendingMtx.
        void trimReadyLocked();

        mutable std::mutex m_mtx;
        std::vector<Entry> m_subs;
        std::atomic<std::uint64_t> m_nextId{1};

        // Owning storage for one-shot observers (lifetime managed here so the
        // weak_ptr in m_subs stays lockable until the value lands).
        std::mutex m_oneShotMtx;
        std::unordered_map<std::uint64_t, std::shared_ptr<IObserver<T>>> m_oneShots;

        // Async work parked here so std::async's blocking destructor only fires
        // when this Observable is destroyed - making destruction a clean barrier
        // for any in-flight fan-out.
        std::mutex m_pendingMtx;
        std::vector<std::future<void>> m_pending;
    };

    template <class T>
    Observable<T>::~Observable()
    {
        // m_pending is destroyed in declaration order; its destructor blocks on
        // every in-flight async fan-out, ensuring no detached task outlives this
        // object.
    }

    template <class T>
    auto Observable<T>::subscribe(std::weak_ptr<IObserver<T>> observer) -> Token
    {
        const auto id = m_nextId.fetch_add(1, std::memory_order_relaxed);
        std::lock_guard<std::mutex> lk(m_mtx);
        m_subs.push_back(Entry{Token{id}, std::move(observer)});
        return Token{id};
    }

    template <class T>
    void Observable<T>::unsubscribe(Token token)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        std::erase_if(m_subs, [&](const Entry& e) { return e.tok.id == token.id; });
    }

    template <class T>
    void Observable<T>::publish(const T& value)
    {
        std::vector<std::shared_ptr<IObserver<T>>> live;
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            std::erase_if(m_subs, [](const Entry& e) { return e.obs.expired(); });
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

    template <class T>
    auto Observable<T>::publishAsync(T value) -> std::future<void>
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

    template <class T>
    auto Observable<T>::nextValue() -> std::future<T>
    {
        auto promise = std::make_shared<std::promise<T>>();
        std::future<T> fut = promise->get_future();

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

    template <class T>
    std::size_t Observable<T>::subscriberCount() const
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_subs.size();
    }

    template <class T>
    void Observable<T>::dropOneShot(Token token)
    {
        this->unsubscribe(token);
        std::lock_guard<std::mutex> lk(m_oneShotMtx);
        m_oneShots.erase(token.id);
    }

    template <class T>
    void Observable<T>::trimReadyLocked()
    {
        std::erase_if(m_pending,
                      [](std::future<void>& f)
                      {
                          return f.valid() &&
                                 f.wait_for(std::chrono::seconds(0)) ==
                                     std::future_status::ready;
                      });
    }
} // namespace DPP
