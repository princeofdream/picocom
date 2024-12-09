#ifndef __ARA_CORE_STRING_H__
#define __ARA_CORE_STRING_H__
#include <string>

namespace ara {
    namespace core {
        template <class _Elem, class _Traits = std::char_traits<_Elem>, class _Alloc = std::allocator<_Elem>>
        using BasicString = std::basic_string<_Elem, _Traits, _Alloc>;

        using String = BasicString<char>;
        using u16String = BasicString<char16_t>;
    }
}

#endif
