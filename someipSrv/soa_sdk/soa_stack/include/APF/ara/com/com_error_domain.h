#ifndef _ARA_COM_ERROR_DOMAIN_H_
#define _ARA_COM_ERROR_DOMAIN_H_
#include "ara/core/error_code.h"
#include "ara/core/exception.h"
#include "ara/core/error_domain.h"
#include "TableDriveTool.h"
namespace ara{
namespace com{
    //[SWS_CM_10432] in 20.11
    enum class ComErrc : ara::core::ErrorDomain::CodeType
    {
        kServiceNotAvailable = 1,
        kMaxSamplesReached = 2,
        kNetworkBindingFailure = 3,
        kGrantEnforcementError = 4,
        kPeerIsUnreachable = 5,
        kFieldValueIsNotValid = 6,
        kSetHandlerNotSet = 7,
        kUnsetFailure = 8,
        kSampleAllocationFailure = 9,
        kIllegalUseOfAllocate = 10,
        kServiceNotOffered = 11,
        kCommunicationLinkError = 12,
        kNoClients = 13,
        kCommunicationStackError = 14,
        kInstanceIDCouldNotBeResolved = 15,
        kMaxSampleCountNotRealizable = 16,
        kWrongMethodCallProcessingMode = 17,
        kErroneousFileHandle = 18,
        kCouldNotExecute = 19,
    };
    //[SWS_CM_11327]
    class ComException : public ara::core::Exception {
    public:
        //[SWS_CORE_00412]
        explicit ComException(ara::core::ErrorCode err) noexcept : Exception(err) {

        }
        char const* what() const noexcept override {
            return m_error_code.Message();
        }
        ara::core::ErrorCode const& Error() const noexcept {
            return m_error_code;
        }
    };
    //[SWS_CM_11329]
    class ComErrorDomain final : public ara::core::ErrorDomain {
    public:
        //[SWS_CM_11336]
        using Errc = ComErrc;
        //[SWS_CM_11337]
        using Exception = ComException;
        //[SWS_CM_11330]
        constexpr ComErrorDomain() noexcept : ErrorDomain(0x8000000000001267) {

        }
        //[SWS_CM_11331]
        char const* Name() const noexcept override {
            return "Com";
        }
        //[SWS_CM_11332]
        char const* Message(ErrorDomain::CodeType errorCode) const noexcept override {
            static const std::pair<Errc, const char*> err_and_message[] = {
                { Errc::kServiceNotAvailable , "Service is not available." },
                { Errc::kMaxSamplesReached , "Application holds more SamplePtrs than commited in Subscribe()." },
                { Errc::kNetworkBindingFailure , "Local failure has been detected by the network binding." },
                { Errc::kGrantEnforcementError , "Request was refused by Grant enforcement layer." },
                { Errc::kPeerIsUnreachable , "TLS handshake fail." },
                { Errc::kFieldValueIsNotValid , "Field Value is not valid,." },
                { Errc::kSetHandlerNotSet , "SetHandler has not been registered." },
                { Errc::kUnsetFailure , "Failure has been detected by unset operation." },
                { Errc::kSampleAllocationFailure , "Not Sufficient memory resources can be allocated." },
                { Errc::kIllegalUseOfAllocate , "The allocation was illegally done via custom allocator (i.e., not via shared memory allocation)." },
                { Errc::kServiceNotOffered , "Service not offered." },
                { Errc::kCommunicationLinkError , "Communication link is broken." },
                { Errc::kNoClients , "No clients connected." },
                { Errc::kCommunicationStackError , "Communication Stack Error, e.g. network stack, network binding, or communication framework reports an error" },
                { Errc::kInstanceIDCouldNotBeResolved , "ResolveInstanceIDs() failed to resolve InstanceID from InstanceSpecifier, i.e. is not mapped correctly." },
                { Errc::kMaxSampleCountNotRealizable , "Provided maxSampleCount not realizable." },
                { Errc::kWrongMethodCallProcessingMode, "Wrong processing mode passed to constructor method call." },
                { Errc::kErroneousFileHandle , "The FileHandle returned from FindServce is corrupt/service not available." },
                { Errc::kCouldNotExecute , "Command could not be executed in provided Execution Context." },
            };

            return MatchAndPickupItem_withDefault(err_and_message, _item.first == static_cast<Errc>(errorCode), _item.second, "");
        }
        //[SWS_CM_11333]
        void ThrowAsException(ara::core::ErrorCode const& errorCode) const noexcept(false) override {
            throw(Exception(errorCode));
        }
    };
    //[SWS_CM_11334]
    constexpr ara::core::ErrorDomain const& GetComErrorDomain() noexcept;
    //[SWS_CM_11335]
    constexpr ara::core::ErrorCode MakeErrorCode(ComErrc code, ComErrorDomain::SupportDataType data = 0, char const* message = nullptr) noexcept;

    static ComErrorDomain s_Com_error_domain;
    class ComTmpCalss {
        friend constexpr ara::core::ErrorDomain const& GetComErrorDomain() noexcept {
            return s_Com_error_domain;
        }

        friend constexpr ara::core::ErrorCode MakeErrorCode(ComErrc code, ara::core::ErrorDomain::SupportDataType data, char const* message) noexcept {
            return ara::core::ErrorCode((ara::core::ErrorDomain::CodeType)code, GetComErrorDomain(), data, message);
        }

    };

}
namespace core {
    template <>
    struct is_error_code_enum<ara::com::ComErrc> : std::true_type
    {	// for ErrorCode constructor overload resolution
    };
}
}
#endif