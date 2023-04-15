
#ifndef INCLUDED_INTERVALS_CONCEPTS_HPP_
#define INCLUDED_INTERVALS_CONCEPTS_HPP_


#include <concepts>     // for derived_from<>, convertible_to<>, integral<>, floating_point<>, same_as<>
#include <iterator>     // for random_access_iterator<>
#include <type_traits>  // for remove_cvref<>, is_arithmetic<>, common_type<>

#include <intervals/detail/concepts.hpp>


namespace intervals {

namespace gsl = gsl_lite;


template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;


template <typename T>
concept floating_point_interval_value = std::floating_point<std::remove_cvref_t<T>>;
template <typename T>
concept integral_interval_value = std::integral<std::remove_cvref_t<T>>;
template <typename T>
concept arithmetic_interval_value = floating_point_interval_value<T> || integral_interval_value<T>;
template <typename T>
concept iterator_interval_value = detail::not_interval<T> && std::random_access_iterator<std::remove_cvref_t<T>>;
template <typename T>
concept interval_value = arithmetic_interval_value<T> || iterator_interval_value<T>;

template <typename... Ts>
concept any_interval = (std::derived_from<std::remove_cvref_t<Ts>, detail::interval_functions> || ...);
template <typename T>
concept floating_point_interval = any_interval<T> && floating_point_interval_value<typename std::remove_cvref_t<T>::value_type>;
template <typename T>
concept integral_interval = any_interval<T> && integral_interval_value<typename std::remove_cvref_t<T>::value_type>;
template <typename T>
concept arithmetic_interval = any_interval<T> && arithmetic_interval_value<typename std::remove_cvref_t<T>::value_type>;
template <typename T>
concept iterator_interval = any_interval<T> && iterator_interval_value<typename std::remove_cvref_t<T>::value_type>;

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

template <typename... Ts>
concept any_interval_arg = (interval_arg<Ts> || ...);


} // namespace intervals


#endif // INCLUDED_INTERVALS_CONCEPTS_HPP_
