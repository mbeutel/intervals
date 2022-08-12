
#ifndef INCLUDED_INTERVALS_DETAIL_SIGN_HPP_
#define INCLUDED_INTERVALS_DETAIL_SIGN_HPP_


#include <compare>

#include <gsl-lite/gsl-lite.hpp>  // for gsl_ExpectsDebug()


namespace intervals {

namespace gsl = gsl_lite;

namespace detail {


constexpr int
_sgn(std::partial_ordering ordering)
{
    gsl_ExpectsDebug(ordering != std::partial_ordering::unordered);

    if (std::is_gt(ordering)) return 1;
    if (std::is_lt(ordering)) return -1;
    else return 0;
}
constexpr int
_sgn(std::weak_ordering ordering)
{
    if (std::is_gt(ordering)) return 1;
    if (std::is_lt(ordering)) return -1;
    else return 0;
}
constexpr int
_sgn(std::strong_ordering ordering)
{
    if (std::is_gt(ordering)) return 1;
    if (std::is_lt(ordering)) return -1;
    else return 0;
}


}  // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_SIGN_HPP_
