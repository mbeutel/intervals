
#ifndef INCLUDED_INTERVALS_ALGORITHM_HPP_
#define INCLUDED_INTERVALS_ALGORITHM_HPP_


#include <ranges>       // for random_access_range<>, ssize()
#include <utility>      // for forward<>()
#include <iterator>     // for random_access_iterator<>
#include <algorithm>
#include <functional>   // for less<>
#include <type_traits>  // for declval<>()

#include <gsl-lite/gsl-lite.hpp>

#include <intervals/set.hpp>
#include <intervals/interval.hpp>

#include <intervals/detail/algorithm.hpp>


namespace intervals {

namespace gsl = gsl_lite;


template <std::ranges::range R, typename T, typename CompareT = std::less<>>
[[nodiscard]] constexpr std::ranges::borrowed_iterator_t<R>
lower_bound(R&& range, T const& value, CompareT&& comp = { })
{
    return std::ranges::lower_bound(range, value, std::forward<CompareT>(comp));
}

template <std::ranges::random_access_range R, typename T, typename CompareT = std::less<>>
[[nodiscard]] constexpr interval<std::ranges::borrowed_iterator_t<R>>
lower_bound(R&& range, interval<T> const& value, CompareT&& comp = { })
{
    gsl_Expects(value.assigned());

    auto from = std::ranges::lower_bound(range, value.lower_unchecked(), comp);
    auto to = std::ranges::lower_bound(range, value.upper_unchecked(), comp);
    return interval{ from, to };
}

template <std::ranges::random_access_range R, typename T, typename CompareT = std::less<>>
[[nodiscard]] constexpr std::ranges::borrowed_iterator_t<R>
upper_bound(R&& range, T const& value, CompareT&& comp = { })
{
    return std::ranges::upper_bound(range, value, std::forward<CompareT>(comp));
}

template <std::ranges::random_access_range R, typename T, typename CompareT = std::less<>>
[[nodiscard]] constexpr interval<std::ranges::borrowed_iterator_t<R>>
upper_bound(R&& range, interval<T> const& value, CompareT&& comp = { })
{
    gsl_Expects(value.assigned());

    auto from = std::ranges::upper_bound(range, value.lower_unchecked(), comp);
    auto to = std::ranges::upper_bound(range, value.upper_unchecked(), comp);
    return interval{ from, to };
}


template <typename T>
[[nodiscard]] constexpr detail::single_value_range<T>
enumerate(T value)
{
    return { value };
}
template <typename T>
[[nodiscard]] constexpr detail::set_value_range<T>
enumerate(set<T> const& value)
{
    gsl_Expects(value.assigned());

    return { value };
}
template <typename T>
[[nodiscard]] constexpr detail::interval_value_range<T>
enumerate(interval<T> const& value)
requires detail::integral<T> || std::random_access_iterator<T>
{
    gsl_Expects(value.assigned());

    return { value };
}


template <std::ranges::random_access_range R>
[[nodiscard]] constexpr std::ranges::range_value_t<R>
at(R const& range, gsl::index index)
{
    gsl_Expects(index >= 0 && index < std::ranges::ssize(range));

    return range[index];
}
template <std::ranges::random_access_range R>
[[nodiscard]] constexpr set_of_t<std::ranges::range_value_t<R>>
at(R const& range, interval<gsl::index> const& indexInterval)
{
    using Result = set_of_t<std::ranges::range_value_t<R>>;

    gsl_Expects(indexInterval.assigned());
    gsl_Expects(indexInterval.lower_unchecked() >= 0 && indexInterval.upper_unchecked() < std::ranges::ssize(range));

    auto result = Result{ };
    for (gsl::index index = indexInterval.lower_unchecked(); index <= indexInterval.upper_unchecked(); ++index)
    {
        intervals::assign_partial(result, range[index]);
    }
    return result;
}


} // namespace intervals


#endif // INCLUDED_INTERVALS_ALGORITHM_HPP_
