
#ifndef INCLUDED_INTERVALS_DETAIL_ALGORITHM_HPP_
#define INCLUDED_INTERVALS_DETAIL_ALGORITHM_HPP_


#include <array>
#include <ranges>       // for random_access_range
#include <iterator>     // for random_access_iterator
#include <type_traits>  // for is_same<>

#include <makeshift/metadata.hpp>  // for metadata::values()

#include <intervals/set.hpp>
#include <intervals/interval.hpp>
#include <intervals/utility.hpp>


namespace intervals {

namespace detail {


struct sentinel { };

template <typename T>
struct single_value_iterator
{
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T const*;
    using reference = T const&;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;

    T value = { };
    bool exhausted = true;

    [[nodiscard]] friend constexpr bool
    operator ==(sentinel, single_value_iterator const& it)
    {
        return it.exhausted;
    }
    [[nodiscard]] friend constexpr bool
    operator ==(single_value_iterator const& it, sentinel)
    {
        return it.exhausted;
    }
    constexpr single_value_iterator&
    operator ++()
    {
        gsl_Expects(!exhausted);

        exhausted = true;
        return *this;
    }
    constexpr single_value_iterator
    operator ++(int)
    {
        gsl_Expects(!exhausted);

        exhausted = true;
        return { value, false };
    }
    [[nodiscard]] constexpr T const&
    operator *() const
    {
        gsl_Expects(!exhausted);

        return value;
    }
};
template <typename T>
struct single_value_range
{
    T value;

    [[nodiscard]] constexpr single_value_iterator<T>
    begin() const
    {
        return { .value = value, .exhausted = false };
    }
    [[nodiscard]] constexpr sentinel
    end() const
    {
        return { };
    }
};

template <typename T>
struct set_value_iterator
{
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T const*;
    using reference = T const&;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;

    as_regular<set<T>> value = { };
    gsl::index index = -1;
    bool exhausted = true;

    [[nodiscard]] friend constexpr bool
    operator ==(sentinel, set_value_iterator const& it)
    {
        return it.exhausted;
    }
    [[nodiscard]] friend constexpr bool
    operator ==(set_value_iterator const& it, sentinel)
    {
        return it.exhausted;
    }
    constexpr set_value_iterator&
    operator ++()
    {
        gsl_Expects(!exhausted);

        while (++index != gsl::ssize(set<T>::values))
        {
            if (value().contains_index(index))
            {
                return *this;
            }
        }
        exhausted = true;
        return *this;
    }
    constexpr set_value_iterator
    operator ++(int)
    {
        auto result = *this;
        ++*this;
        return result;
    }
    [[nodiscard]] constexpr T
    operator *() const
    {
        gsl_Expects(!exhausted);

        return set<T>::values[index];
    }
};
template <typename T>
struct set_value_range
{
    as_regular<set<T>> value;

    [[nodiscard]] constexpr set_value_iterator<T>
    begin() const
    {
        auto result = set_value_iterator<T>{ .value = value, .index = -1, .exhausted = false };
        ++result;
        return result;
    }
    [[nodiscard]] constexpr sentinel
    end() const
    {
        return { };
    }
};

template <typename T>
struct interval_value_iterator
{
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T const*;
    using reference = T const&;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;

    T pos = { };
    T upper = { };
    bool exhausted = true;

    [[nodiscard]] friend constexpr bool
    operator ==(sentinel, interval_value_iterator const& it)
    {
        return it.exhausted;
    }
    [[nodiscard]] friend constexpr bool
    operator ==(interval_value_iterator const& it, sentinel)
    {
        return it.exhausted;
    }
    constexpr interval_value_iterator&
    operator ++()
    {
        gsl_Expects(!exhausted);

        if (pos == upper)
        {
            exhausted = true;
        }
        else
        {
            ++pos;
        }
        return *this;
    }
    constexpr interval_value_iterator
    operator ++(int)
    {
        auto result = *this;
        ++*this;
        return result;
    }
    [[nodiscard]] constexpr T
    operator *() const
    {
        gsl_Expects(!exhausted);

        return pos;
    }
};
template <typename T>
struct interval_value_range
{
    as_regular<interval<T>> value;

    [[nodiscard]] constexpr interval_value_iterator<T>
    begin() const
    {
        return { .pos = value().lower_unchecked(), .upper = value().upper_unchecked(), .exhausted = false };
    }
    [[nodiscard]] constexpr sentinel
    end() const
    {
        return { };
    }
};


struct partitioning_constraint_base { };

template <std::ranges::random_access_range R, std::invocable<std::ranges::range_value_t<R> const&> PredicateT>
requires std::ranges::borrowed_range<R>
struct partitioning_constraint : partitioning_constraint_base
{
    R range;
    PredicateT predicate;
    gsl::index index;
};
template <std::ranges::random_access_range R, std::invocable<std::ranges::range_value_t<R> const&> PredicateT>
requires std::ranges::borrowed_range<R>
struct partitioning_interval_constraint : partitioning_constraint_base
{
    R range;
    PredicateT predicate;
    interval<gsl::index> index;
};

template <std::ranges::random_access_range R, std::invocable<std::ranges::range_value_t<R> const&> PredicateT>
requires std::ranges::borrowed_range<R>
class partitioning
{
private:
    R range_;
    PredicateT predicate_;

public:
    constexpr partitioning(R&& _range, PredicateT&& _predicate)
        : range_(std::forward<R>(_range)), predicate_(std::forward<PredicateT>(_predicate))
    {
    }
    constexpr partitioning_constraint<R, PredicateT>
    operator [](std::ranges::borrowed_iterator_t<R> it)
    {
        return { range_, predicate_, it - std::ranges::begin(range_) };
    }
    constexpr partitioning_interval_constraint<R, PredicateT>
    operator [](interval<std::ranges::borrowed_iterator_t<R>> const& its)
    {
        return { range_, predicate_, its - std::ranges::begin(range_) };
    }
    constexpr partitioning_constraint<R, PredicateT>
    operator [](gsl::index index)
    {
        gsl_ExpectsDebug(index >= 0 && index <= std::ranges::ssize(range_));

        return { { }, range_, predicate_, index };
    }
    constexpr partitioning_interval_constraint<R, PredicateT>
    operator [](interval_base<gsl::index> const& index)
    {
        gsl_Expects(index.assigned());
        gsl_ExpectsDebug(index.lower_unchecked() >= 0 && index.upper_unchecked() <= std::ranges::ssize(range_));

        return { { }, range_, predicate_, index };
    }
};


} // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_ALGORITHM_HPP_
