#pragma once

#include <exception>

namespace DPP
{
    // Pure-virtual receiver of events. Modeled loosely on Rx's IObserver:
    // onNext for values, onError for terminal error, onComplete for terminal
    // completion. Owners hold shared_ptr<IObserver<T>>; the observable side
    // stores weak_ptr only.
    template <class T>
    class IObserver
    {
    public:
        virtual ~IObserver() = default;

        virtual void onNext(const T& value) = 0;
        virtual void onError(std::exception_ptr error) = 0;
        virtual void onComplete() = 0;
    };
} // namespace DPP
