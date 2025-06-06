
#ifndef INCLUDED_INTERVALS_TYPE_TRAITS_HPP_
#define INCLUDED_INTERVALS_TYPE_TRAITS_HPP_


#include <makeshift/metadata.hpp>  // for reflector

#include <intervals/concepts.hpp>

#include <intervals/detail/type_traits.hpp>


namespace intervals {


template <typename T, typename ReflectorT>
class set;


template <any_interval T>
using interval_value_t = typename std::remove_cvref_t<T>::value_type;

template <interval_arg T> struct interval_arg_value : detail::interval_arg_value_0_<std::remove_cvref_t<T>> { };
template <interval_arg T> using interval_arg_value_t = typename interval_arg_value<T>::type;

template <interval_arg T> using interval_of_t = interval<interval_arg_value_t<T>>;
template <interval_arg T>
struct interval_of
{
    using type = interval_of<T>;
};

template <any_interval T> using interval_t = typename std::remove_cvref_t<T>::interval_type;

template <interval_arg... Ts>
struct common_interval_value : std::common_type<interval_arg_value_t<Ts>...> { };
template <interval_arg... Ts>
using common_interval_value_t = typename common_interval_value<Ts...>::type;

template <interval_arg... Ts>
using common_interval_t = interval<common_interval_value_t<Ts...>>;
template <interval_arg... Ts>
struct common_interval
{
    using type = common_interval_t<Ts...>;
};


template <typename T, typename ReflectorT = makeshift::reflector> struct set_of;
template <typename ReflectorT> struct set_of<bool, ReflectorT> { using type = set<bool, ReflectorT>; };
template <detail::enum_ T, typename ReflectorT> struct set_of<T, ReflectorT> { using type = set<T, ReflectorT>; };
template <std::floating_point T, typename ReflectorT> struct set_of<T, ReflectorT> { using type = interval<T>; };
template <std::integral T, typename ReflectorT> struct set_of<T, ReflectorT> { using type = interval<T>; };
template <std::random_access_iterator T, typename ReflectorT> struct set_of<T, ReflectorT> { using type = interval<T>; };
template <typename T, typename ReflectorT> struct set_of<set<T, ReflectorT>> { using type = set<T, ReflectorT>; };
template <any_interval IntervalT, typename ReflectorT> struct set_of<IntervalT, ReflectorT> { using type = interval<typename IntervalT::value_type>; };
template <typename T, typename ReflectorT = makeshift::reflector> using set_of_t = typename set_of<T>::type;

template <typename S, typename T, typename ReflectorT = makeshift::reflector> struct propagate_set { using type = T; };
template <any_interval IntervalT, typename T, typename ReflectorT> struct propagate_set<IntervalT, T, ReflectorT> { using type = set_of_t<T, ReflectorT>; };
template <typename U, typename T, typename OldReflectorT, typename ReflectorT> struct propagate_set<set<U, OldReflectorT>, T, ReflectorT> { using type = set_of_t<T, ReflectorT>; };
template <typename S, typename T, typename ReflectorT = makeshift::reflector> using propagate_set_t = typename propagate_set<S, T, ReflectorT>::type;


} // namespace intervals


#endif // INCLUDED_INTERVALS_TYPE_TRAITS_HPP_
