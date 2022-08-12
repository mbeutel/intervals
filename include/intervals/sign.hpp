
#ifndef INCLUDED_INTERVALS_SIGN_HPP_
#define INCLUDED_INTERVALS_SIGN_HPP_


#include <array>

#include <gsl-lite/gsl-lite.hpp>  // for type_identity<>

#include <intervals/detail/math.hpp>
#include <intervals/detail/sign.hpp>


namespace intervals {

namespace gsl = gsl_lite;


enum sign : int
{
    negativeSign = -1,
    zeroSign     = 0,
    positiveSign = +1
};
constexpr auto 
reflect(gsl::type_identity<sign>)
{
    return std::array{ negativeSign, zeroSign, positiveSign };
}

[[nodiscard]] constexpr sign
operator +(sign x)
{
    return x;
}
[[nodiscard]] constexpr sign
operator -(sign x)
{
    return sign(-int(x));
}
[[nodiscard]] constexpr sign
operator *(sign lhs, sign rhs)
{
    return sign(int(lhs)*int(rhs));
}
template <detail::arithmetic T>
[[nodiscard]] constexpr T
operator *(T lhs, sign rhs)
{
    return lhs*int(rhs);
}
template <detail::arithmetic T>
[[nodiscard]] constexpr T
operator *(sign lhs, T rhs)
{
    return int(lhs)*rhs;
}

template <typename T>
[[nodiscard]] constexpr sign
sgn(T x)
{
    return sign(detail::_sgn(x <=> T{ }));
}


} // namespace intervals


#endif // INCLUDED_INTERVALS_SIGN_HPP_
