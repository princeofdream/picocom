#ifndef __SPAN_H__
#define __SPAN_H__

#include <stdexcept>
#if _HAS_CXX20
#include <span>
#endif
#include "span.hpp"

namespace ara {
namespace core {
#if _HAS_CXX20
    template <typename T>
    using Span = std::span<T>;
#else
    template <typename T>
    using Span = nonstd::span<T>;
#endif

}
}

#endif