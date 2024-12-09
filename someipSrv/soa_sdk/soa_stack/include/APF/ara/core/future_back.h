#ifndef _FUTURE_H_
#define _FUTURE_H_
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <system_error>
#include <thread>
#include <utility>
#include "error_code.h"
#include "future_error_domain.h"
#include "Try.h"
#include "Helper.h"

#ifndef _STD
#define _STD       ::std::
#endif

namespace ara {
namespace core {
    class Scheduler {
    public:
        virtual ~Scheduler() {}

        /**
         * The following functions need not to be thread-safe.
         * It is nonsense that schedule callback and submit request are in different threads.
         * Do it like this:
         * e.g.
         *
         * // In this_loop thread.
         *
         * Future<Buffer> ft(ReadFileInSeparateThread(very_big_file));
         *
         * ft.then(&this_loop, [](const Buffer& file_contents) {
         *      // SUCCESS : process file_content;
         * })
         * .OnTimeout(std::chrono::seconds(10), [=very_big_file]() {
         *      // FAILED OR TIMEOUT:
         *      printf("Read file %s failed\n", very_big_file);
         * },
         * &this_loop);
         */
        virtual void ScheduleLater(std::chrono::milliseconds duration, std::function<void()> f) = 0;
        virtual void Schedule(std::function<void()> f) = 0;
    };
    //[SWS_CORE_00361]
    enum class  future_status {
        ready,          //the shared state is ready
        timeout         //the shared state did not become ready before the specified timeout has passed     
    };

    enum class Progress {
        None,
        Timeout,
        Done,
        Retrieved,
    };

    using TimeoutCallback = std::function<void()>;

    template <typename T>
    struct State {
        static_assert(std::is_same<T, void>::value ||
            std::is_copy_constructible<T>() ||
            std::is_move_constructible<T>(),
            "must be copyable or movable or void");

        State() :
            progress_(Progress::None),
            retrieved_{ false } {
        }

        std::mutex thenLock_;

        using ValueType = typename TryWrapper<T>::Type;
        ValueType value_;
        std::function<void(ValueType&&)> then_;
        Progress progress_;

        std::function<void(TimeoutCallback&&)> onTimeout_;
        std::atomic<bool> retrieved_;
    };
    template <typename T2>
    Future<T2> MakeExceptionFuture(std::exception_ptr&&);
    //[SWS_CORE_00321]
    template <typename T>
    class Future{
    public:
        using InnerType = T;

        template <typename U>
        friend class Future;
        //[SWS_CORE_00322]
        Future() = default;
        //[SWS_CORE_00334]
        Future(const Future&) = delete;
        void operator= (const Future&) = delete;
        //[SWS_CORE_00323]
        Future(Future && fut) = default;
        Future& operator= (Future && fut) = default;

        explicit
            Future(std::shared_ptr<State<T>> state) :
            state_(std::move(state)) {
        }
        //[SWS_CORE_00333]
        ~Future() {
        }
        //[SWS_CORE_00327]
        bool valid() const {
            return state_ != nullptr;
        }

        // The blocking interface
        // PAY ATTENTION to deadlock: Wait thread must NOT be same as promise thread!!!
        //[SWS_CORE_00329]
        template <typename Rep, typename Period>
        future_status wait_for(std::chrono::duration< Rep, Period > const& timeoutDuration) {
            std::unique_lock<std::mutex> guard(state_->thenLock_);
            switch (state_->progress_) {
            case Progress::None:
                break;

            case Progress::Timeout:
                //throw std::runtime_error("Future timeout");
                GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::future_already_retrieved));

            case Progress::Done:
                //state_->progress_ = Progress::Retrieved;
                return future_status::ready;

            default:
                //throw std::runtime_error("Future already retrieved");
                GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::future_already_retrieved));
            }
            guard.unlock();

            auto cond(std::make_shared<std::condition_variable>());
            auto mutex(std::make_shared<std::mutex>());
            bool ready = false;
            typename State<T>::ValueType value;

            this->then([&value, &ready,
                wcond = std::weak_ptr<std::condition_variable>(cond),
                wmutex = std::weak_ptr<std::mutex>(mutex)](typename State<T>::ValueType&& v)
            {
                auto cond = wcond.lock();
                auto mutex = wmutex.lock();
                if (!cond || !mutex) return;

                std::unique_lock<std::mutex> guard(*mutex);
                value = std::move(v);
                ready = true;
                cond->notify_one();
            });

            std::unique_lock<std::mutex> waiter(*mutex);
            bool success = cond->wait_for(waiter, timeoutDuration, [&ready]() { return ready; });
            if (success)
                return future_status::ready;
            else
                return future_status::timeout;
        }

        //[SWS_CORE_00330]
        template <typename Clock, typename Duration>
        future_status wait_until(std::chrono::time_point< Clock, Duration > const& deadline){
            std::unique_lock<std::mutex> guard(state_->thenLock_);
            switch (state_->progress_) {
            case Progress::None:
                break;

            case Progress::Timeout:
                //throw std::runtime_error("Future timeout");
                GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::future_already_retrieved));

            case Progress::Done:
                //state_->progress_ = Progress::Retrieved;
                return future_status::ready;

            default:
                //throw std::runtime_error("Future already retrieved");
                GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::future_already_retrieved));
            }
            guard.unlock();

            auto cond(std::make_shared<std::condition_variable>());
            auto mutex(std::make_shared<std::mutex>());
            bool ready = false;
            typename State<T>::ValueType value;

            this->then([&value, &ready,
                wcond = std::weak_ptr<std::condition_variable>(cond),
                wmutex = std::weak_ptr<std::mutex>(mutex)](typename State<T>::ValueType&& v)
            {
                auto cond = wcond.lock();
                auto mutex = wmutex.lock();
                if (!cond || !mutex) return;

                std::unique_lock<std::mutex> guard(*mutex);
                value = std::move(v);
                ready = true;
                cond->notify_one();
            });

            std::unique_lock<std::mutex> waiter(*mutex);
            bool success = cond->wait_until(waiter, deadline, [&ready]() { return ready; });
            if (success)
                return future_status::ready;
            else
                return future_status::timeout;
        }

        //[SWS_CORE_00326]
        typename State<T>::ValueType get() {
            // block until ready then return the stored result or throw the stored exception
            std::unique_lock<std::mutex> guard(state_->thenLock_);
            switch (state_->progress_) {
            case Progress::None:
                break;

            case Progress::Timeout:
                //throw std::runtime_error("Future timeout");
                GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::future_already_retrieved));

            case Progress::Done:
                state_->progress_ = Progress::Retrieved;
                return std::move(state_->value_);

            default:
                //throw std::runtime_error("Future already retrieved");
                GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::future_already_retrieved));
            }
            guard.unlock();

            auto cond(std::make_shared<std::condition_variable>());
            auto mutex(std::make_shared<std::mutex>());
            bool ready = false;
            typename State<T>::ValueType value;

            this->then([&value, &ready,
                wcond = std::weak_ptr<std::condition_variable>(cond),
                wmutex = std::weak_ptr<std::mutex>(mutex)](typename State<T>::ValueType&& v)
            {
                auto cond = wcond.lock();
                auto mutex = wmutex.lock();
                if (!cond || !mutex) return;

                std::unique_lock<std::mutex> guard(*mutex);
                value = std::move(v);
                ready = true;
                cond->notify_one();
            });

            std::unique_lock<std::mutex> waiter(*mutex);
            while (!ready)
            {
                cond->wait(waiter);
            }
            state_->value_.Check();
            return std::move(state_->value_);
        }

        //[SWS_CORE_00336]
        //Result<T, E> GetResult() noexcept;

        // T is of type Future<InnerType>
        template <typename SHIT = T>
        typename std::enable_if<IsFuture<SHIT>::value, SHIT>::type
            Unwrap() {
            using InnerType = typename IsFuture<SHIT>::Inner;

            static_assert(std::is_same<SHIT, Future<InnerType>>::value, "Kidding me?");

            Promise<InnerType> prom;
            Future<InnerType> fut = prom.get_future();

            std::unique_lock<std::mutex> guard(state_->thenLock_);
            if (state_->progress_ == Progress::Timeout) {
                throw std::runtime_error("Wrong state : Timeout");
            }
            else if (state_->progress_ == Progress::Done) {
                try {
                    auto innerFuture = std::move(state_->value_);
                    return std::move(innerFuture.Value());
                }
                catch (const std::exception& e) {
                    return MakeExceptionFuture<InnerType>(std::current_exception());
                }
            }
            else {
                _SetCallback([pm = std::move(prom)](typename TryWrapper<SHIT>::Type&& innerFuture) mutable {
                    try {
                        SHIT future = std::move(innerFuture);
                        future._SetCallback([pm = std::move(pm)](typename TryWrapper<InnerType>::Type&& t) mutable {
                            // No need scheduler here, think about this code:
                            // `outer.Unwrap().then(sched, func);`
                            // outer.Unwrap() is the inner future, the below line
                            // will trigger func in sched thread.
                            pm.set_value(std::move(t));
                        });
                    }
                    catch (...) {
                        pm.SetException(std::current_exception());
                    }
                });
            }

            return fut;
        }

        //[SWS_CORE_00331]
        template <typename F,
            typename R = CallableResult<F, T> >
            auto then(F && f) -> typename R::ReturnFutureType {
            typedef typename R::Arg Arguments;
            return _ThenImpl<F, R>(nullptr, std::forward<F>(f), Arguments());
        }

        // f will be called in sched
        template <typename F,
            typename R = CallableResult<F, T> >
            auto then(Scheduler * sched, F && f) -> typename R::ReturnFutureType {
            typedef typename R::Arg Arguments;
            return _ThenImpl<F, R>(sched, std::forward<F>(f), Arguments());
        }

        //1. F does not return future type
        template <typename F, typename R, typename... Args>
        typename std::enable_if<!R::IsReturnsFuture::value, typename R::ReturnFutureType>::type
            _ThenImpl(Scheduler * sched, F && f, ResultOfWrapper<F, Args...>) {
            static_assert(sizeof...(Args) <= 1, "Then must take zero/one argument");

            using FReturnType = typename R::IsReturnsFuture::Inner;

            Promise<FReturnType> pm;
            auto nextFuture = pm.get_future();

            using FuncType = typename std::decay<F>::type;

            std::unique_lock<std::mutex> guard(state_->thenLock_);
            if (state_->progress_ == Progress::Timeout) {
                throw std::runtime_error("Wrong state : Timeout");
            }
            else if (state_->progress_ == Progress::Done) {
                typename TryWrapper<T>::Type t;
                try {
                    t = std::move(state_->value_);
                }
                catch (const std::exception& e) {
                    t = (typename TryWrapper<T>::Type)(std::current_exception());
                }

                guard.unlock();

                if (sched) {
                    sched->Schedule([t = std::move(t),
                        f = std::forward<FuncType>(f),
                        pm = std::move(pm)]() mutable {
                        auto result = WrapWithTry(f, std::move(t));
                        pm.set_value(std::move(result));
                    });
                }
                else {
                    auto result = WrapWithTry(f, std::move(t));
                    pm.set_value(std::move(result));
                }
            }
            else {
                // set this future's then callback
                _SetCallback([sched,
                    func = std::forward<FuncType>(f),
                    prom = std::move(pm)](typename TryWrapper<T>::Type&& t) mutable {
                    //if (prom.IsReady())
                    //{
                    //    return;
                    //}
                    if (sched) {
                        sched->Schedule([func = std::move(func),
                            t = std::move(t),
                            prom = std::move(prom)]() mutable {
                            // run callback, T can be void, thanks to folly
                            auto result = WrapWithTry(func, std::move(t));
                            // set next future's result
                            prom.set_value(std::move(result));
                        });
                    }
                    else {
                        // run callback, T can be void, thanks to folly Try<>
                        auto result = WrapWithTry(func, std::move(t));
                        // set next future's result
                        prom.set_value(std::move(result));
                    }
                });
            }

            return std::move(nextFuture);
        }

        //2. F return another future type
        template <typename F, typename R, typename... Args>
        typename std::enable_if<R::IsReturnsFuture::value, typename R::ReturnFutureType>::type
            _ThenImpl(Scheduler * sched, F && f, ResultOfWrapper<F, Args...>) {
            static_assert(sizeof...(Args) <= 1, "Then must take zero/one argument");

            using FReturnType = typename R::IsReturnsFuture::Inner;

            Promise<FReturnType> pm;
            auto nextFuture = pm.get_future();

            using FuncType = typename std::decay<F>::type;

            std::unique_lock<std::mutex> guard(state_->thenLock_);
            if (state_->progress_ == Progress::Timeout) {
                throw std::runtime_error("Wrong state : Timeout");
            }
            else if (state_->progress_ == Progress::Done) {
                typename TryWrapper<T>::Type t;
                try {
                    t = std::move(state_->value_);
                }
                catch (const std::exception& e) {
                    t = decltype(t)(std::current_exception());
                }

                guard.unlock();

                auto cb = [res = std::move(t),
                    f = std::forward<FuncType>(f),
                    prom = std::move(pm)]() mutable {
                    // because func return another future: innerFuture, when innerFuture is done, nextFuture can be done
                    decltype(f(res.template Get<Args>()...)) innerFuture;
                    if (res.HasException()) {
                        // Failed if Args... is void
                        innerFuture = f(typename TryWrapper<typename std::decay<Args...>::type>::Type(res.Exception()));
                    }
                    else {
                        innerFuture = f(res.template Get<Args>()...);
                    }

                    if (!innerFuture.valid()) {
                        return;
                    }

                    std::unique_lock<std::mutex> guard(innerFuture.state_->thenLock_);
                    if (innerFuture.state_->progress_ == Progress::Timeout) {
                        throw std::runtime_error("Wrong state : Timeout");
                    }
                    else if (innerFuture.state_->progress_ == Progress::Done) {
                        typename TryWrapper<FReturnType>::Type t;
                        try {
                            t = std::move(innerFuture.state_->value_);
                        }
                        catch (const std::exception& e) {
                            t = decltype(t)(std::current_exception());
                        }

                        guard.unlock();
                        prom.set_value(std::move(t));
                    }
                    else {
                        innerFuture._SetCallback([prom = std::move(prom)](typename TryWrapper<FReturnType>::Type&& t) mutable {
                            prom.set_value(std::move(t));
                        });
                    }
                };

                if (sched)
                    sched->Schedule(std::move(cb));
                else
                    cb();
            }
            else {
                // set this future's then callback
                _SetCallback([sched = sched,
                    func = std::forward<FuncType>(f),
                    prom = std::move(pm)](typename TryWrapper<T>::Type&& t) mutable {

                    auto cb = [func = std::move(func), t = std::move(t), prom = std::move(prom)]() mutable {
                        // because func return another future: innerFuture, when innerFuture is done, nextFuture can be done
                        decltype(func(t.template Get<Args>()...)) innerFuture;
                        if (t.HasException()) {
                            // Failed if Args... is void
                            innerFuture = func(typename TryWrapper<typename std::decay<Args...>::type>::Type(t.Exception()));
                        }
                        else {
                            innerFuture = func(t.template Get<Args>()...);
                        }

                        if (!innerFuture.valid()) {
                            return;
                        }
                        std::unique_lock<std::mutex> guard(innerFuture.state_->thenLock_);
                        if (innerFuture.state_->progress_ == Progress::Timeout) {
                            throw std::runtime_error("Wrong state : Timeout");
                        }
                        else if (innerFuture.state_->progress_ == Progress::Done) {
                            typename TryWrapper<FReturnType>::Type t;
                            try {
                                t = std::move(innerFuture.state_->value_);
                            }
                            catch (const std::exception& e) {
                                t = decltype(t)(std::current_exception());
                            }

                            guard.unlock();
                            prom.set_value(std::move(t));
                        }
                        else {
                            innerFuture._SetCallback([prom = std::move(prom)](typename TryWrapper<FReturnType>::Type&& t) mutable {
                                prom.set_value(std::move(t));
                            });
                        }
                    };

                    if (sched)
                        sched->Schedule(std::move(cb));
                    else
                        cb();
                });
            }

            return std::move(nextFuture);
        }

        /*
         * When register callbacks and timeout for a future like this:
         *      Future<int> f;
         *      f.Then(xx).OnTimeout(yy);
         *
         * There will be one future object created except f, we call f as root future.
         * The yy callback is registed on the last future, here are the possiblities:
         * 1. xx is called, and yy is not called.
         * 2. xx is not called, and yy is called.
         *
         * BUT BE CAREFUL BELOW:
         *
         *      Future<int> f;
         *      f.Then(xx).Then(yy).OnTimeout(zz);
         *
         * There will be 3 future objects created except f, we call f as root future.
         * The zz callback is registed on the last future, here are the possiblities:
         * 1. xx is called, and zz is called, yy is not called.
         * 2. xx and yy are called, and zz is called, aha, it's rarely happend but...
         * 3. xx and yy are called, it's the normal case.
         * So, you may shouldn't use OnTimeout with chained futures!!!
         */
        void OnTimeout(std::chrono::milliseconds duration,
            TimeoutCallback f,
            Scheduler * scheduler) {

            scheduler->ScheduleLater(duration, [state = state_, cb = std::move(f)]() mutable {
                std::unique_lock<std::mutex> guard(state->thenLock_);
                if (state->progress_ != Progress::None)
                    return;

                state->progress_ = Progress::Timeout;
                guard.unlock();

                cb();
            });
        }

        //[SWS_CORE_00332]
        bool is_ready() const {
            return state_->progress_ != Progress::None;
        }

    private:
        void _SetCallback(std::function<void(typename TryWrapper<T>::Type&&)> && func) {
            state_->then_ = std::move(func);
        }

        void _SetOnTimeout(std::function<void(TimeoutCallback&&)> && func) {
            state_->onTimeout_ = std::move(func);
        }


        std::shared_ptr<State<T>> state_;
    };

    // Make ready future
    template <typename T2>
    inline Future<typename std::decay<T2>::type> MakeReadyFuture(T2&& value) {
        Promise<typename std::decay<T2>::type> pm;
        auto f(pm.get_future());
        pm.set_value(std::forward<T2>(value));

        return f;
    }

    //inline Future<void> MakeReadyFuture() {
    //    Promise<void> pm;
    //    auto f(pm.get_future());
    //    pm.set_value();

    //    return f;
    //}

    // Make exception future
    template <typename T2, typename E>
    inline Future<T2> MakeExceptionFuture(E&& exp) {
        Promise<T2> pm;
        pm.SetException(std::make_exception_ptr(std::forward<E>(exp)));

        return pm.get_future();
    }

    template <typename T2>
    inline Future<T2> MakeExceptionFuture(std::exception_ptr&& eptr) {
        Promise<T2> pm;
        pm.SetException(std::move(eptr));

        return pm.get_future();
    }

    // When All
    template <typename... FT>
    typename CollectAllVariadicContext<typename std::decay<FT>::type::InnerType...>::FutureType
        WhenAll(FT&&... futures) {
        auto ctx = std::make_shared<CollectAllVariadicContext<typename std::decay<FT>::type::InnerType...>>();

        CollectVariadicHelper<CollectAllVariadicContext>(
            ctx, std::forward<typename std::decay<FT>::type>(futures)...);

        return ctx->pm.get_future();
    }


    template <class InputIterator>
    Future<
        std::vector<typename TryWrapper<typename std::iterator_traits<InputIterator>::value_type::InnerType>::Type>
    >
        WhenAll(InputIterator first, InputIterator last) {
        using TryT = typename TryWrapper<typename std::iterator_traits<InputIterator>::value_type::InnerType>::Type;
        if (first == last)
            return MakeReadyFuture(std::vector<TryT>());

        struct AllContext {
            AllContext(int n) : results(n) {}
            ~AllContext() {
                // I think this line is useless.
                // pm.set_value(std::move(results));
            }

            Promise<std::vector<TryT>> pm;
            std::vector<TryT> results;
            std::atomic<size_t> collected{ 0 };
        };

        auto ctx = std::make_shared<AllContext>(std::distance(first, last));

        for (size_t i = 0; first != last; ++first, ++i) {
            first->then([ctx, i](TryT&& t) {
                ctx->results[i] = std::move(t);
                if (ctx->results.size() - 1 ==
                    std::atomic_fetch_add(&ctx->collected, std::size_t(1))) {
                    ctx->pm.set_value(std::move(ctx->results));
                }
                });
        }

        return ctx->pm.get_future();
    }

    // When Any
    template <class InputIterator>
    Future<
        std::pair<size_t, typename TryWrapper<typename std::iterator_traits<InputIterator>::value_type::InnerType>::Type>
    >
        WhenAny(InputIterator first, InputIterator last) {
        using T = typename std::iterator_traits<InputIterator>::value_type::InnerType;
        using TryT = typename TryWrapper<T>::Type;

        if (first == last) {
            return MakeReadyFuture(std::make_pair(size_t(0), TryT()));
        }

        struct AnyContext {
            AnyContext() {};
            Promise<std::pair<size_t, TryT>> pm;
            std::atomic<bool> done{ false };
        };

        auto ctx = std::make_shared<AnyContext>();
        for (size_t i = 0; first != last; ++first, ++i) {
            first->then([ctx, i](TryT&& t) {
                if (!ctx->done.exchange(true)) {
                    ctx->pm.set_value(std::make_pair(i, std::move(t)));
                }
                });
        }

        return ctx->pm.get_future();
    }


    // When N
    template <class InputIterator>
    Future<
        std::vector<std::pair<size_t, typename TryWrapper<typename std::iterator_traits<InputIterator>::value_type::InnerType>::Type>>
    >
        WhenN(size_t N, InputIterator first, InputIterator last) {
        using T = typename std::iterator_traits<InputIterator>::value_type::InnerType;
        using TryT = typename TryWrapper<T>::Type;

        size_t nFutures = std::distance(first, last);
        const size_t needCollect = std::min<size_t>(nFutures, N);

        if (needCollect == 0) {
            return MakeReadyFuture(std::vector<std::pair<size_t, TryT>>());
        }

        struct NContext {
            NContext(size_t _needs) : needs(_needs) {}
            Promise<std::vector<std::pair<size_t, TryT>>> pm;

            std::mutex mutex;
            std::vector<std::pair<size_t, TryT>> results;
            const size_t needs;
            bool done{ false };
        };

        auto ctx = std::make_shared<NContext>(needCollect);
        for (size_t i = 0; first != last; ++first, ++i) {
            first->then([ctx, i](TryT&& t) {
                std::unique_lock<std::mutex> guard(ctx->mutex);
                if (ctx->done)
                    return;

                ctx->results.push_back(std::make_pair(i, std::move(t)));
                if (ctx->needs == ctx->results.size()) {
                    ctx->done = true;
                    guard.unlock();
                    ctx->pm.set_value(std::move(ctx->results));
                }
                });
        }

        return ctx->pm.get_future();
    }

    // When If Any
    template <class InputIterator>
    Future<
        std::pair<size_t, typename TryWrapper<typename std::iterator_traits<InputIterator>::value_type::InnerType>::Type>
    >
        WhenIfAny(InputIterator first, InputIterator last,
            std::function<bool(const Try<typename std::iterator_traits<InputIterator>::value_type::InnerType>&)> cond) {

        using T = typename std::iterator_traits<InputIterator>::value_type::InnerType;
        using TryT = typename TryWrapper<T>::Type;

        if (first == last) {
            return MakeReadyFuture(std::make_pair(size_t(0), TryT()));
        }

        const size_t nFutures = std::distance(first, last);

        struct IfAnyContext {
            IfAnyContext() {};
            Promise<std::pair<size_t, TryT>> pm;
            std::atomic<size_t> returned{ 0 };  // including fail response, eg, cond(rsp) == false
            std::atomic<bool> done{ false };
        };

        auto ctx = std::make_shared<IfAnyContext>();
        for (size_t i = 0; first != last; ++first, ++i) {
            first->then([ctx, i, nFutures, cond](TryT&& t) {
                if (ctx->done) {
                    ctx->returned.fetch_add(1);
                    return;
                }
                if (!cond(t)) {
                    const size_t returned = ctx->returned.fetch_add(1) + 1;
                    if (returned == nFutures) {
                        // If some success future done, below if statement will be false
                        if (!ctx->done.exchange(true)) {
                            // FAILED...
                            try {
                                throw std::runtime_error("WhenIfAny Failed, no true condition.");
                            }
                            catch (...) {
                                ctx->pm.SetException(std::current_exception());
                            }
                        }
                    }

                    return;
                }
                if (!ctx->done.exchange(true)) {
                    ctx->pm.set_value(std::make_pair(i, std::move(t)));
                }
                ctx->returned.fetch_add(1);
                });
        }

        return ctx->pm.get_future();
    }


    // When if N
    template <class InputIterator>
    Future<
        std::vector<std::pair<size_t, typename TryWrapper<typename std::iterator_traits<InputIterator>::value_type::InnerType>::Type>>
    >
        WhenIfN(size_t N, InputIterator first, InputIterator last,
            std::function<bool(const Try<typename std::iterator_traits<InputIterator>::value_type::InnerType>&)> cond) {

        using T = typename std::iterator_traits<InputIterator>::value_type::InnerType;
        using TryT = typename TryWrapper<T>::Type;

        size_t nFutures = std::distance(first, last);
        const size_t needCollect = std::min<size_t>(nFutures, N);

        if (needCollect == 0) {
            return MakeReadyFuture(std::vector<std::pair<size_t, TryT>>());
        }

        struct IfNContext {
            IfNContext(size_t _needs) : needs(_needs) {}
            Promise<std::vector<std::pair<size_t, TryT>>> pm;

            std::mutex mutex;
            std::vector<std::pair<size_t, TryT>> results;
            size_t returned{ 0 }; // including fail response, eg, cond(rsp) == false
            const size_t needs;
            bool done{ false };
        };

        auto ctx = std::make_shared<IfNContext>(needCollect);
        for (size_t i = 0; first != last; ++first, ++i) {
            first->then([ctx, i, nFutures, cond](TryT&& t) {
                std::unique_lock<std::mutex> guard(ctx->mutex);
                ++ctx->returned;
                if (ctx->done)
                    return;

                if (!cond(t)) {
                    if (ctx->returned == nFutures) {
                        // Failed: all returned, but not enough true cond(t)!
                        // Should I return partial result ???
                        try {
                            throw std::runtime_error("WhenIfN Failed, not enough true condition.");
                        }
                        catch (...) {
                            ctx->done = true;
                            guard.unlock();
                            ctx->pm.SetException(std::current_exception());
                        }
                    }

                    return;
                }

                ctx->results.push_back(std::make_pair(i, std::move(t)));
                if (ctx->needs == ctx->results.size()) {
                    ctx->done = true;
                    guard.unlock();
                    ctx->pm.set_value(std::move(ctx->results));
                }
                });
        }

        return ctx->pm.get_future();
    }
}
}
#endif