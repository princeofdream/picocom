#ifndef _APF_UTILITY_H_
#define _APF_UTILITY_H_

#include "ara/core/string_view.h"
#include <array>
#include <utility>

namespace apf {
namespace utility {

template <typename U, typename T>
U safeIntCast(T _value) {
    //ENSURE(_value < static_cast<T>((std::numeric_limits<U>::max)()))(_value);

    return static_cast<U>(_value);
}


template <size_t N>
size_t copyToCharArray(char(&_ary)[N], ara::core::StringView _str) {
    memset(_ary, 0x00U, N);
    auto end_pos = (std::min)(_str.size(), N - 1);
    std::copy(_str.begin(), _str.begin() + end_pos, std::begin(_ary));

    return end_pos;
}

#if 0
// todo span version
size_t copyToCharArray(ara::core::Span<char> _dest, nonara::core::String_view _sour) {
    std::fill_n(_dest.begin(), 0x00U, N);
    auto end_pos = (std::min)(_str.size(), N - 1);
    std::copy(_str.begin(), _str.begin() + end_pos, std::begin(_ary));

    return end_pos;
}
#endif

namespace detail {
 
template <class T, std::size_t N, std::size_t... I>
constexpr std::array<std::remove_cv_t<T>, N>
    to_array_impl(T (&a)[N], std::index_sequence<I...>)
{
    return { {a[I]...} };
}

template <class T, std::size_t N, std::size_t... I>
constexpr std::array<std::remove_cv_t<T>, N>
    to_array_impl(T (&&a)[N], std::index_sequence<I...>)
{
    return { {std::move(a[I])...} };
}
 
}
 
template <class T, std::size_t N>
constexpr std::array<std::remove_cv_t<T>, N> to_array(T (&a)[N])
{
    return detail::to_array_impl(a, std::make_index_sequence<N>{});
}

template <class T, std::size_t N>
constexpr std::array<std::remove_cv_t<T>, N> to_array(T (&&a)[N])
{
    return detail::to_array_impl(std::move(a), std::make_index_sequence<N>{});
}

template< class Enum >
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(e);
}

} // namespace utility
} // namespace apf
#endif
