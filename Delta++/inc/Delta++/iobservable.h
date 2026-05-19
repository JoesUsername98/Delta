#pragma once

#include <cstdint>
#include <future>
#include <memory>

#include "iobserver.h"

namespace DPP
{
    // Abstract observable: concrete strategies (sync, async, thread-pool, ...)
    // can be swapped without changing call sites.
    template <class T>
    class IObservable
    {
    public:
        struct Token
        {
            std::uint64_t id{};
        };

        virtual ~IObservable() = default;

        // Subscribers are passed in as weak_ptr so the observable never extends
        // observer lifetime past its owner.
        [[nodiscard]] virtual Token subscribe(std::weak_ptr<IObserver<T>> observer) = 0;
        virtual void unsubscribe(Token token) = 0;

        virtual void publish(const T& value) = 0;

        // Fan-out off the caller's thread; returned future signals when every
        // live observer has been notified.
        [[nodiscard]] virtual std::future<void> publishAsync(T value) = 0;

        // Promise-backed one-shot: resolves with the next published value.
        [[nodiscard]] virtual std::future<T> nextValue() = 0;
    };
} // namespace DPP
