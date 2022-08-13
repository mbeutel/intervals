
#ifndef INCLUDED_INTERVALS_LOGIC_HPP_
#define INCLUDED_INTERVALS_LOGIC_HPP_


#include <gsl-lite/gsl-lite.hpp>  // for gsl_ExpectsDebug()


namespace intervals {

namespace gsl = gsl_lite;


[[nodiscard]] constexpr bool
maybe(bool x) noexcept
{
    return x;
}
[[nodiscard]] constexpr bool
maybe_not(bool x) noexcept
{
    return !x;
}
[[nodiscard]] constexpr bool
definitely(bool x) noexcept
{
    return x;
}
[[nodiscard]] constexpr bool
definitely_not(bool x) noexcept
{
    return !x;
}
[[nodiscard]] constexpr bool
contingent(bool) noexcept
{
    return false;
}

template <typename T>
[[nodiscard]] constexpr T
if_else(bool cond, T resultIfTrue, T resultIfFalse)
{
    return cond ? resultIfTrue : resultIfFalse;
}


} // namespace intervals


#endif // INCLUDED_INTERVALS_LOGIC_HPP_
