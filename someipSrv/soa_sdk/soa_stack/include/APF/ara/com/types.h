#ifndef _ARA_COM_TYPES_H_
#define _ARA_COM_TYPES_H_
#include <functional>
#include "ara/core/vector.h"
#include "ara/core/string_view.h"
#include "ara/core/instance_specifier.h"
#if nssv_CPP17_OR_GREATER
#include "ara/core/result.h"
#else
#include "ara/core/result1.h"
#endif
#include "ara/core/future.h"
#include "ara/core/promise.h"
#include "ara/com/internal.h"
#include "ara/com/sample_ptr.h"
#include <array>
#include "AddressTranslator.h"
#include "ara/core/exception.h"
#include "ara/core/error_code.h"
#include "com_error_domain.h"
#include "runtime/runtime.h"
#include "instance_identifyer.h"

#if  __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#  define COMMONAPI_DEPRECATED __attribute__ ((__deprecated__))
#elif defined(_MSC_VER) && (_MSC_VER >= 1300)
#  define COMMONAPI_DEPRECATED __declspec(deprecated)
#else
#  define COMMONAPI_DEPRECATED
#endif


#ifdef WIN32
#define CCALL __cdecl
#pragma section(".CRT$XCU",read)
#define INITIALIZER(f) \
    static void __cdecl f(void); \
    __declspec(allocate(".CRT$XCU")) void(__cdecl*f##_)(void) = f; \
    static void __cdecl f(void)
#else
#define CCALL
#define INITIALIZER(f) \
    static void f(void) __attribute__((constructor)); \
    static void f(void)
#endif

#define SERVICE_VERSION_MAX_LENGTH 128
#define SERVICE_IDENTIFIER_MAX_LENGTH 256
namespace ara{
namespace com{
    struct FindServiceHandle {
        internal::ServiceId service_id;
        internal::InstanceId instance_id;
        std::uint32_t uid;
        bool operator==(const FindServiceHandle& other) const {
            return (service_id == other.service_id && instance_id == other.instance_id);
        }
        bool operator<(const FindServiceHandle& other) const {
            if (service_id < other.service_id)
            {
                return true;
            }
            else if (service_id == other.service_id && instance_id < other.instance_id)
            {
                return true;
            }
            return false;
        }
        FindServiceHandle& operator=(const FindServiceHandle& other) {
            service_id = other.service_id;
            instance_id = other.instance_id;
            return *this;
        }

    };
    
    template <typename T>
    using ServiceHandleContainer = ara::core::Vector<T>;
    
    template <typename T>
    using FindServiceHandler = std::function<void(ServiceHandleContainer<T>, FindServiceHandle)>;

    //[SWS_CM_00308]
    template <typename T>
    using SampleAllocateePtr = std::unique_ptr<T>;
    //[SWS_CM_00309]
    using EventReceiveHandler = std::function<void()>;
    //[SWS_CM_00310]
    enum class SubscriptionState : uint8_t {
        kSubscribed,
        kNotSubscribed,
        kSubscriptionPending
    };
    //[SWS_CM_00311]
    using SubscriptionStateChangeHandler = std::function<void(SubscriptionState)>;
    //[SWS_CM_00301]
    enum class MethodCallProcessingMode : uint8_t {
        kPoll,
        kEvent,
        kEventSingleThread
    };

    class ServiceIdentifierType {
    public:
        constexpr ServiceIdentifierType(std::array<char, SERVICE_IDENTIFIER_MAX_LENGTH> _id) : m_identify(_id) {

        }
        bool operator==(const ServiceIdentifierType& other) const {
            if (m_identify == other.m_identify)
            {
                return true;
            }
            return false;
        }
        bool operator<(const ServiceIdentifierType& other) const {
            if (m_identify < other.m_identify)
            {
                return true;
            }
            return false;
        }
        ServiceIdentifierType& operator=(const ServiceIdentifierType& other) {
            m_identify = other.m_identify;
            return *this;
        }
        ara::core::StringView toString() const {
            return ara::core::StringView(m_identify.data());
        }
    private:
        std::array<char, SERVICE_IDENTIFIER_MAX_LENGTH> m_identify;
    };

    class ServiceVersionType {
    public:
        constexpr ServiceVersionType(std::array<char, SERVICE_VERSION_MAX_LENGTH> _version) : m_version(_version) {

        }
        bool operator==(const ServiceVersionType& other) const {
            if (m_version == other.m_version)
            {
                return true;
            }
            return false;
        }
        bool operator<(const ServiceVersionType& other) const {
            if (m_version < other.m_version)
            {
                return true;
            }
            return false;
        }
        ServiceVersionType& operator=(const ServiceVersionType& other) {
            m_version = other.m_version;
            return *this;
        }
        ara::core::StringView toString() const {
            return ara::core::StringView(m_version.data());
        }
    private:
        std::array<char, SERVICE_VERSION_MAX_LENGTH> m_version;
    };
}
}
#endif
