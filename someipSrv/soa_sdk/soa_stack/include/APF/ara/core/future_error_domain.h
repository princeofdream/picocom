#ifndef _FUTURE_ERROR_DOMAIN_H_
#define _FUTURE_ERROR_DOMAIN_H_
#include "exception.h"
#include "error_domain.h"
#include "../../TableDriveTool.h"

namespace ara{
namespace core{
    //[SWS_CORE_00400]
    enum class future_errc
    {
        broken_promise= 101,            //the asynchronous task abandoned its shared state
        future_already_retrieved= 102,  //the contents of the shared state were already accessed
        promise_already_satisfied= 103, //attempt to store a value into the shared state twice
        no_state= 104,                  //attempt to access Promise or Future without an associated state
        unknown_error=105,
    };
    //[SWS_CORE_00411]
    class FutureException : public Exception {
    public:
        //[SWS_CORE_00412]
        explicit FutureException(ErrorCode err) noexcept :Exception(err) {}
        char const* what() const noexcept override {
            return m_error_code.Message();
        }
        ara::core::ErrorCode const& Error() const noexcept {
            return m_error_code;
        }
    };
    //[SWS_CORE_00421]
    class FutureErrorDomain final : public ErrorDomain {
    public:
        //[SWS_CORE_00431]
        using Errc = future_errc;
        //[SWS_CORE_00432]
        using Exception = FutureException;
        //[SWS_CORE_00441]
        constexpr FutureErrorDomain () noexcept : ErrorDomain(0x8000000000000013) {

        }
        //[SWS_CORE_00442]
        char const* Name () const noexcept override {
            return "Future";
        }
        //[SWS_CORE_00443]
        char const* Message(ErrorDomain::CodeType errorCode) const noexcept override {
            static const std::pair<Errc, const char*> err_and_message[] = {
                { Errc::broken_promise , "the asynchronous task abandoned its shared state." },
                { Errc::future_already_retrieved , "the contents of the shared state were already accessed." },
                { Errc::promise_already_satisfied , "attempt to store a value into the shared state twice." },
                { Errc::no_state , "attempt to access Promise or Future without an associated state." },
                { Errc::unknown_error , "unknown excption." },
            };

            return MatchAndPickupItem_withDefault(err_and_message, _item.first == static_cast<Errc>(errorCode), _item.second, "");
        }
        //[SWS_CORE_00444]
        void ThrowAsException (ErrorCode const &errorCode) const noexcept(false) override {
            throw(Exception(errorCode));
        }
    };
    //[SWS_CORE_00480]
    constexpr ErrorDomain const& GetFutureDomain () noexcept;
    //[SWS_CORE_00490]
    constexpr ErrorCode MakeErrorCode (future_errc code, ErrorDomain::SupportDataType data = 0, char const *message = nullptr) noexcept;

    static FutureErrorDomain s_Future_error_domain;
    class FutureTmpClass {
        friend constexpr ErrorDomain const& GetFutureDomain() noexcept {
            return s_Future_error_domain;
        }

        friend constexpr ErrorCode MakeErrorCode(future_errc code, ErrorDomain::SupportDataType data, char const* message) noexcept {
            return ara::core::ErrorCode((ErrorDomain::CodeType)code, GetFutureDomain(), data, message);
        }

    };
    template <>
    struct is_error_code_enum<future_errc> : std::true_type
    {	// for ErrorCode constructor overload resolution
    };
}
}
#endif