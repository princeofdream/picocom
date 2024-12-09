#ifndef BERT_FUTURE_H
#define BERT_FUTURE_H

#include <atomic>
#include <mutex>
#include <functional>
#include <type_traits>
#include <condition_variable>

#include "error_code.h"
#include "future_error_domain.h"
#include "result.h"

#include "Helper.h"
#include "Try.h"
namespace ara
{
namespace core{

    class Scheduler
    {
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
         * ft.Then(&this_loop, [](const Buffer& file_contents) {
         *      // SUCCESS : process file_content;
         * })
         * .OnTimeout(std::chrono::seconds(10), [=very_big_file]() {
         *      // FAILED OR TIMEOUT:
         *      printf("Read file %s failed\n", very_big_file);
         * },
         * &this_loop);
         */
        virtual void ScheduleAfter(std::chrono::milliseconds duration, std::function<void()> f) = 0;
        virtual void Schedule(std::function<void()> f) = 0;
    };

    //[SWS_CORE_00361]
    enum class  future_status {
        ready,          //the shared state is ready
        timeout         //the shared state did not become ready before the specified timeout has passed     
    };

namespace internal
{

enum class Progress
{
    None,
    Timeout,
    Done
};

using TimeoutCallback = std::function<void ()>;

template <typename T>
struct State
{
    static_assert(std::is_same<T, void>::value ||
                  std::is_copy_constructible<T>() ||
                  std::is_move_constructible<T>(),
                  "must be copyable or movable or void");

    State() :
        progress_(Progress::None),
        retrieved_ {false}
    {
    }

    std::mutex thenLock_;

    using ValueType = typename TryWrapper<T>::Type;
    ValueType value_;
    std::function<void (ValueType&& )> then_;
    Progress progress_;

    std::function<void (TimeoutCallback&& )> onTimeout_;
    std::atomic<bool> retrieved_;

    bool IsRoot() const { return !onTimeout_; }

    //for wait functions
    void DoNotify() {
        std::unique_lock<std::mutex> waiter(mtx_);
        cond_.notify_all();
    }

    std::mutex mtx_;
    std::condition_variable cond_;
};

} // end namespace internal


template <typename T, typename E>
class Future;

template <typename T2>
Future<T2> MakeExceptionFuture(std::exception_ptr&& );

/*
* [SWS_CORE_00321]
* Provides ara::core specific Future operations to collect the results of an asynchronous call.
*/
template <typename T, typename E>
class Future final
{
public: 
    using InnerType = T;

    template <typename U, typename Er>
    friend class Future;
    /*
    * [SWS_CORE_00322]
    * Default constructor. 
    * This function shall behave the same as the corresponding std::future function.
    */
    Future() noexcept = default;

    /*
    * [SWS_CORE_00334]
    * Copy constructor shall be disabled.
    */
    Future(const Future&) = delete;/*
    * [SWS_CORE_00335]
    * Copy assignment operator shall be disabled.
    */
    void operator= (const Future&) = delete;

    /*
    * [SWS_CORE_00323]
    * Move construct from another instance.
    * This function shall behave the same as the corresponding std::future function.
    */
    Future(Future&& fut) = default;
    /*
    * [SWS_CORE_00325]
    * Move assign from another instance.
    * This function shall behave the same as the corresponding std::future function.
    */
    Future& operator= (Future&& fut) = default;

    /*
    * [SWS_CORE_00333]
    * Destructor for Future objects.
    * This function shall behave the same as the corresponding std::future function.
    */
    ~Future() {}

    explicit
    Future(std::shared_ptr<internal::State<T>> state) :
        state_(std::move(state))
    {
    }

    /*
    * [SWS_CORE_00332]
    * Return whether the asynchronous operation has finished.
    * If this function returns true, get(), GetResult() and the wait calls are guaranteed not to block.
    * The behavior of this function is undefined if valid() returns false.
    */
    bool is_ready() const {
        return state_->progress_ != internal::Progress::None;
    }

    /*
    * [SWS_CORE_00328]
    * Wait for a value or an error to be available.
    * This function shall behave the same as the corresponding std::future function.
    */
    void wait() const {
        std::unique_lock<std::mutex> guard(state_->thenLock_);
        if (state_->progress_ != internal::Progress::None)
        {
            return;
        }
        guard.unlock();
        std::unique_lock<std::mutex> waiter(state_->mtx_);
        while (!is_ready())
        {
            state_->cond_.wait(waiter);
        }
    }

    /*
    * [SWS_CORE_00329]
    * Wait for the given period, or until a value or an error is available.
    * This function shall behave the same as the corresponding std::future function.
    */
    template <typename Rep, typename Period>
    future_status wait_for(std::chrono::duration< Rep, Period > const& timeoutDuration) const{
        std::unique_lock<std::mutex> guard(state_->thenLock_);
        if (state_->progress_ != internal::Progress::None)
        {
            return future_status::ready;
        }
        guard.unlock();

        std::unique_lock<std::mutex> waiter(state_->mtx_);
        bool success = state_->cond_.wait_for(waiter, timeoutDuration, [weak_parent = std::weak_ptr<internal::State<T>>(state_)]() {
            auto parent = weak_parent.lock();
            return parent->progress_ == internal::Progress::Done;
        });
        return success ? future_status::ready : future_status::timeout;
    }

    /*
    * [SWS_CORE_00330]
    * Wait until the given time, or until a value or an error is available.
    * This function shall behave the same as the corresponding std::future function.
    */
    template <typename Clock, typename Duration>
    future_status wait_until(std::chrono::time_point< Clock, Duration > const& deadline)  const {
        std::unique_lock<std::mutex> guard(state_->thenLock_);
        if (state_->progress_ != internal::Progress::None)
        {
            return future_status::ready;
        }
        guard.unlock();

        std::unique_lock<std::mutex> waiter(state_->mtx_);
        bool success = state_->cond_.wait_until(waiter, deadline, [weak_parent = std::weak_ptr<internal::State<T>>(state_)]() {
            auto parent = weak_parent.lock();
            return parent->progress_ == internal::Progress::Done; 
        });
        return success ? future_status::ready : future_status::timeout;
    }

    /*
    * [SWS_CORE_00326]
    * Get the value.
    * This function shall behave the same as the corresponding std::future function.
    * This function does not participate in overload resolution when the compiler toolchain does not
    * support C++ exceptions.
    * 
    * Errors:the error that has been put into the corresponding
    *       Promise via Promise::SetError
    */
    typename internal::State<T>::ValueType get() {
        // block until ready then return the stored result or throw the stored exception
        std::unique_lock<std::mutex> guard(state_->thenLock_);
        if (state_->progress_ != internal::Progress::None)
        {
            if (state_->value_.HasValue())
            {
                return std::move(state_->value_);
            }
            else if (state_->value_.HasException())
            {
                std::rethrow_exception(state_->value_.Exception());
            }
            else
            {
                //none
                GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::future_already_retrieved));
            }
        }
        guard.unlock();
        std::unique_lock<std::mutex> waiter(state_->mtx_);
        while (!is_ready())
        {
            state_->cond_.wait(waiter);
        }
        guard.lock();
        if (state_->value_.HasValue())
        {
            return std::move(state_->value_);
        }
        else if (state_->value_.HasException())
        {
            std::rethrow_exception(state_->value_.Exception());
        }
        else
        {
            //none
            GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::future_already_retrieved));
        }
    }

    /*
    * [SWS_CORE_00336]
    * Get the result.
    * Similar to get(), this call blocks until the value or an error is available. However, this call will
    * never throw an exception.
    * 
    * Errors:the error that has been put into the corresponding
    *       Promise via Promise::SetError
    */
    Result<T, E> GetResult() noexcept;

    /*
    * [SWS_CORE_00327]
    * Checks if the Future is valid, i.e. if it has a shared state.
    * This function shall behave the same as the corresponding std::future function.
    */
    bool valid() const noexcept {
        return state_ != nullptr;
    }

    // T is of type Future<InnerType>
    template <typename ValueType = T>
    typename std::enable_if<internal::IsFuture<ValueType>::value, ValueType>::type
    Unwrap()
    {
        using namespace internal;
        using InnerType = typename IsFuture<ValueType>::Inner;

        static_assert(std::is_same<ValueType, Future<InnerType>>::value, "Kidding me?");

        Promise<InnerType> prom;
        Future<InnerType> fut = prom.GetFuture();

        std::unique_lock<std::mutex> guard(state_->thenLock_);
        if (state_->progress_ == Progress::Timeout)
        {
            struct FutureWrongState {};
            throw FutureWrongState();
        }
        else if (state_->progress_ == Progress::Done)
        {
            try {
                auto innerFuture = std::move(state_->value_);
                return std::move(innerFuture.Value());
            }
            catch(const std::exception& e) {
                return MakeExceptionFuture<InnerType>(std::current_exception());
            }
        }
        else
        {
            SetCallback([pm = std::move(prom)](typename TryWrapper<ValueType>::Type&& innerFuture) mutable {
                    try {
                        ValueType future = std::move(innerFuture);
                        future.SetCallback([pm = std::move(pm)](typename TryWrapper<InnerType>::Type&& t) mutable {
                            // No need scheduler here, think about this code:
                            // `outer.Unwrap().Then(sched, func);`
                            // outer.Unwrap() is the inner future, the below line
                            // will trigger func in sched thread.
                            pm.SetValue(std::move(t));
                        });
                    }
                    catch (...) {
                        pm.SetException(std::current_exception());
                    }
            });
        }

        return fut;
    }

    template <typename F,
              typename R = internal::CallableResult<F, T> > 
    auto Then(F&& f) -> typename R::ReturnFutureType 
    {
        typedef typename R::Arg Arguments;
        return _ThenImpl<F, R>(nullptr, std::forward<F>(f), Arguments());  
    }

    // f will be called in sched
    template <typename F,
              typename R = internal::CallableResult<F, T> > 
    auto Then(Scheduler* sched, F&& f) -> typename R::ReturnFutureType 
    {
        typedef typename R::Arg Arguments;
        return _ThenImpl<F, R>(sched, std::forward<F>(f), Arguments());  
    }

    //1. F does not return future type
    template <typename F, typename R, typename... Args>
    typename std::enable_if<!R::IsReturnsFuture::value, typename R::ReturnFutureType>::type
    _ThenImpl(Scheduler* sched, F&& f, internal::ResultOfWrapper<F, Args...> )
    {
        static_assert(std::is_void<T>::value ? sizeof...(Args) == 0 : sizeof...(Args) == 1,
                      "Then callback must take 0/1 argument");

        using FReturnType = typename R::IsReturnsFuture::Inner;
        using namespace internal;

        Promise<FReturnType> pm;
        auto nextFuture = pm.GetFuture();

        // FIXME
        using FuncType = typename std::decay<F>::type;

        std::unique_lock<std::mutex> guard(state_->thenLock_);
        if (state_->progress_ == Progress::Timeout)
        {
            struct FutureWrongState {};
            throw FutureWrongState();
        }
        else if (state_->progress_ == Progress::Done)
        {
            typename TryWrapper<T>::Type t;
            try {
                t = std::move(state_->value_);
            }
            catch(const std::exception&) {
                t = (typename TryWrapper<T>::Type)(std::current_exception());
            }

            guard.unlock();

            auto func = [res = std::move(t),
                         f = std::forward<FuncType>(f),
                         prom = std::move(pm)]() mutable {
                auto result = WrapWithTry(f, std::move(res));
                prom.SetValue(std::move(result));
            };

            if (sched)
                sched->Schedule(std::move(func));
            else
                func();
        }
        else
        {
            // 1. set pm's timeout callback
            nextFuture.SetOnTimeout([weak_parent = std::weak_ptr<State<T>>(state_)](TimeoutCallback&& cb) {
                    auto parent = weak_parent.lock();
                    if (!parent)
                        return;

                    {
                        std::unique_lock<std::mutex> guard(parent->thenLock_);
                        // if parent future is Done, let it go down
                        if (parent->progress_ != Progress::None)
                            return;
                    
                        parent->progress_ = Progress::Timeout;
                    }

                    if (!parent->IsRoot())
                        parent->onTimeout_(std::move(cb)); // propogate to the root
                    else
                        cb();
                });

            // 2. set this future's then callback
            SetCallback([sched,
                         func = std::forward<FuncType>(f),
                         prom = std::move(pm)](typename TryWrapper<T>::Type&& t) mutable {

                auto cb = [func = std::move(func), t = std::move(t), prom = std::move(prom)]() mutable {
                    // run callback, T can be void, thanks to folly Try<>
                    auto result = WrapWithTry(func, std::move(t));
                    // set next future's result
                    prom.SetValue(std::move(result));
                };

                if (sched)
                    sched->Schedule(std::move(cb));
                else
                    cb();
            });
        }

        return std::move(nextFuture);
    }

    //2. F return another future type
    template <typename F, typename R, typename... Args>
    typename std::enable_if<R::IsReturnsFuture::value, typename R::ReturnFutureType>::type
    _ThenImpl(Scheduler* sched, F&& f, internal::ResultOfWrapper<F, Args...>)
    {
        static_assert(sizeof...(Args) <= 1, "Then must take zero/one argument");

        using FReturnType = typename R::IsReturnsFuture::Inner;
        using namespace internal;

        Promise<FReturnType> pm;
        auto nextFuture = pm.GetFuture();

        // FIXME
        using FuncType = typename std::decay<F>::type;

        std::unique_lock<std::mutex> guard(state_->thenLock_);
        if (state_->progress_ == Progress::Timeout)
        {
            struct FutureWrongState {};
            throw FutureWrongState();
        }
        else if (state_->progress_ == Progress::Done)
        {
            typename TryWrapper<T>::Type t;
            try {
                t = std::move(state_->value_);
            }
            catch(const std::exception& e) {
                t = decltype(t)(std::current_exception());
            }

            guard.unlock();

            auto cb = [res = std::move(t),
                       f = std::forward<FuncType>(f),
                       prom = std::move(pm)]() mutable {
                // because func return another future: innerFuture, when innerFuture is done, nextFuture can be done
                decltype(f(res.template Get<Args>()...)) innerFuture;
                if (res.HasException()) {
                    // FIXME if Args... is void
                    innerFuture = f(typename TryWrapper<typename std::decay<Args...>::type>::Type(res.Exception()));
                }
                else {
                    innerFuture = f(res.template Get<Args>()...);
                }

                std::unique_lock<std::mutex> guard(innerFuture.state_->thenLock_);
                if (innerFuture.state_->progress_ == Progress::Timeout) {
                    struct FutureWrongState {};
                    throw FutureWrongState();
                }
                else if (innerFuture.state_->progress_ == Progress::Done) {
                    typename TryWrapper<FReturnType>::Type t;
                    try {
                        t = std::move(innerFuture.state_->value_);
                    }
                    catch(const std::exception& e) {
                        t = decltype(t)(std::current_exception());
                    }

                    guard.unlock();
                    prom.SetValue(std::move(t));
                }
                else {
                    innerFuture.SetCallback([prom = std::move(prom)](typename TryWrapper<FReturnType>::Type&& t) mutable {
                        prom.SetValue(std::move(t));
                    });
                }
            };

            if (sched)
                sched->Schedule(std::move(cb));
            else
                cb();
        }
        else
        {
            // 1. set pm's timeout callback
            nextFuture.SetOnTimeout([weak_parent = std::weak_ptr<State<T>>(state_)](TimeoutCallback&& cb) {
                    auto parent = weak_parent.lock();
                    if (!parent)
                        return;

                    {
                        std::unique_lock<std::mutex> guard(parent->thenLock_);
                        if (parent->progress_ != Progress::None)
                            return;
                    
                        parent->progress_ = Progress::Timeout;
                    }

                    if (!parent->IsRoot())
                        parent->onTimeout_(std::move(cb)); // propogate to the root
                    else
                        cb();

                });

            // 2. set this future's then callback
            SetCallback([sched = sched,
                         func = std::forward<FuncType>(f),
                         prom = std::move(pm)](typename TryWrapper<T>::Type&& t) mutable {
                auto cb = [func = std::move(func), t = std::move(t), prom = std::move(prom)]() mutable {
                    // because func return another future: innerFuture, when innerFuture is done, nextFuture can be done
                    decltype(func(t.template Get<Args>()...)) innerFuture;
                    if (t.HasException()) {
                        // FIXME if Args... is void
                        innerFuture = func(typename TryWrapper<typename std::decay<Args...>::type>::Type(t.Exception()));
                    }
                    else {
                        innerFuture = func(t.template Get<Args>()...);
                    }

                    std::unique_lock<std::mutex> guard(innerFuture.state_->thenLock_);
                    if (innerFuture.state_->progress_ == Progress::Timeout) {
                        struct FutureWrongState {};
                        throw FutureWrongState();
                    }
                    else if (innerFuture.state_->progress_ == Progress::Done) {
                        typename TryWrapper<FReturnType>::Type t;
                        try {
                            t = std::move(innerFuture.state_->value_);
                        }
                        catch(const std::exception& e) {
                            t = decltype(t)(std::current_exception());
                        }

                        guard.unlock();
                        prom.SetValue(std::move(t));
                    }
                    else {
                        innerFuture.SetCallback([prom = std::move(prom)](typename TryWrapper<FReturnType>::Type&& t) mutable {
                            prom.SetValue(std::move(t));
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

    void SetCallback(std::function<void (typename TryWrapper<T>::Type&& )>&& func)
    {
        state_->then_ = std::move(func);
    }

    void SetOnTimeout(std::function<void (internal::TimeoutCallback&& )>&& func)
    {
        state_->onTimeout_ = std::move(func);
    }

    /*
     * When register callbacks and timeout for a future like this:
     *
     *      Future<int> f;
     *      f.Then(xx).Then(yy).OnTimeout(zz);
     *
     * There will be 3 future objects created except f, we call f as root future.
     * The zz callback is registed on the last future, however, timeout and future satisfication
     * can happened almost in the same time, we should ensure that both xx and yy will be called
     * or zz will be called, but they can't happened both or neither. So we pass the cb
     * to the root future, if we find out that root future is indeed timeout, we call cb there.
     */
    void OnTimeout(std::chrono::milliseconds duration,
                   internal::TimeoutCallback f,
                   Scheduler* scheduler)
    {
        scheduler->ScheduleAfter(duration, [state = state_, cb = std::move(f)]() mutable {
                {
                    std::unique_lock<std::mutex> guard(state->thenLock_);

                    if (state->progress_ != internal::Progress::None)
                        return;

                    state->progress_ = internal::Progress::Timeout;
                }

                if (!state->IsRoot())
                    state->onTimeout_(std::move(cb)); // propogate to the root future
                else
                    cb();
        });
    }

private:
    std::shared_ptr<internal::State<T>> state_;
};

// Make ready future
template <typename T2>
inline Future<typename std::decay<T2>::type> MakeReadyFuture(T2&& value)
{
    Promise<typename std::decay<T2>::type> pm;
    auto f(pm.GetFuture());
    pm.SetValue(std::forward<T2>(value));

    return f;
}

// Make exception future
template <typename T2, typename E>
inline Future<T2> MakeExceptionFuture(E&& exp)
{
    Promise<T2> pm;
    pm.SetException(std::make_exception_ptr(std::forward<E>(exp)));

    return pm.GetFuture();
}

template <typename T2>
inline Future<T2> MakeExceptionFuture(std::exception_ptr&& eptr)
{
    Promise<T2> pm;
    pm.SetException(std::move(eptr));

    return pm.GetFuture();
}


// When All
template <typename... FT>
typename internal::CollectAllVariadicContext<typename std::decay<FT>::type::InnerType...>::FutureType
WhenAll(FT&&... futures)
{
    auto ctx = std::make_shared<internal::CollectAllVariadicContext<typename std::decay<FT>::type::InnerType...>>();

    internal::CollectVariadicHelper<internal::CollectAllVariadicContext>( 
            ctx, std::forward<typename std::decay<FT>::type>(futures)...);

    return ctx->pm.GetFuture();
}


template <class InputIterator>
Future<
    ara::core::Vector<
    Try<typename std::iterator_traits<InputIterator>::value_type::InnerType>>>
    WhenAll(InputIterator first, InputIterator last)
{
    using T = typename std::iterator_traits<InputIterator>::value_type::InnerType;
    if (first == last)
        return MakeReadyFuture(ara::core::Vector<Try<T>>());

    struct CollectAllContext
    {
        CollectAllContext(int n) : results(n) {}
        ~CollectAllContext()
        { 
            // I think this line is useless.
            // pm.SetValue(std::move(results));
        }
             
        Promise<ara::core::Vector<Try<T>>> pm;
        ara::core::Vector<Try<T>> results;
        std::atomic<size_t> collected{0};
    };

    auto ctx = std::make_shared<CollectAllContext>(std::distance(first, last));
                
    for (size_t i = 0; first != last; ++first, ++i)
    {
        first->SetCallback([ctx, i](Try<T>&& t) {
                ctx->results[i] = std::move(t);
                if (ctx->results.size() - 1 ==
                    std::atomic_fetch_add (&ctx->collected, std::size_t(1))) {
                    ctx->pm.SetValue(std::move(ctx->results));
                }
            });
    }

    return ctx->pm.GetFuture();
}

// When Any
template <class InputIterator>
Future<
  std::pair<size_t,
           Try<typename std::iterator_traits<InputIterator>::value_type::InnerType>>>
WhenAny(InputIterator first, InputIterator last)
{
    using T = typename std::iterator_traits<InputIterator>::value_type::InnerType;

    if (first == last)
    {
        return MakeReadyFuture(std::make_pair(size_t(0), Try<T>(T())));
    }

    struct CollectAnyContext
    {
        CollectAnyContext() {};
        Promise<std::pair<size_t, Try<T>>> pm;
        std::atomic<bool> done{false};
    };

    auto ctx = std::make_shared<CollectAnyContext>();
    for (size_t i = 0; first != last; ++first, ++i)
    {
        first->SetCallback([ctx, i](Try<T>&& t) {
            if (!ctx->done.exchange(true)) {
                ctx->pm.SetValue(std::make_pair(i, std::move(t)));
            }
       });
    }
           
    return ctx->pm.GetFuture();
}


// When N 
template <class InputIterator>
Future<
    ara::core::Vector<
    std::pair<size_t, Try<typename std::iterator_traits<InputIterator>::value_type::InnerType>>
    >
    >
    WhenN(size_t N, InputIterator first, InputIterator last)
{
    using T = typename std::iterator_traits<InputIterator>::value_type::InnerType;

    size_t nFutures = std::distance(first, last);
    const size_t needCollect = std::min(nFutures, N);

    if (needCollect == 0)
    {
        return MakeReadyFuture(ara::core::Vector<std::pair<size_t, Try<T>>>());
    }

    struct CollectNContext
    {
        CollectNContext(size_t _needs) : needs(_needs) {}
        Promise<ara::core::Vector<std::pair<size_t, Try<T>>>> pm;
    
        std::mutex mutex;
        ara::core::Vector<std::pair<size_t, Try<T>>> results;
        const size_t needs;
        bool done {false};
    };

    auto ctx = std::make_shared<CollectNContext>(needCollect);
    for (size_t i = 0; first != last; ++first, ++i)
    {
        first->SetCallback([ctx, i](Try<T>&& t) {
                std::unique_lock<std::mutex> guard(ctx->mutex);
                if (ctx->done)
                    return;
                    
                ctx->results.push_back(std::make_pair(i, std::move(t)));
                if (ctx->needs == ctx->results.size()) {
                    ctx->done = true;
                    ctx->pm.SetValue(std::move(ctx->results));
                }
            });
    }
           
    return ctx->pm.GetFuture();
}
// When if N
template <class InputIterator>
Future<
    ara::core::Vector<std::pair<size_t, typename TryWrapper<typename std::iterator_traits<InputIterator>::value_type::InnerType>::Type>>
>
WhenIfN(size_t N, InputIterator first, InputIterator last,
    std::function<bool(const Try<typename std::iterator_traits<InputIterator>::value_type::InnerType>&)> cond) {

    using T = typename std::iterator_traits<InputIterator>::value_type::InnerType;
    using TryT = typename TryWrapper<T>::Type;

    size_t nFutures = std::distance(first, last);
    const size_t needCollect = std::min<size_t>(nFutures, N);

    if (needCollect == 0) {
        return MakeReadyFuture(ara::core::Vector<std::pair<size_t, TryT>>());
    }

    struct IfNContext {
        IfNContext(size_t _needs) : needs(_needs) {}
        Promise<ara::core::Vector<std::pair<size_t, TryT>>> pm;

        std::mutex mutex;
        ara::core::Vector<std::pair<size_t, TryT>> results;
        size_t returned{ 0 }; // including fail response, eg, cond(rsp) == false
        const size_t needs;
        bool done{ false };
    };

    auto ctx = std::make_shared<IfNContext>(needCollect);
    for (size_t i = 0; first != last; ++first, ++i) {
        first->Then([ctx, i, nFutures, cond](TryT&& t) {
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
} // end namespace ananas

#endif