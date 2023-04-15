
#ifndef INCLUDED_INTERVALS_TYPE_TRAITS_HPP_
#define INCLUDED_INTERVALS_TYPE_TRAITS_HPP_


#include <intervals/concepts.hpp>

#include <intervals/detail/type_traits.hpp>


namespace intervals {


template <any_interval T>
using interval_value_t = typename std::remove_cvref_t<T>::value_type;

template <interval_arg T> struct interval_arg_value : detail::interval_arg_value_0_<std::remove_cvref_t<T>> { };
template <interval_arg T> using interval_arg_value_t = typename interval_arg_value<T>::type;

template <interval_arg T> using interval_of_t = interval<interval_arg_value_t<T>>;

template <any_interval T> using interval_t = typename std::remove_cvref_t<T>::interval_type;

template <interval_arg... Ts>
using common_interval_value_t = std::common_type_t<interval_arg_value_t<Ts>...>;

template <interval_arg... Ts>
using common_interval_t = interval<common_interval_value_t<Ts...>>;


} // namespace intervals


#endif // INCLUDED_INTERVALS_TYPE_TRAITS_HPP_
