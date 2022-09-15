
#ifndef INCLUDED_INTERVALS_DETAIL_MATH_HPP_
#define INCLUDED_INTERVALS_DETAIL_MATH_HPP_


#include <type_traits>  // for is_arithmetic<>, is_floating_point<>, is_integral<>, is_enum<>

#include <makeshift/tuple.hpp>        // for template_for()
#include <makeshift/type_traits.hpp>  // for is_instantiation_of<>


namespace intervals {

namespace detail {


template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept non_const = !std::is_const_v<T>;

template <typename T, template <typename...> class U>
concept instantiation_of = makeshift::is_instantiation_of_v<T, U>;

template <typename T, template <typename...> class U>
concept not_instantiation_of = !makeshift::is_instantiation_of_v<T, U>;

template <typename T>
concept enum_ = std::is_enum_v<T>;


}  // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_MATH_HPP_
