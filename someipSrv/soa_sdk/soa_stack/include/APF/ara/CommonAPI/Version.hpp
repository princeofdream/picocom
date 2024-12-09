#ifndef COMMONAPI_VERSION_HPP_
#define COMMONAPI_VERSION_HPP_

#include <cstdint>

namespace CommonAPI {

struct Version {
    Version() = default;
    Version(const uint32_t &majorValue, const uint32_t &minorValue)
        : Major(majorValue), Minor(minorValue) {
    }

    uint32_t Major;
    uint32_t Minor;
};

} // namespace CommonAPI

#endif // COMMONAPI_STRUCT_HPP_
