
#ifndef INCLUDED_INTERVALS_MATH_HPP_
#define INCLUDED_INTERVALS_MATH_HPP_


#include <gsl-lite/gsl-lite.hpp>  // for gsl_ExpectsDebug()

#include <intervals/detail/math.hpp>


namespace intervals {

namespace gsl = gsl_lite;


    // Essential functions
template <typename T>
[[nodiscard]] constexpr T
square(T x)
{
    return x*x;
}
template <typename T>
[[nodiscard]] constexpr T
cube(T x)
{
    return x*x*x;
}

template <detail::floating_point T>
[[nodiscard]] inline T
sqrt(T x)
{
    gsl_ExpectsDebug(x >= 0);

    return std::sqrt(x);
}

template <detail::floating_point T>
[[nodiscard]] inline T
log(T x)
{
    gsl_ExpectsDebug(x >= 0);  // 0 is allowed because floats can represent  -∞ .

    return std::log(x);
}

template <detail::floating_point T>
[[nodiscard]] inline float
asin(float x)
{
    gsl_ExpectsDebug(x >= -1 && x <= 1);

    return std::asin(x);
}

template <detail::floating_point T>
[[nodiscard]] inline T
acos(T x)
{
    gsl_ExpectsDebug(x >= -1 && x <= 1);

    return std::acos(x);
}

template <detail::floating_point T>
[[nodiscard]] inline T
atan2(T y, T x)
{
    gsl_ExpectsDebug(y != 0 || x > 0);

    return std::atan2(y, x);
}

template <detail::floating_point T>
[[nodiscard]] inline T
frac(T x)
{
    return x - std::floor(x);
}


    //
    // Wrap  x  into the  [min,max)  interval.
    //
    // This routine is not adequate for arguments vastly larger than the target range. To support these arguments with high
    // accuracy, use an approach that avoids cancellation, e.g. Payne–Hanek (cf. https://stackoverflow.com/a/30465751).
    //
template <detail::floating_point T>
[[nodiscard]] constexpr T
wraparound(T x, T min, T max)
{
    gsl_Expects(min < max);

    T range = max - min;
    return min + std::fmod(range + fmod(x - min, range), range);
}


template <detail::floating_point T>
[[nodiscard]] constexpr std::pair<T, T>
fractional_weights(T a, T b)
{
    gsl_ExpectsDebug(a >= 0);
    gsl_ExpectsDebug(b >= 0);
    gsl_ExpectsDebug(a + b > 0);

    return { a/(a + b), b/(a + b) };
}

template <typename T>
constexpr T
blend_linear(T a, T b, T x, T y)
{
    auto [wa, wb] = fractional_weights(a, b);
    return wa*x + wb*y;
}
template <typename T>
constexpr T
blend_quadratic(T a, T b, T x, T y)
{
    auto [wa, wb] = fractional_weights(a, b);
    return sqrt(square(wa*x) + square(wb*y));
}


template <detail::arithmetic T>
[[nodiscard]] constexpr detail::assigner<T>
branch_value(T& x) noexcept
{
    return detail::assigner<T>(x);
}
template <detail::arithmetic T>
[[nodiscard]] constexpr detail::assigner<T>
uniform_value(T& x) noexcept
{
    return detail::assigner<T>(x);
}


template <detail::floating_point T>
[[nodiscard]] constexpr T
constrain(T x, bool c)
{
    gsl_Assert(c);
    return x;
}


} // namespace intervals


#endif // INCLUDED_INTERVALS_MATH_HPP_
