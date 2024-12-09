#ifndef __ARA_CORE_ARRAY_H__
#define __ARA_CORE_ARRAY_H__
#include <array>

namespace ara {
    namespace core {
        template <class _Ty, size_t _Size>
        using Array = std::array<_Ty, _Size>;
    }
}

#endif