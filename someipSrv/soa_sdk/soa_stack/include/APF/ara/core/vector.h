#ifndef __ARA_CORE_VECTOR_H__
#define __ARA_CORE_VECTOR_H__
#include <vector>

namespace ara {
    namespace core {
        template <class _Ty, class _Alloc = std::allocator<_Ty>>
        using Vector = std::vector<_Ty, _Alloc>;
    }
}
#endif