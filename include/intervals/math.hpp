
#ifndef INCLUDED_INTERVALS_MATH_HPP_
#define INCLUDED_INTERVALS_MATH_HPP_


#include <concepts>  // for floating_point<>

#include <gsl-lite/gsl-lite.hpp>  // for gsl_Expects(), gsl_ExpectsDebug()

#include <intervals/detail/concepts.hpp>


namespace intervals {

namespace gsl = gsl_lite;


    //
    // Wrap  x  into the  [min,max)  interval.
    //
    // This routine is not adequate for arguments vastly larger than the target range. To support these arguments with high
    // accuracy, use an approach that avoids cancellation, e.g. Payne–Hanek (cf. https://stackoverflow.com/a/30465751).
    //
template <std::floating_point T>
[[nodiscard]] constexpr T
wraparound(T x, T min, T max)
{
    gsl_Expects(min < max);

    T range = max - min;
    return min + std::fmod(range + fmod(x - min, range), range);
}


inline namespace math {


template <detail::interval_value X>
[[nodiscard]] constexpr inline X
infimum(X x)
{
    return x;
}
template <detail::interval_value X>
[[nodiscard]] constexpr inline X
supremum(X x)
{
    return x;
}

template <detail::interval_value T>
[[nodiscard]] constexpr inline T
min(T lhs, T rhs)
{
    return rhs < lhs ? rhs : lhs;
}
template <detail::interval_value T>
[[nodiscard]] constexpr inline T
max(T lhs, T rhs)
{
    return lhs < rhs ? rhs : lhs;
}

template <detail::arithmetic T>
[[nodiscard]] constexpr inline T
square(T x)
{
    return x*x;
}
template <detail::arithmetic T>
[[nodiscard]] constexpr inline T
cube(T x)
{
    return x*x*x;
}
template <detail::arithmetic T>
[[nodiscard]] inline T
abs(T x)
{
    return std::abs(x);
}

template <std::floating_point T>
[[nodiscard]] inline T
sqrt(T x)
{
    gsl_ExpectsDebug(x >= 0);

    return std::sqrt(x);
}
template <std::floating_point T>
[[nodiscard]] inline T
cbrt(T x)
{
    return std::cbrt(x);
}
template <std::floating_point T>
[[nodiscard]] inline T
log(T x)
{
    gsl_ExpectsDebug(x >= 0);  // 0 is allowed because floats can represent -∞.

    return std::log(x);
}
template <std::floating_point T>
[[nodiscard]] inline T
exp(T x)
{
    return std::exp(x);
}
template <std::floating_point T>
[[nodiscard]] inline T
pow(T x, T y)
{
    return std::pow(x, y);
}

template <std::floating_point T>
[[nodiscard]] inline T
sin(T x)
{
    return std::sin(x);
}
template <std::floating_point T>
[[nodiscard]] inline T
cos(T x)
{
    return std::cos(x);
}
template <std::floating_point T>
[[nodiscard]] inline T
tan(T x)
{
    return std::tan(x);
}
template <std::floating_point T>
[[nodiscard]] inline T
asin(T x)
{
    gsl_ExpectsDebug(x >= -1 && x <= 1);

    return std::asin(x);
}
template <std::floating_point T>
[[nodiscard]] inline T
acos(T x)
{
    gsl_ExpectsDebug(x >= -1 && x <= 1);

    return std::acos(x);
}
template <std::floating_point T>
[[nodiscard]] inline T
atan(T x)
{
    return std::atan(x);
}
template <std::floating_point T>
[[nodiscard]] inline T
atan2(T y, T x)
{
    gsl_ExpectsDebug(y != 0 || x > 0);

    return std::atan2(y, x);
}

template <std::floating_point T>
[[nodiscard]] inline T
floor(T x)
{
    return std::floor(x);
}
template <std::floating_point T>
[[nodiscard]] inline T
ceil(T x)
{
    return std::ceil(x);
}
template <std::floating_point T>
[[nodiscard]] inline T
round(T x)
{
    return std::round(x);
}
template <std::floating_point T>
[[nodiscard]] inline T
frac(T x)
{
    return x - std::floor(x);
}

template <std::floating_point T>
[[nodiscard]] inline bool
isinf(T x)
{
    return std::isinf(x);
}
template <std::floating_point T>
[[nodiscard]] inline bool
isfinite(T x)
{
    return std::isfinite(x);
}
template <std::floating_point T>
[[nodiscard]] inline bool
isnan(T x)
{
    return std::isnan(x);
}

template <std::random_access_iterator T>
[[nodiscard]] constexpr T
prev(T x)
{
    return std::prev(x);
}
template <std::random_access_iterator T>
[[nodiscard]] constexpr T
next(T x)
{
    return std::next(x);
}

template <std::floating_point T>
[[nodiscard]] constexpr std::pair<T, T>
fractional_weights(T a, T b)
{
    gsl_ExpectsDebug(a >= 0);
    gsl_ExpectsDebug(b >= 0);
    gsl_ExpectsDebug(a + b > 0);

    return { a/(a + b), b/(a + b) };
}

template <std::floating_point T>
constexpr T
blend_linear(T a, T b, T x, T y)
{
    auto [wa, wb] = intervals::fractional_weights(a, b);
    return wa*x + wb*y;
}
template <std::floating_point T>
constexpr T
blend_quadratic(T a, T b, T x, T y)
{
    auto [wa, wb] = intervals::fractional_weights(a, b);
    return std::sqrt(intervals::square(wa*x) + intervals::square(wb*y));
}


template <detail::interval_value T>
requires detail::non_const<T>
constexpr void
assign(T& lhs, gsl::type_identity_t<T> rhs)
{
    lhs = rhs;
}
template <detail::interval_value T>
requires detail::non_const<T>
constexpr void
assign_partial(T& lhs, gsl::type_identity_t<T> rhs)
{
    lhs = rhs;
}
template <detail::interval_value T>
requires detail::non_const<T>
constexpr void
reset(T& lhs, gsl::type_identity_t<T> rhs)
{
    lhs = rhs;
}


template <typename T>
requires std::floating_point<T> || std::integral<T> || std::random_access_iterator<T>
[[nodiscard]] constexpr T
constrain(T x, bool c)
{
    gsl_Assert(c);
    return x;
}


template <detail::interval_value T, detail::interval_value U>
[[nodiscard]] constexpr T
narrow_cast(U const& u) noexcept
{
    return static_cast<T>(u);
}
template <detail::interval_value T, detail::interval_value U>
[[nodiscard]] constexpr T
narrow(U const& u)
{
    return gsl::narrow<T>(u);
}
template <detail::interval_value T, detail::interval_value U>
[[nodiscard]] constexpr T
narrow_failfast(U const& u)
{
    return gsl::narrow_failfast<T>(u);
}


} // inline namespace math

} // namespace intervals


#endif // INCLUDED_INTERVALS_MATH_HPP_
