
#ifndef INCLUDED_INTERVALS_DETAIL_CONCEPTS_HPP_
#define INCLUDED_INTERVALS_DETAIL_CONCEPTS_HPP_


#include <concepts>     // for derived_from<>
#include <type_traits>  // for remove_cvref<>, is_const<>, is_enum<>

#include <makeshift/type_traits.hpp>  // for is_instantiation_of<>


namespace intervals {


template <typename T> class interval;
template <typename T, typename ReflectorT> class set;


namespace detail {


template <typename T>
concept non_const = !std::is_const_v<T>;

template <typename T, template <typename...> class U>
concept not_instantiation_of = !makeshift::is_instantiation_of_v<T, U>;

template <typename T>
concept enum_ = std::is_enum_v<T>;


class interval_functions;


template <typename T>
concept not_interval = !std::derived_from<std::remove_cvref_t<T>, interval_functions>;


}  // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_CONCEPTS_HPP_
