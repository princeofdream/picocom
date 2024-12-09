#ifndef _PROMISE_H_
#define _PROMISE_H_
#include "future.h"
#include "error_code.h"
namespace ara {
namespace core {
    //[SWS_CORE_00340]
    template <typename T, typename E>
    class Promise
    {
    public:
        /*
        * [SWS_CORE_00341]
        * Default constructor.
        * This function shall behave the same as the corresponding std::promise function.
        */
        Promise() :
            state_(std::make_shared<internal::State<T>>())
        {
        }

        // TODO: C++11 lambda doesn't support move capture
        // just for compile, copy Promise is ub, do NOT do that!
        /*
        * [SWS_CORE_00350]
        * Copy constructor shall be disabled.----TODO
        */
        Promise(const Promise&) = default;
        /*
        * [SWS_CORE_00351]
        * Copy assignment operator shall be disabled.----TODO
        */
        Promise& operator= (const Promise&) = default;

        /*
        * [SWS_CORE_00342]
        * Move constructor.
        * This function shall behave the same as the corresponding std::promise function.
        */
        Promise(Promise&& pm) noexcept = default;
        /*
        * [SWS_CORE_00343]
        * Move assignment.
        * This function shall behave the same as the corresponding std::promise function.
        */
        Promise& operator= (Promise&& pm) noexcept = default;

        /*
        * [SWS_CORE_00349]
        * Destructor for Promise objects.
        * This function shall behave the same as the corresponding std::promise function.
        */
        ~Promise() {}

        /*
        * [SWS_CORE_00352]
        * Swap the contents of this instance with another one¡¯s.
        * This function shall behave the same as the corresponding std::promise function.
        */
        void swap(Promise& other) noexcept;

        void SetException(std::exception_ptr exp)
        {
            std::unique_lock<std::mutex> guard(state_->thenLock_);
            bool isRoot = state_->IsRoot();
            if (isRoot && state_->progress_ != internal::Progress::None)
            {
                if (state_->value_.HasValue() || state_->value_.HasException())
                {
                    GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::promise_already_satisfied));
                }
                return;
            }

            state_->progress_ = internal::Progress::Done;
            state_->value_ = typename internal::State<T>::ValueType(std::move(exp));
            guard.unlock();

            if (state_->then_)
                state_->then_(std::move(state_->value_));
            state_->DoNotify();
        }

        /*
        * [SWS_CORE_00353]
        * Move an error into the shared state and make the state ready.
        */
        void SetError(E&& error) {
            SetException(std::make_exception_ptr(FutureException(std::move(error))));
        }
        /*
        * [SWS_CORE_00354]
        * Copy an error into the shared state and make the state ready.
        */
        void SetError(E const& error) {
            SetException(std::make_exception_ptr(FutureException(error)));
        }

        template <typename ValueType = T>
        typename std::enable_if<!std::is_void<ValueType>::value, void>::type
        SetValue(ValueType&& t)
        {
            // if ThenImp is running, here will wait for the lock.
            // ThenImp will release lock after set then_ callback.
            // And this func acquired lock, definitely call then_.
            std::unique_lock<std::mutex> guard(state_->thenLock_);
            bool isRoot = state_->IsRoot();
            if (isRoot && state_->progress_ != internal::Progress::None)
            {
                if (state_->value_.HasValue() || state_->value_.HasException())
                {
                    GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::promise_already_satisfied));
                }
                return;
            }

            state_->progress_ = internal::Progress::Done;
            state_->value_ = std::forward<ValueType>(t);

            guard.unlock();
            // when here, the state is defined, so mutex is useless
            // If the ThenImp function run, it'll see the Done state
            // and call func there, not use then_ callback.
            if (state_->then_)
                state_->then_(std::move(state_->value_));
            state_->DoNotify();
        }

        /*
        * [SWS_CORE_00346]
        * Move a value into the shared state and make the state ready.
        * This function shall behave the same as the corresponding std::promise function.
        */
        template <typename ValueType = T, typename = typename std::enable_if<!std::is_void<ValueType>::value, void>::type>        
        void set_value(ValueType&& value)
        {
            SetValue(std::move(value));
        }

        template <typename ValueType = T>
        typename std::enable_if<!std::is_void<ValueType>::value, void>::type
        SetValue(const ValueType& t)
        {
            std::unique_lock<std::mutex> guard(state_->thenLock_);
            bool isRoot = state_->IsRoot();
            if (isRoot && state_->progress_ != internal::Progress::None)
            {
                if (state_->value_.HasValue() || state_->value_.HasException())
                {
                    GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::promise_already_satisfied));
                }
                return;
            }

            state_->progress_ = internal::Progress::Done;
            state_->value_ = t;

            guard.unlock();
            if (state_->then_)
                state_->then_(std::move(state_->value_));
            state_->DoNotify();
        }
        /*
        * [SWS_CORE_00345]
        * Copy a value into the shared state and make the state ready.
        * This function shall behave the same as the corresponding std::promise function.
        */
        template <typename ValueType = T, typename = typename std::enable_if<!std::is_void<ValueType>::value, void>::type>        
        void set_value(const ValueType& t) {
            SetValue(t);
        }

        template <typename ValueType = T>
        typename std::enable_if<!std::is_void<ValueType>::value, void>::type
        SetValue(Try<ValueType>&& t)
        {
            std::unique_lock<std::mutex> guard(state_->thenLock_);
            bool isRoot = state_->IsRoot();
            if (isRoot && state_->progress_ != internal::Progress::None)
            {
                if (state_->value_.HasValue() || state_->value_.HasException())
                {
                    GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::promise_already_satisfied));
                }
                return;
            }

            state_->progress_ = internal::Progress::Done;
            state_->value_ = std::forward<Try<ValueType>>(t);

            guard.unlock();
            if (state_->then_)
                state_->then_(std::move(state_->value_));
            state_->DoNotify();
        }

        template <typename ValueType = T>
        typename std::enable_if<!std::is_void<ValueType>::value, void>::type
        SetValue(const Try<ValueType>& t)
        {
            std::unique_lock<std::mutex> guard(state_->thenLock_);
            bool isRoot = state_->IsRoot();
            if (isRoot && state_->progress_ != internal::Progress::None)
            {
                if (state_->value_.HasValue() || state_->value_.HasException())
                {
                    GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::promise_already_satisfied));
                }
                return;
            }

            state_->progress_ = internal::Progress::Done;
            state_->value_ = t;

            guard.unlock();
            if (state_->then_)
                state_->then_(std::move(state_->value_));
            state_->DoNotify();
        }

        template <typename ValueType = T>
        typename std::enable_if<std::is_void<ValueType>::value, void>::type
        SetValue(Try<void>&&)
        {
            std::unique_lock<std::mutex> guard(state_->thenLock_);
            bool isRoot = state_->IsRoot();
            if (isRoot && state_->progress_ != internal::Progress::None)
            {
                if (state_->value_.HasValue() || state_->value_.HasException())
                {
                    GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::promise_already_satisfied));
                }
                return;
            }

            state_->progress_ = internal::Progress::Done;
            state_->value_ = Try<void>();

            guard.unlock();
            if (state_->then_)
                state_->then_(std::move(state_->value_));
            state_->DoNotify();
        }

        template <typename ValueType = T>
        typename std::enable_if<std::is_void<ValueType>::value, void>::type
        SetValue(const Try<void>&)
        {
            std::unique_lock<std::mutex> guard(state_->thenLock_);
            bool isRoot = state_->IsRoot();
            if (isRoot && state_->progress_ != internal::Progress::None)
            {
                if (state_->value_.HasValue() || state_->value_.HasException())
                {
                    GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::promise_already_satisfied));
                }
                return;
            }

            state_->progress_ = internal::Progress::Done;
            state_->value_ = Try<void>();

            guard.unlock();
            if (state_->then_)
                state_->then_(std::move(state_->value_));
            state_->DoNotify();
        }

        template <typename ValueType = T>
        typename std::enable_if<std::is_void<ValueType>::value, void>::type
        SetValue()
        {
            std::unique_lock<std::mutex> guard(state_->thenLock_);
            bool isRoot = state_->IsRoot();
            if (isRoot && state_->progress_ != internal::Progress::None)
            {
                if (state_->value_.HasValue() || state_->value_.HasException())
                {
                    GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::promise_already_satisfied));
                }
                return;
            }

            state_->progress_ = internal::Progress::Done;
            state_->value_ = Try<void>();

            guard.unlock();
            if (state_->then_)
                state_->then_(std::move(state_->value_));
            state_->DoNotify();
        }

        Future<T> GetFuture()
        {
            bool expect = false;
            if (!state_->retrieved_.compare_exchange_strong(expect, true))
            {
                /*struct FutureAlreadyRetrieved {};
                throw FutureAlreadyRetrieved();*/
                GetFutureDomain().ThrowAsException(MakeErrorCode(future_errc::future_already_retrieved));
            }

            return Future<T>(state_);
        }

        /*
        * [SWS_CORE_00344]
        * Return the associated Future.
        * The returned Future is set as soon as this Promise receives the result or an error. This method
        * must only be called once as it is not allowed to have multiple Futures per Promise.
        */
        Future<T> get_future() {
            return std::move(GetFuture());
        }

        bool IsReady() const
        {
            return state_->progress_ != internal::Progress::None;
        }

        /*
        * [SWS_CORE_00355]
        * Copy a Result into the shared state and make the state ready.
        */
        void SetResult(const Result< T, E >& result);
        /*
        * [SWS_CORE_00356]
        * Move a Result into the shared state and make the state ready.
        */
        void SetResult(Result< T, E >&& result);
    private:
        std::shared_ptr<internal::State<T>> state_;
    };

    inline Future<void> MakeReadyFuture()
    {
        Promise<void> pm;
        auto f(pm.GetFuture());
        pm.SetValue();

        return f;
    }
}
}
#endif