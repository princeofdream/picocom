#ifndef __ARA_CORE_MAP_H__
#define __ARA_CORE_MAP_H__
#include <map>

namespace ara {
    namespace core {
        template <class _Kty, class _Ty, class _Pr = std::less<_Kty>, class _Alloc = std::allocator<std::pair<const _Kty, _Ty>>>
        using Map = std::map<_Kty, _Ty, _Pr, _Alloc>;
    }
}

#endif