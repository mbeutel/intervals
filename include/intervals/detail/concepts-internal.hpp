
#ifndef INCLUDED_INTERVALS_DETAIL_CONCEPTS_INTERNAL_HPP_
#define INCLUDED_INTERVALS_DETAIL_CONCEPTS_INTERNAL_HPP_


#include <concepts>     // for convertible_to<>, same_as<>

#include <intervals/concepts.hpp>
#include <intervals/type_traits.hpp>


namespace intervals {

namespace detail {


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

template <typename... Ts>
concept arithmetic_operands =
    (interval_arg<Ts> && ...) && arithmetic_interval_value<common_interval_value_t<Ts...>>;

template <typename... Ts>
concept floating_point_operands =
    (interval_arg<Ts> && ...) && floating_point_interval_value<common_interval_value_t<Ts...>>;


}  // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_CONCEPTS_INTERNAL_HPP_
