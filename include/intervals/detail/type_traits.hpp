
#ifndef INCLUDED_INTERVALS_DETAIL_TYPE_TRAITS_HPP_
#define INCLUDED_INTERVALS_DETAIL_TYPE_TRAITS_HPP_


#include <intervals/concepts.hpp>


namespace intervals {

namespace detail {


template <interval_arg T> struct interval_arg_value_0_;
template <interval_value T> struct interval_arg_value_0_<T> { using type = T; };
template <any_interval IntervalT> struct interval_arg_value_0_<IntervalT> { using type = typename IntervalT::value_type; };


}  // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_TYPE_TRAITS_HPP_
