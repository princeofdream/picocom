#ifndef __ARA_CORE_VARIANT_H__
#define __ARA_CORE_VARIANT_H__

#include <utility>

#ifdef HAS_CPP17

#include <variant>

namespace ara {
    namespace core {
        template <class... _Types>
        using Variant = std::variant<_Types...>;
    }
}

#else

#include "../../nonstd/variant.hpp"

namespace ara {
    namespace core {
        template <class... _Types>
        using Variant = nonstd::variant<_Types...>;

        template< std::size_t I, class... Types >
        constexpr nonstd::variant_alternative_t<I, nonstd::variant<Types...>>&
            get( nonstd::variant<Types...>& v ) {
                return nonstd::get<I, Types...>(v);
            }

        template< std::size_t I, class... Types >
        constexpr nonstd::variant_alternative_t<I, nonstd::variant<Types...>>&&
            get( nonstd::variant<Types...>&& v ){
                return nonstd::get<I, Types...>(v);
            }

        template< std::size_t I, class... Types >
        constexpr const nonstd::variant_alternative_t<I, nonstd::variant<Types...>>&
            get( const nonstd::variant<Types...>& v ) {
                return nonstd::get<I, Types...>(v);
            }

        template< std::size_t I, class... Types >
        constexpr const nonstd::variant_alternative_t<I, nonstd::variant<Types...>>&&
            get( const nonstd::variant<Types...>&& v ) {
                return nonstd::get<I, Types...>(v);
            }

        template< class T, class... Types >
        constexpr T& get( nonstd::variant<Types...>& v ) {
                return nonstd::get<T, Types...>(v);
            }

        template< class T, class... Types >
        constexpr T&& get( nonstd::variant<Types...>&& v ) {
                return nonstd::get<T, Types...>(v);
            }

        template< class T, class... Types >
        constexpr const T& get( const nonstd::variant<Types...>& v ) {
                return nonstd::get<T, Types...>(v);
            }

        template< class T, class... Types >
        constexpr const T&& get( const nonstd::variant<Types...>&& v ) {
                return nonstd::get<T, Types...>(v);
            }


    }
}


#endif

#endif