#ifndef _ARA_INTERNAL_TYPES_H_
#define _ARA_INTERNAL_TYPES_H_
#include <stdint.h>
namespace ara {
namespace com {
    namespace internal {
        using ServiceId = uint16_t;
        using InstanceId = uint16_t;

        using MethodId = uint16_t;
        using SessionId = uint16_t;

        typedef uint8_t major_version_t;
        typedef uint32_t minor_version_t;
    }


    typedef uint8_t byte_t;
}
}
#endif
