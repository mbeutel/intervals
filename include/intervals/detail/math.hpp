
#ifndef INCLUDED_INTERVALS_DETAIL_MATH_HPP_
#define INCLUDED_INTERVALS_DETAIL_MATH_HPP_


#include <type_traits>  // for is_arithmetic<>, is_floating_point<>, is_enum<>

#include <makeshift/tuple.hpp>  // for template_for()


namespace intervals {

namespace detail {


template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

template <typename T>
concept non_const = !std::is_const_v<T>;

template <typename T>
concept enum_ = std::is_enum_v<T>;


template <typename T>
class assigner
{
private:
    T& ref_;

public:
    explicit constexpr assigner(T& _ref) noexcept
        : ref_(_ref)
    {
    }
    operator T&() const && noexcept
    {
        return ref_;
    }
    constexpr T&
    operator =(T value) && noexcept
    {
        ref_ = value;
        return ref_;
    }
};

template <typename T>
class member_assigner
{
private:
    T& ref_;

public:
    explicit constexpr member_assigner(T& _ref) noexcept
        : ref_(_ref)
    {
    }
    operator T&() const && noexcept
    {
        return ref_;
    }
    template <typename U>
    constexpr T&
    operator =(U&& value) && noexcept
    {
        ref_.assign(std::forward<U>(value));
        return ref_;
    }
};

template <typename T>
class member_resetter
{
private:
    T& ref_;

public:
    explicit constexpr member_resetter(T& _ref) noexcept
        : ref_(_ref)
    {
    }
    operator T&() const && noexcept
    {
        return ref_;
    }
    template <typename U>
    constexpr T&
    operator =(U&& value) && noexcept
    {
        ref_.reset(std::forward<U>(value));
        return ref_;
    }
};
template <typename T>
class tuple_assigner
{
private:
    T& ref_;

public:
    explicit constexpr tuple_assigner(T& _ref) noexcept
        : ref_(_ref)
    {
    }
    operator T&() const && noexcept
    {
        return ref_;
    }
    template <typename U>
    constexpr T&
    operator =(U&& value) && noexcept
    {
        makeshift::template_for(
            [](auto& lhs, auto&& rhs)
            {
                maybe(lhs) = std::forward<decltype(rhs)>(rhs);
            },
            ref_, std::forward<U>(value));
        return ref_;
    }
};
template <typename T>
class tuple_resetter
{
private:
    T& ref_;

public:
    explicit constexpr tuple_resetter(T& _ref) noexcept
        : ref_(_ref)
    {
    }
    operator T&() const && noexcept
    {
        return ref_;
    }
    template <typename U>
    constexpr T&
    operator =(U&& value) && noexcept
    {
        makeshift::template_for(
            [](auto& lhs, auto&& rhs)
            {
                definitely(lhs) = std::forward<decltype(rhs)>(rhs);
            },
            ref_, std::forward<U>(value));
        return ref_;
    }
};



}  // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_MATH_HPP_
