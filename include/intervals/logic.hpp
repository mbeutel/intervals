
#ifndef INCLUDED_INTERVALS_LOGIC_HPP_
#define INCLUDED_INTERVALS_LOGIC_HPP_


#include <gsl-lite/gsl-lite.hpp>  // for gsl_ExpectsDebug()


namespace intervals {

namespace gsl = gsl_lite;


[[nodiscard]] constexpr bool
possibly(bool x) noexcept
{
    return x;
}
[[nodiscard]] constexpr bool
possibly_not(bool x) noexcept
{
    return !x;
}
[[nodiscard]] constexpr bool
always(bool x) noexcept
{
    return x;
}
[[nodiscard]] constexpr bool
never(bool x) noexcept
{
    return !x;
}
[[nodiscard]] constexpr bool
contingent(bool) noexcept
{
    return false;
}
[[nodiscard]] constexpr bool
vacuous(bool) noexcept
{
    return false;
}

// TODO: define assign(), assign_partial(), and reset() for set values (i.e. bool, enum, anything that has value metadata available)
//       - or alternatively, change the definitions in math.hpp such that they match any type except sets and intervals, which have
//       their own overloads

template <typename T>
[[nodiscard]] constexpr T
if_else(bool cond, T resultIfTrue, T resultIfFalse)
{
    return cond ? resultIfTrue : resultIfFalse;
}


} // namespace intervals


#endif // INCLUDED_INTERVALS_LOGIC_HPP_
