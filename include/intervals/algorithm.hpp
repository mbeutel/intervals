
#ifndef INCLUDED_INTERVALS_ALGORITHM_HPP_
#define INCLUDED_INTERVALS_ALGORITHM_HPP_


#include <ranges>       // for random_access_range<>, ssize()
#include <utility>      // for pair<>, forward<>()
#include <concepts>     // for invocable<>
#include <iterator>     // for random_access_iterator<>
#include <algorithm>
#include <functional>   // for less<>
#include <type_traits>  // for declval<>(), invoke_result<>, is_same<>

#include <gsl-lite/gsl-lite.hpp>

#include <intervals/set.hpp>
#include <intervals/interval.hpp>

#include <intervals/detail/algorithm.hpp>


namespace intervals {

namespace gsl = gsl_lite;


template <detail::interval_value T, std::derived_from<detail::partitioning_constraint_base> PartitioningConstraintT>
[[nodiscard]] constexpr T const&
constrain(T const& x, PartitioningConstraintT const& c)
{
    if (!std::ranges::empty(c.range))
    {
        gsl::index ilo = detail::lower(c.index);
        gsl::index ihi = detail::upper(c.index);
        if (ilo > 0)
        {
            gsl_ExpectsDebug(c.predicate(c.range[ilo - 1]));
        }
        if (ihi < std::ranges::ssize(c.range))
        {
            gsl_ExpectsDebug(!c.predicate(c.range[ihi]));
        }
    }
    return x;
}
template <detail::interval_value T, std::derived_from<detail::partitioning_constraint_base> PartitioningConstraintT>
[[nodiscard]] constexpr T const&&
constrain(T const&& x, PartitioningConstraintT const&)
{
    static_assert(makeshift::dependent_false<T>, "rvalue interval cannot be constrained");
    return std::move(x);
}
template <detail::any_interval IntervalT, std::derived_from<detail::partitioning_constraint_base> PartitioningConstraintT>
[[nodiscard]] constexpr detail::as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, PartitioningConstraintT const& c)
{
    using CInterval = detail::as_constrained_interval_t<IntervalT>;
    using UInterval = detail::interval_t<IntervalT>;
    if (!std::ranges::empty(c.range))
    {
        gsl::index ilo = detail::lower(c.index);
        gsl::index ihi = detail::upper(c.index);
        auto xlo = CInterval(x);
        auto xhi = CInterval(x);
        if (ilo > 0)
        {
            xlo.reset(intervals::constrain(xlo, c.predicate(c.range[ilo - 1])));
        }
        if (ihi < std::ranges::ssize(c.range))
        {
            xhi.reset(intervals::constrain(xhi, !c.predicate(c.range[ihi])));
        }

            // Make sure that the two constraints overlap.
        gsl_AssertDebug(detail::upper(xlo) >= detail::lower(xhi) && detail::lower(xlo) <= detail::upper(xhi));

        return CInterval::make_with_identity(x, UInterval{ std::max(detail::lower(xlo), detail::lower(xhi)), std::min(detail::upper(xlo), detail::upper(xhi)) });
    }
    return x;
}
template <detail::any_interval IntervalT, std::derived_from<detail::partitioning_constraint_base> PartitioningConstraintT>
[[nodiscard]] constexpr IntervalT const&&
constrain(IntervalT const&& x, PartitioningConstraintT const&)
{
    static_assert(makeshift::dependent_false<IntervalT>, "rvalue interval cannot be constrained");
    return std::move(x);
}

template <std::ranges::random_access_range R, std::invocable<std::ranges::range_value_t<R> const&> PredicateT>
[[nodiscard]] constexpr auto
partition_point(R&& range, PredicateT&& predicate)
{
    using Value = std::ranges::range_value_t<R>;
    using PredVal = std::invoke_result_t<PredicateT, Value const&>;
    if constexpr (std::is_convertible_v<PredVal, bool>)
    {
        auto pos = std::ranges::partition_point(range, predicate);
        return std::pair{
            detail::partitioning<R, PredicateT>{ std::forward<R>(range), std::forward<PredicateT>(predicate) },
            pos
        };
    }
    else if constexpr (std::is_convertible_v<PredVal, set<bool>>)
    {
        auto pos = interval{
            std::ranges::partition_point(range,
                [&predicate](Value const& value)
                {
                    return intervals::always(predicate(value));
                }),
            std::ranges::partition_point(range,
                [&predicate](Value const& value)
                {
                    return intervals::possibly(predicate(value));
                })
        };
        return std::pair{
            detail::partitioning<R, PredicateT>{ std::forward<R>(range), std::forward<PredicateT>(predicate) },
            pos
        };
    }
}

template <std::ranges::random_access_range R, detail::interval_arg T, typename CompareT = std::less<>>
[[nodiscard]] constexpr auto
lower_bound(R&& range, T const& value, CompareT&& comp = { })
{
    return intervals::partition_point(std::forward<R>(range),
        [&value, comp = std::forward<CompareT>(comp)]
        (auto const& element)
        {
            return comp(element, value);
        });
}
template <std::ranges::random_access_range R, detail::interval_arg T, typename CompareT = std::less<>>
[[nodiscard]] constexpr auto
lower_bound(R&& range, T const&& value, CompareT&& comp = { })
{
    return intervals::partition_point(std::forward<R>(range),
        [value, comp = std::forward<CompareT>(comp)]
        (auto const& element)
        {
            return comp(element, T(value));
        });
}
template <std::ranges::random_access_range R, detail::interval_arg T, typename CompareT = std::less<>>
[[nodiscard]] constexpr auto
upper_bound(R&& range, T const& value, CompareT&& comp = { })
{
    return intervals::partition_point(std::forward<R>(range),
        [&value, comp = std::forward<CompareT>(comp)]
        (auto const& element)
        {
            return !comp(value, element);
        });
}
template <std::ranges::random_access_range R, detail::interval_arg T, typename CompareT = std::less<>>
[[nodiscard]] constexpr auto
upper_bound(R&& range, T const&& value, CompareT&& comp = { })
{
    return intervals::partition_point(std::forward<R>(range),
        [value, comp = std::forward<CompareT>(comp)]
        (auto const& element)
        {
            return !comp(T(value), element);
        });
}


template <typename T>
requires detail::not_interval<T> && detail::not_instantiation_of<T, set>
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
enumerate(detail::interval_base<T> const& value)
requires std::integral<T> || std::random_access_iterator<T>
{
    gsl_Expects(value.assigned());

    return { interval<T>(value) };
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
