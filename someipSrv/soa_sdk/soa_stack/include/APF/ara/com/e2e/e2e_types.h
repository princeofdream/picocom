#ifndef _ARA_COM_E2E_TYPES_H_
#define _ARA_COM_E2E_TYPES_H_
#include <stdint.h>
namespace ara{
namespace com{
namespace e2e{
    //[SWS_CM_90421]
    enum class ProfileCheckStatus : uint8_t
    {
        Ok,
        Repeated,
        WrongSequence,
        Error,
        NotAvailable,
        NoNewData,
        CheckDisabled
    };
    //[SWS_CM_90422]
    enum class SMState : uint8_t
    {
        Valid,
        NoData,
        Init,
        Invalid,
        StateMDisabled
    };
    //[SWS_CM_90423]
    class Result {
        public:
        ara::com::e2e::ProfileCheckStatus GetCheckStatus() const noexcept;
        ara::com::e2e::SMState GetSMState() const noexcept;
    };
}
}
}

#endif