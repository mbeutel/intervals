
#ifndef INCLUDED_INTERVALS_DETAIL_ALGORITHM_HPP_
#define INCLUDED_INTERVALS_DETAIL_ALGORITHM_HPP_


#include <array>
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


} // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_ALGORITHM_HPP_
