
#ifndef INCLUDED_INTERVALS_DETAIL_CONCEPTS_HPP_
#define INCLUDED_INTERVALS_DETAIL_CONCEPTS_HPP_


#include <concepts>     // for derived_from<>, integral<>, floating_point<>, same_as<>
#include <iterator>     // for random_access_iterator<>
#include <type_traits>  // for remove_cvref<>, common_type<>, is_arithmetic<>, is_const<>, is_enum<>

#include <makeshift/type_traits.hpp>  // for is_instantiation_of<>


namespace intervals {


template <typename T> class interval;


namespace detail {


template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept non_const = !std::is_const_v<T>;

template <typename T, template <typename...> class U>
concept instantiation_of = makeshift::is_instantiation_of_v<T, U>;

template <typename T, template <typename...> class U>
concept not_instantiation_of = !makeshift::is_instantiation_of_v<T, U>;

template <typename T>
concept enum_ = std::is_enum_v<T>;


class interval_functions;

template <typename T>
concept floating_point_interval_value = std::floating_point<std::remove_cvref_t<T>>;
template <typename T>
concept integral_interval_value = std::integral<std::remove_cvref_t<T>>;
template <typename T>
concept arithmetic_interval_value = floating_point_interval_value<T> || integral_interval_value<T>;
template <typename T>
concept iterator_interval_value = std::random_access_iterator<std::remove_cvref_t<T>>;
template <typename T>
concept interval_value = arithmetic_interval_value<T> || iterator_interval_value<T>;

template <typename T>
concept not_interval = !std::derived_from<std::remove_cvref_t<T>, interval_functions>;
template <typename T>
concept any_interval = std::derived_from<std::remove_cvref_t<T>, interval_functions>;
template <any_interval T>
using interval_value_t = typename std::remove_cvref_t<T>::value_type;
template <typename T>
concept floating_point_interval = any_interval<T> && floating_point_interval_value<interval_value_t<T>>;
template <typename T>
concept integral_interval = any_interval<T> && integral_interval_value<interval_value_t<T>>;
template <typename T>
concept arithmetic_interval = any_interval<T> && arithmetic_interval_value<interval_value_t<T>>;
template <typename T>
concept iterator_interval = any_interval<T> && iterator_interval_value<interval_value_t<T>>;

template <typename T>
concept interval_arg = interval_value<T> || any_interval<T>;
template <typename T>
concept floating_point_interval_arg = floating_point_interval_value<T> || floating_point_interval<T>;
template <typename T>
concept integral_interval_arg = integral_interval_value<T> || integral_interval<T>;
template <typename T>
concept arithmetic_interval_arg = arithmetic_interval_value<T> || arithmetic_interval<T>;
template <typename T>
concept iterator_interval_arg = iterator_interval_value<T> || iterator_interval<T>;

template <interval_arg T> struct interval_arg_value_0_;
template <interval_value T> struct interval_arg_value_0_<T> { using type = T; };
template <any_interval IntervalT> struct interval_arg_value_0_<IntervalT> { using type = typename IntervalT::value_type; };
template <interval_arg T> struct interval_arg_value : interval_arg_value_0_<std::remove_cvref_t<T>> { };
template <interval_arg T> using interval_arg_value_t = typename interval_arg_value<T>::type;

template <interval_arg T> using interval_of_t = interval<interval_arg_value_t<T>>;

template <any_interval T> using interval_t = typename std::remove_cvref_t<T>::interval_type;

template <typename L, typename R>
concept relational = requires(L const& lhs, R const& rhs) {
    { lhs < rhs } -> std::convertible_to<bool>;
    { lhs <= rhs } -> std::convertible_to<bool>;
    { lhs > rhs } -> std::convertible_to<bool>;
    { lhs >= rhs } -> std::convertible_to<bool>;
    { lhs == rhs } -> std::convertible_to<bool>;
    { lhs != rhs } -> std::convertible_to<bool>;
};
template <typename L, typename R>
concept relational_values =
    interval_arg<L> &&
    interval_arg<R> &&
    relational<interval_arg_value_t<L>, interval_arg_value_t<R>>;

template <typename L, typename R>
concept same_values =
    interval_arg<L> &&
    interval_arg<R> &&
    std::same_as<interval_arg_value_t<L>, interval_arg_value_t<R>>;

template <interval_arg... Ts>
using common_interval_value_t = std::common_type_t<interval_arg_value_t<Ts>...>;

template <interval_arg... Ts>
using common_interval_t = interval<common_interval_value_t<Ts...>>;

template <typename L, typename R>
concept arithmetic_operands =
    interval_arg<L> &&
    interval_arg<R> &&
    arithmetic_interval_value<common_interval_value_t<L, R>>;

template <typename L, typename R>
concept floating_point_operands =
    interval_arg<L> &&
    interval_arg<R> &&
    floating_point_interval_value<common_interval_value_t<L, R>>;


}  // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_CONCEPTS_HPP_
