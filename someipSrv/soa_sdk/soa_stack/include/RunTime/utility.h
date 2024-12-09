#ifndef __ARA_CORE_UTILITY_H__
#define __ARA_CORE_UTILITY_H__
/******************************************************************************
 **    INCLUDES
 ******************************************************************************/

#include <initializer_list>
#include <type_traits>
#include "byte.hpp"
/******************************************************************************
 **    MACROS
 ******************************************************************************/


/******************************************************************************
 **    TYPE DEFINITIONS
 ******************************************************************************/
namespace ara
{
namespace core
{

/*
    [R20-11][SWS_CORE_04200] A non-integral binary type.
*/
using Byte = nonstd::byte;
/******************************************************************************
 **    CLASSES/FUNCTIONS DEFINITIONS
 ******************************************************************************/

/**
*  @brief   Denote an operation to be performed in-place.
*  An instance of this type can be passed to certain constructors of ara::core::Optional to denote
*  the intention that construction of the contained type shall be done in-place, i.e. without any
*  copying taking place.
*  @param   [IN]
*  @return  None
*  @note    
*  @version [R20-11]
*  @remarks SWS_CORE_04011
*/
struct in_place_t 
{
    /**
    *  @brief  Default constructor.
    *  @param   [IN]
    *  @return  None
    *  @note    
    *  @version [R20-11]
    *  @remarks SWS_CORE_04012
    */
    explicit in_place_t() = default;
};

/** The singleton instance of in_place_t */
// #if __cplusplus > 201402L
// inline constexpr in_place_t in_place;
// #else
/**
*  @brief  The singleton instance of in_place_t.
*  @param   [IN]
*  @return  None
*  @note    
*  @version [R20-11]
*  @remarks SWS_CORE_04013
*/
constexpr in_place_t in_place;
// #endif


/**
*  @brief  Denote a type-distinguishing operation to be performed in-place.
*  @param   [IN]
*  @return  None
*  @note    An instance of this type can be passed to certain constructors of ara::core::Variant to denote
*  the intention that construction of the contained type shall be done in-place, i.e. without any
*  copying taking place
*  @version [R20-11]
*  @remarks SWS_CORE_04021
*/
template <typename T>
struct in_place_type_t
{
    /**
    *  @brief  Default constructor.
    *  @param   [IN]
    *  @return  None
    *  @note    
    *  @version [R20-11]
    *  @remarks SWS_CORE_04022
    */
    explicit in_place_type_t ()=default;
};

/**
*  @brief  The singleton instances (one for each T) of in_place_type_t.
*  @param   [IN]
*  @return  None
*  @note    
*  @version [R20-11]
*  @remarks SWS_CORE_04023
*/
template <typename T>
constexpr in_place_type_t<T> in_place_type;


/**
*  @brief  Denote an index-distinguishing operation to be performed in-place.
*  @param   [IN]
*  @return  None
*  @note    
*  @version [R20-11]
*  @remarks SWS_CORE_04031
*/
template <size_t I>
struct in_place_index_t
{
    /**
    *  @brief  Default constructor.
    *  @param   [IN]
    *  @return  None
    *  @note    
    *  @version [R20-11]
    *  @remarks SWS_CORE_04032
    */
    explicit in_place_index_t() = default;
};

/**
*  @brief  The singleton instances (one for each I) of in_place_index_t.
*  @param   [IN]
*  @return  None
*  @note    
*  @version [R20-11]
*  @remarks SWS_CORE_04033
*/
template <std::size_t I>
constexpr in_place_index_t<I> in_place_index {};

/** template conjunction and disjunction only c++17 defined, so here as workaround */
#if __cplusplus < 201703L

/**for disjunction */
template <typename...>
struct Disjunction : std::false_type {};

template <typename B1>
struct Disjunction<B1> : B1 {};

template <typename B1, typename B2>
struct Disjunction<B1, B2> : std::conditional<B1::value, B1, B2>::type {};

template <typename B1, typename... Bs>
struct Disjunction<B1, Bs...> : std::conditional<B1::value, B1, Disjunction<Bs...>>::type {};

/**for conjunction */
// STRUCT TEMPLATE conjunction
template <bool _First_value, class _First, class... _Rest>
struct _Conjunction_l
{ // handle false trait or last trait
    using type = _First;
};

template <class _True, class _Next, class... _Rest>
struct _Conjunction_l<true, _True, _Next, _Rest...>
{ // the first trait is true, try the next one
    using type = typename _Conjunction_l<_Next::value, _Next, _Rest...>::type;
};

template <class... _Traits>
struct Conjunction : std::true_type {}; // If _Traits is empty, true_type

template <class _First, class... _Rest>
struct Conjunction<_First, _Rest...> : _Conjunction_l<_First::value, _First, _Rest...>::type
{
    // the first false trait in _Traits, or the last trait if none are false
};

#else
template <typename... Args>
using Disjunction = std::disjunction<Args...>;

template <class... _Traits>
using Conjunction = std::conjunction<_Traits...>;
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**These global functions allow uniform access to the data and size 
 * properties of contiguous containers. 
 */
/**
*  @brief  Return a pointer to the block of memory that contains the elements of a container.
*  @param   [IN] an instance of Container
*  @return  decltype(c.data())  a pointer to the first element of the container
*  @note    
*  @version [R20-11]
*  @remarks SWS_CORE_04110
*/
template <typename Container>
constexpr auto data(Container& c) -> decltype(c.data())
{
    return c.data();
}

/**
*  @brief  Return a const_pointer to the block of memory that contains the elements of a container.
*  @param   [IN] a type with a data() method
*  @return  decltype(c.data()) a pointer to the first element of the container
*  @note    
*  @version [R20-11]
*  @remarks SWS_CORE_04111
*/
template <typename Container>
constexpr auto data(Container const& c) -> decltype(c.data())
{
    return c.data();
}

/**
*  @brief  Return a pointer to the block of memory that contains the elements of a raw array.
*  @param   [IN] reference to a raw arra
*  @return  T *  a pointer to the first element of the array
*  @note    
*  @version [R20-11]
*  @remarks SWS_CORE_04112
*/
template <typename T, std::size_t N>
constexpr T* data(T(&array)[N]) noexcept
{
    // get pointer to data of array, todo, is this right??
    return (array);
}

/**
*  @brief  Return a pointer to the block of memory that contains the elements of a std::initializer_list.
*  @param   [IN] the std::initializer_list
*  @return  const E *  a pointer to the first element of the std::initializer_list
*  @note    
*  @version [R20-11]
*  @remarks SWS_CORE_04113
*/
template <typename E>
constexpr E const* data(std::initializer_list< E > il) noexcept
{
    // get pointer to data of initializer_list
    return (il.begin());
}

/**
*  @brief  Return the size of a container.
*  @param   [IN] an instance of Container
*  @return  decltype(c.size())  the size of the container
*  @note    
*  @version [R20-11]
*  @remarks SWS_CORE_04120
*/
template <typename Container>
constexpr auto size(Container const& c) -> decltype(c.size())
{
    return c.size();
}

/**
*  @brief  Return the size of a raw array.
*  @param   [IN] reference to a raw array
*  @return  std::size_t  the size of the array, i.e. N
*  @note    
*  @version [R20-11]
*  @remarks SWS_CORE_04121
*/
template <typename T, std::size_t N>
constexpr std::size_t size(T const (&array)[N]) noexcept
{
    return N;
}

/**
*  @brief  Return whether the given container is empty.
*  @param   [IN] an instance of Container
*  @return  decltype(c.empty())  true if the container is empty, false otherwise
*  @note    
*  @version [R20-11]
*  @remarks SWS_CORE_04130
*/
template <typename Container>
constexpr auto empty(Container const& c) -> decltype(c.empty())
{
    return c.empty();
}

/**
*  @brief  Return whether the given raw array is empty.
*  @param   [IN] the raw array
*  @return  bool  false
*  @note    As raw arrays cannot have zero elements in C++, this function always returns false.
*  @version [R20-11]
*  @remarks SWS_CORE_04131
*/
template <typename T, std::size_t N>
constexpr bool empty(T const (&array)[N]) noexcept
{
    return false;
}

/**
*  @brief  Return whether the given std::initializer_list is empty.
*  @param   [IN] the std::initializer_list
*  @return  bool   true if the std::initializer_list is empty, false otherwise
*  @note    
*  @version [R20-11]
*  @remarks SWS_CORE_04132
*/
template <typename E>
constexpr bool empty(std::initializer_list< E > il) noexcept
{
    // get dimension==0 for initializer_list
    return (il.size() == 0);
}

namespace internal_impl
{
    /** to check is a spec of a template */
    template<template<class...> class Target, class T>
    struct is_template_of
    {
        static const bool value = false;
    };

    template<template<class...> class Target, class...Args>
    struct is_template_of<Target, Target<Args...>>
    {
        static const bool value = true;
    };
} /** namespace internal_impl */


} /** namespace core */
} /** namespace ara */

#endif /** !__ARA_CORE_UTILITY_H__ */

