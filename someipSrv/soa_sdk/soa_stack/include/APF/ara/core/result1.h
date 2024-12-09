#ifndef __RESULT_H__
#define __RESULT_H__
/*
 *
 * \brief Encapsulation of an error code
 * 
 * An ErrorCode contains a raw error code value and an error domain. The raw error code value is
 * specific to this error domain
 * 
 * AutoSAR Adaptive Platform Release 18-10
 * Author : Even Lee
 * 
 * 2019-3-30
 * 
 */

#include <cstdint>
#include <memory>
#include <exception>

#include "error_code.h"

namespace ara {
namespace core {


// helplers for Result::Bind begin
template <typename T, typename E>
class Result;

template <typename T>
struct isResult : std::false_type {
	using value_type = T;
};

//template <>
template <typename T, typename E>
struct isResult<Result<T, E>> : std::true_type {
	using value_type = T;
};

//// helplers for Result::Bind end;

// \brief This class is a type that contains either a value or an error.
// \trace_id [SWS_CORE_00701]
template <typename T, typename E = ErrorCode>
class Result {
public:
    // Type alias for the type T of values.
    // \trace_id : [SWS_CORE_00711]
    using value_type = T;

    // Type alias for the type E of errors.
    // \trace_id : [SWS_CORE_00712]
    using error_type = E;

public:
    // \brief Construct a new Result from the specified value (given as lvalue)
    // \trace_id : [SWS_CORE_00721]
	Result(T const & t) : m_has_error(false) {
		m_value = std::make_unique<T>(t);
	}

    // \brief Construct a new Result from the specified value (given as rvalue)
    // \trace_id : [SWS_CORE_00722]
	Result(T && t) : m_has_error(false) {
		m_value = std::make_unique<T>(std::move(t));

	}
    // \brief Construct a new Result from the specified error (given as lvalue)
    // \trace_id : [SWS_CORE_00723]
	Result(E const & e) : m_has_error(true) {
		m_error = std::make_unique<E>(e);
	}

    // \brief Construct a new Result from the specified error (given as rvalue)
    // \trace_id : [SWS_CORE_00724]
	Result(E && e) : m_has_error(true) {
		m_error = std::make_unique<E>(std::move(e));
	}

    // \brief Copy-construct a new Result from another instance (given as lvalue)
    // \trace_id : [SWS_CORE_00725]
	Result(Result const & other) {
		*this = other;
	}

    // \brief Copy-construct a new Result from another instance (given as rvalue)
    // \trace_id : [SWS_CORE_00726]
	Result(Result && other) {
		*this = std::move(other);
	}

public:
    // \brief Build a new Result from the specified value (given as lvalue).
    // \return a Result that contains a value
    // \trace_id : [SWS_CORE_00731]
	static Result FromValue(T const & t) {
		return Result(t);
	}

    // \brief Build a new Result from the specified value (given as rvalue).
    // \return a Result that contains a value
    // \trace_id : [SWS_CORE_00732]
	static Result FromValue(T && t) {
		return Result(std::move(t));
	}

    // \brief Build a new Result from a value that is constructed in-place from the given arguments
    //
    // This function shall not participate in overload resolution unless: std::is_constructible<T, Args&&...>::value 
    // is true, and the first type of the expanded parameter pack is not T, andthe first
    // type of the expanded parameter pack is not a specialization of Result
    //
    // \return a Result that contains a value
    // \trace_id : [SWS_CORE_00733]
    template <typename ... Args>
	static Result FromValue(Args && ... args) {
		return Result<T>(T(args...));
	}

    // \brief Build a new Result from the specified error (given as lvalue).
    // \return a Result that contains an error
    // \trace_id : [SWS_CORE_00734]
	static Result FromError(E const & e) {
		return Result(e);
	}

    // \brief Build a new Result from the specified error (given as rvalue).
    // \return a Result that contains an error
    // \trace_id : [SWS_CORE_00735]
	static Result FromError(E && e) {
		return Result(std::move(e));
	}

    // \brief Build a new Result from an error that is constructed in-place from the given arguments
    //
    // \tempalte_param Args : the types of arguments given to this function
    //
    // \param args : the arguments used for constructing the error
    //
    // This function shall not participate in overload resolution unless: std::is_constructible<E, Args&&...>::value is true, 
    // and the first type of the expanded parameter pack is not E, and the
    // first type of the expanded parameter pack is not a specialization of R
    //
    // \return a Result that contains an error
    // \trace_id : [SWS_CORE_00736]
    template <typename ... Args>
	static Result FromError(Args && ... args) {
		return Result<T, E>(E(args...));
	}

public:
    // \brief Copy-assign another Result to this instance
    //
    // \param other : the other instance
    // \return *this, containing the contents of other
    // \trace_id : [SWS_CORE_00741]
	Result & operator = (Result const & other) {
		m_has_error = other.m_has_error;
		if (m_has_error) {
			m_error = std::make_unique(*other.m_error);
		}
		else {
			m_value = std::make_unique(*other.m_value);
		}

        return *this;

	}
	

    // \brief Move-assign another Result to this instance
    //
    // \param other : the other instance
    // \return *this, containing the contents of other
    // \trace_id : [SWS_CORE_00742]
    Result & operator = (Result && other) {
		m_has_error = other.m_has_error;
		if (m_has_error) {
			m_error = std::make_unique<E>(std::move(*other.m_error));
		}
		else {
			m_value = std::make_unique<T>(std::move(*other.m_value));
		}

        return *this;
	}


    // \brief Put a new value into this instance, constructed in-place from the given arguments
    //
    // \tempalte_param Args : the types of arguments given to this function
    //
    // \param args : the arguments used for constructing the value
    // \trace_id : [SWS_CORE_00743]
    template <typename ... Args>
	void EmplaceValue(Args && ... args) {
		m_has_error = false;
		m_error = std::make_unique<T>(args...);
	}

    // \brief Put a new error into this instance, constructed in-place from the given arguments
    //
    // \tempalte_param Args : the types of arguments given to this function
    //
    // \param args : the arguments used for constructing the error
    // \trace_id : [SWS_CORE_00744]
    template <typename ... Args>
	void EmplaceError(Args && ... args) {
		m_has_error = true;
		m_error = std::make_unique<E>(args...);
	}

    // \brief Exchange the contents of this instance with those of other.
    //
    // \param other : the other instances
    // \trace_id : [SWS_CORE_00745]
    void Swap(Result& other); // T.B.D C++14 not support? !noexcept(std::is_nothrow_constructible<T>::value && std::is_nothrow_assignable<T>::value);

public:
    // \brief Check whether *this contains a value
    // \return true if *this contains a value, false otherwise
    // \trace_id : [SWS_CORE_00751]
    bool HasValue() const noexcept {
		return !m_has_error;
	}

    // \brief Check whether *this contains a value
    // \return true if *this contains a value, false otherwise
    // \trace_id : [SWS_CORE_00752]
	explicit operator bool() const noexcept {
		return HasValue();
	}

    // \brief Access the contained value.
    // This function’s behavior is undefined if *this does not contain a value.
    // a reference to the contained value
    //
    // \return a reference to the contained value
    // \trace_id : [SWS_CORE_00753]
    T const & operator * () const;

    // \brief Access the contained value.
    // This function’s behavior is undefined if *this does not contain a value.
    // a pointer to the contained value
    //
    // \return a pointer to the contained value
    // \trace_id : [SWS_CORE_00754]
    T const * operator -> () const;

    // \brief Access the contained value
    //
    // The behavior of this function is undefined if *this does not contain a value.
    // a const reference to the contained value.
    //
    // \return T const & a const reference to the contained value.
    // \trace_id : [SWS_CORE_00755]
    // \note : TODO: -issue doc contains  T const & Value() const &; & is unnecessary.
	T const & Value() const & {
		return *m_value;
	}

    // \brief Access the contained value
    //
    // The behavior of this function is undefined if *this does not contain a value.
    // a rvalue reference to the contained value.
    //
    // \return T && rvalue reference to the contained value.
    // \trace_id : [SWS_CORE_00756]
    // \note : TODO: -issue doc contains  T const & Value() const &; & is unnecessary.
	T && Value() && {
		return *m_value;
	}

    // \brief Access the contained error
    //
    // The behavior of this function is undefined if *this does not contain a error.
    // a const reference to the contained error.
    //
    // \return T const & a const reference to the contained error.
    // \trace_id : [SWS_CORE_00757]
    // \note : TODO: -issue doc contains  T const & Error() const &; & is unnecessary.
	E const & Error() const & {
		return *m_error;
	}

    // \brief Access the contained error
    //
    // The behavior of this function is undefined if *this does not contain a error.
    // a rvalue reference to the contained error.
    //
    // \return T && rvalue reference to the contained error.
    // \trace_id : [SWS_CORE_00758]
    // \note : TODO: -issue doc contains  T const & Error() const &; & is unnecessary.
	E && Error() && {
		return std::move(*m_error);
	}

    // \brief Return the contained value or the given default value.
    // If *this contains a value, it is returned. Otherwise, the specified default value is returned,
    // static_cast’d to T.
    // \trace_id : [SWS_CORE_00761]
    // \note : TODO: -issue doc contains  return type and parameter type combination is not clear.
    template <typename U>
    typename std::enable_if<std::is_convertible<U, T>::value, T>::type ValueOr(U && defaultValue) const & {
		if (m_has_error) {
			return std::move(defaultValue);
		}
		return *m_value;
	} // TODO : enable_if in other way?

    // \brief Return the contained value or the given default value.
    // If *this contains a value, it is returned. Otherwise, the specified default value is returned,
    // static_cast’d to T.
    // \trace_id : [SWS_CORE_00762]
    template <typename U>
    typename std::enable_if<std::is_convertible<U, T>::value, T>::type  ValueOr(U && defaultValue) && {
		if (m_has_error) {
			return std::move(defaultValue);
		}
		return std::move(*m_value);		
	}

    // \brief Return the contained error or the given default error.
    // If *this contains a error, it is returned. Otherwise, the specified default error is returned,
    // static_cast’d to E.
    // \trace_id : [SWS_CORE_00763]
    template <typename G>
	E const & ErrorOr(G && defaultError) {
		if (m_has_error) {
			return *m_error;
		}
		return std::move(defaultError);
	}

    // \brief Return the contained error or the given default error.
    // If *this contains a error, it is returned. Otherwise, the specified default error is returned,
    // static_cast’d to E.
    // \trace_id : [SWS_CORE_00764]
    // \note : TODO: -issue SWS_CORE_00764 not in document.....
    //template <typename G>
    //E const & ErrorOr(G && defaultError);

    // \brief Return the contained error or the given default error.
    // \tempalte_param G the type of the error argument error
    // \param error : the error to check
    // \return bool true if *this contains an error that is equivalent to the given error, false otherwise
    // \trace_id : [SWS_CORE_00765]
    template <typename G>
    bool CheckError(G && error) const;

    // \brief Return the contained value or throw an exception.
    // This function does not participate in overload resolution when the compiler toolchain does not
    // support C++ exceptions
    // \return T const & : the value
    // \exception the exception type associated with the contained error.
    // \trace_id : [SWS_CORE_00766]
    T const& ValueOrThrow() const noexcept(false) {
        if (HasValue()) {
        }
        else {
            (*m_error).Domain().ThrowAsException(*m_error);
        }
        return *m_value;
    }

    // \brief Return the contained value or return the result of a function call.
    // If *this contains a value, it is returned. Otherwise, the specified callable is invoked and its return
    // value which is to be compatible to type T is returned from this function.
    // The Callable is expected to be compatible to this interface: T f(E const&);
    // 
    // \tempalte_param F the type of the Callable f
    // \param f : the Callable
    // \return T the value
    // \trace_id : [SWS_CORE_00767]
    template <typename F>
    T Resolve(F && f) const {
        if (HasValue()) {
            return *m_value;
        }
        
        return f(*m_error);
    }

    // \brief Apply the given Callable to the value of this instance, and return a new Result with the result of the call.
    //
    // The Callable is expected to be compatible to one of these two interfaces: 
    // Result<XXX, E> f(T const&); XXX f(T const&); meaning that the Callable either returns a Result<XXX> or a XXX
    // directly, where XXX can be any type that is suitable for use by class Result.
    // The return type of this function is decltype(f(Value())) for a template argument F that returns a
    // Result type, and it is Result<decltype(f(Value())), E> for a template argument F that does not
    // return a Result type.
    // If this instance does not contain a value, a new Result<XXX, E> is still created and returned,
    // with the original error contents of this instance being copied into the new instance.
    // 
    // \tempalte_param F the type of the Callable f
    // \param f : the Callable
    //
    // \return auto a new Result instance of the possibly transformed type
    // \trace_id : [SWS_CORE_00768]
    // \note : TODO : may not able to support [](int &) {} -> note the parameter, (const int &) and (int) is OK, but int & isn't
    template <typename F>
    auto Bind(F && f) const 
        -> typename std::conditional<isResult<decltype(f(Value()))>::value,
									 decltype(f(Value())),
									 Result<decltype(f(Value())), E>
									>::type
    {
        if (HasValue()) {
            return f(Value());
        }
        return *m_error;
    }
    

private:
    bool       m_has_error;
	// m_value and m_error may better use std::variant.
    std::unique_ptr<value_type> m_value;
    std::unique_ptr<error_type> m_error;
	// std::variant<T, E> m_contents;
};


// \brief Specialization of class Result for "void" values. 
// \trace_id : [SWS_CORE_00801]
template <typename E>
class Result<void, E> {

public:
    // Type alias for the type E of errors [SWS_CORE_00812]
    using error_type = E;

    // Construct a new Result with "void" value [SWS_CORE_00821]
    Result() noexcept :m_has_error(false) {}

    // Construct a new Result from the specified error (given as lvalue) [SWS_CORE_00823]
    explicit Result(E const & e) : m_has_error(true) {
        m_error = std::make_unique<E>(e);
    }

    // Construct a new Result from the specified error (given as rvalue) [SWS_CORE_00824]
    explicit Result(E && e) : m_has_error(true) {
        m_error = std::make_unique<E>(std::move(e));
    }

    // Copy-construct a new Result from another instance. [SWS_CORE_00825]
    Result(Result const& other) {
        *this = other;
    }

    // Copy-construct a new Result from another instance. [SWS_CORE_00826]
    Result(Result&& other) {
        *this = std::move(other);
    }

    // \brief Destructor.
    // 
    // If the object contains an error and its type E is not trivially destructible (determined by
    // std::is_trivially_destructible<E>), then this destructor destroys the contained value by calling its
    // destructor. Otherwise, does nothing.
    ~Result() {
    }

    // \brief Build a new Result with "void" as value [SWS_CORE_00831]
    static Result FromValue() {
        return Result();
    }

    // \brief Build a new Result from the specified error (given as lvalue) [SWS_CORE_00834]
    static Result FromError(E const e) {
        return Result(e);
    }

    // \brief Build a new Result from the specified error (given as lvalue) [SWS_CORE_00835]
    static Result FromError(E && e) {
        return Result(std::move(e));
    }

    // \brief Build a new Result from an error that is constructed in-place from the given arguments.

    // This function shall not participate in overload resolution unless: std::is_constructible<E, Args&&...>::value 
    // is true, and the first type of the expanded parameter pack is not E, andthe
    // first type of the expanded parameter pack is not a specialization of Result[SWS_CORE_00836]
    template <typename ... Arg>
    static Result FromError(Arg && ...args) {
        return Result(E(args...));
    }

    // \brief Copy-assign another Result to this instance. [SWS_CORE_00841]
    Result& operator = (Result const &other);

    // \brief Move-assign another Result to this instance. [SWS_CORE_00842]
    Result& operator = (Result && other);

    // \brief Put a new value into this instance, constructed in-place from the given arguments. [SWS_CORE_00843]
    template <typename ...Arg>
    void EmplaceValue(Arg && ...args) noexcept;

    // \brief Put a new error into this instance, constructed in-place from the given arguments [SWS_CORE_00844]
    template <typename ...Arg>
    void EmplaceError(Arg && ...args) noexcept;

    // \brief Exchange the contents of this instance with those of other. [SWS_CORE_00845]
    void Swap (Result & other) noexcept(
        std::is_nothrow_move_constructible<E>::value
        && std::is_nothrow_move_assignable<E>::value);

    // \brief Check whether *this contains a value. [SWS_CORE_00851]
    bool HasValue() const noexcept {
        return !m_has_error;
    }

    // \brief Check whether *this contains a value. [SWS_CORE_00852]
    explicit operator bool () const noexcept {
        return !m_has_error;
    }

    // \brief Do nothing. [SWS_CORE_00855]
    // 
    // This function only exists for helping with generic programming.
    // The behavior of this function is undefined if *this does not contain a value.
    void Value () const ;

    // \brief Access the contained error. [SWS_CORE_00857]
    // 
    // The behavior of this function is undefined if *this does not contain an error.
    E const& Error() const& {
        return *m_error;
    }

    // \brief Access the contained error. [SWS_CORE_00858]
    // The behavior of this function is undefined if *this does not contain an error.
    E&& Error() &&;

    // \brief Return the contained error or the given default error. [SWS_CORE_00863]
    // 
    // If *this contains an error, it is returned. Otherwise, the specified default error is returned,
    // static_cast’d to E.
    template <typename G>
    E ErrorOr(G && defaultError) const;

    // \brief Return whether this instance contains the given error  [SWS_CORE_00865]
    // 
    // This call compares the argument error, static_cast’d to E, with the return value from Error().
    template <typename G>
    bool CheckError(G && error) const;

    // \brief Return the contained value or throw an exception. [SWS_CORE_00866]
    // 
    // This function does not participate in overload resolution when the compiler toolchain does not
    // support C++ exceptions.
    void ValueOrThrow() noexcept(false);
private:
    bool       m_has_error;
    std::unique_ptr<error_type> m_error;

};



// \brief  Swap the contents of the two given arguments. [SWS_CORE_00796]
// 
// \param lhs : one instance
// \param rhs : another instance
template <typename T, typename E>
void swap(Result<T, E> & lhs, Result<T, E> & rhs) noexcept(noexcept(lhs.Swap(rhs)));
    


} // core    
} // ara


#endif // __RESULT_H__

