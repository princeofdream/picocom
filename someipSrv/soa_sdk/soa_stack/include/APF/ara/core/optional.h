#ifndef __ARA_CORE_OPTIONAL_H__
#define __ARA_CORE_OPTIONAL_H__
#include <optional>

namespace ara {
    namespace core {
        template <typename T>
        using Optional = std::optional<T>;
    }
}

#endif