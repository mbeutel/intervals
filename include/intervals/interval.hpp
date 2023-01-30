
#ifndef INCLUDED_INTERVALS_INTERVAL_HPP_
#define INCLUDED_INTERVALS_INTERVAL_HPP_


#include <cmath>
#include <array>
#include <bitset>
#include <limits>
#include <iosfwd>
#include <numbers>
#include <utility>      // for pair<>
#include <concepts>
#include <iterator>     // for random_access_iterator<>
#include <type_traits>  // for is_const<>, common_type<>

#include <gsl-lite/gsl-lite.hpp>  // for gsl_Assert(), gsl_Expects(), gsl_ExpectsDebug(), gsl_FailFast(), narrow_cast<>(), narrow<>(), narrow_failfast<>()

#include <makeshift/type_traits.hpp>  // for dependent_false<>

#include <intervals/set.hpp>
#include <intervals/sign.hpp>
#include <intervals/math.hpp>   // for floating_point<>, integral<>, and to make assign*() and reset() available
#include <intervals/logic.hpp>  // to make possibly() et al. for Boolean arguments available

#include <intervals/detail/interval.hpp>


namespace intervals {

namespace gsl = gsl_lite;


template <typename T>
class interval;


namespace detail {


class interval_functions
{
private:
    template <typename L, typename R>
    friend constexpr set<bool>
    compare_eq(L const& lhs, R const& rhs)
    {
        auto result = set<bool>{ };
        if (upper(rhs) >= lower(lhs) && lower(rhs) <= upper(lhs))  // intervals overlap
        {
            result.assign(true);
        }
        if (lower(lhs) != upper(rhs) || upper(lhs) != lower(rhs))  // intervals are non-identical or non-degenerate
        {
            result.assign(false);
        }
        return result;
    }
    template <typename L, typename R>
    friend constexpr set<bool>
    compare_neq(L const& lhs, R const& rhs)
    {
        auto result = set<bool>{ };
        if (upper(rhs) >= lower(lhs) && lower(rhs) <= upper(lhs))  // intervals overlap
        {
            result.assign(false);
        }
        if (lower(lhs) != upper(rhs) || upper(lhs) != lower(rhs))  // intervals are non-identical or non-degenerate
        {
            result.assign(true);
        }
        return result;
    }
    template <typename L, typename R>
    static constexpr set<bool>
    compare_lt(L const& lhs, R const& rhs)
    {
        auto result = set<bool>{ };
        if (lower(lhs) < upper(rhs))
        {
            result.assign(true);
        }
        if (upper(lhs) >= lower(rhs))
        {
            result.assign(false);
        }
        return result;
    }
    template <typename L, typename R>
    static constexpr set<bool>
    compare_leq(L const& lhs, R const& rhs)
    {
        auto result = set<bool>{ };
        if (lower(lhs) <= upper(rhs))
        {
            result.assign(true);
        }
        if (upper(lhs) > lower(rhs))
        {
            result.assign(false);
        }
        return result;
    }

        // Compute  x⋅y  but impose that  0⋅∞ = 0 .
    template <interval_value X, interval_value Y>
    static constexpr std::common_type_t<X, Y>
    _multiply_0(X x, Y y)
    {
        auto result = x*y;
        if (std::isnan(result) && (std::isinf(x) && y == 0 || std::isinf(y) && x == 0))
        {
            return 0;
        }
        return result;
    }
    template <interval_value X, arithmetic_interval Y>
    static constexpr common_interval_t<X, Y>
    _multiply_0(X const& x, Y const& y)
    {
        using T = common_interval_value_t<X, Y>;
        T v1 = _multiply_0(x, lower(y));
        T v2 = _multiply_0(x, upper(y));
        return common_interval_t<X, Y>{ intervals::min(v1, v2), intervals::max(v1, v2) };
    }
    template <arithmetic_interval X, interval_value Y>
    static constexpr common_interval_t<X, Y>
    _multiply_0(X const& x, Y const& y)
    {
        using T = common_interval_value_t<X, Y>;
        T v1 = _multiply_0(lower(x), y);
        T v2 = _multiply_0(upper(x), y);
        return common_interval_t<X, Y>{ intervals::min(v1, v2), intervals::max(v1, v2) };
    }
    template <arithmetic_interval X, arithmetic_interval Y>
    static constexpr common_interval_t<X, Y>
    _multiply_0(X const& x, Y const& y)
    {
        using T = common_interval_value_t<X, Y>;
        T v1 = _multiply_0(lower(x), lower(y));
        T v2 = _multiply_0(lower(x), upper(y));
        T v3 = _multiply_0(upper(x), lower(y));
        T v4 = _multiply_0(upper(x), upper(y));
        return common_interval_t<X, Y>{ intervals::min(intervals::min(v1, v2), intervals::min(v3, v4)), intervals::max(intervals::max(v1, v2), intervals::max(v3, v4)) };
    }

public:
    template <typename L, typename R>
    requires any_interval<L, R> && relational_values<L, R>
    [[nodiscard]] friend constexpr auto
    operator ==(L&& lhs, R&& rhs)
    {
        gsl_ExpectsDebug(assigned(lhs) && assigned(rhs));

        return equal_constraint{
            condition{ compare_eq(lhs, rhs) },
            as_interval(std::forward<L>(lhs)),
            as_interval(std::forward<R>(rhs))
        };
    }
    template <typename L, typename R>
    requires any_interval<L, R> && relational_values<L, R>
    [[nodiscard]] friend constexpr auto
    operator !=(L&& lhs, R&& rhs)
    {
        gsl_ExpectsDebug(assigned(lhs) && assigned(rhs));

        return not_equal_constraint{
            condition{ compare_neq(lhs, rhs) },
            as_interval(std::forward<L>(lhs)),
            as_interval(std::forward<R>(rhs))
        };
    }
    template <typename L, typename R>
    requires any_interval<L, R> && relational_values<L, R>
    [[nodiscard]] friend constexpr auto
    operator <(L&& lhs, R&& rhs)
    {
        gsl_ExpectsDebug(assigned(lhs) && assigned(rhs));

        return less_constraint{
            condition{ compare_lt(lhs, rhs) },
            as_interval(std::forward<L>(lhs)),
            as_interval(std::forward<R>(rhs))
        };
    }
    template <typename L, typename R>
    requires any_interval<L, R> && relational_values<L, R>
    [[nodiscard]] friend constexpr auto
    operator <=(L&& lhs, R&& rhs)
    {
        gsl_ExpectsDebug(assigned(lhs) && assigned(rhs));

        return less_equal_constraint{
            condition{ compare_leq(lhs, rhs) },
            as_interval(std::forward<L>(lhs)),
            as_interval(std::forward<R>(rhs))
        };
    }
    template <typename L, typename R>
    requires any_interval<L, R> && relational_values<L, R>
    [[nodiscard]] friend constexpr auto
    operator >(L&& lhs, R&& rhs)
    {
        gsl_ExpectsDebug(assigned(lhs) && assigned(rhs));

        return less_constraint{
            condition{ compare_lt(rhs, lhs) },
            as_interval(std::forward<R>(rhs)),
            as_interval(std::forward<L>(lhs))
        };
    }
    template <typename L, typename R>
    requires any_interval<L, R> && relational_values<L, R>
    [[nodiscard]] friend constexpr auto
    operator >=(L&& lhs, R&& rhs)
    {
        gsl_ExpectsDebug(assigned(lhs) && assigned(rhs));

        return less_equal_constraint{
            condition{ compare_leq(rhs, lhs) },
            as_interval(std::forward<R>(rhs)),
            as_interval(std::forward<L>(lhs))
        };
    }

    template <interval_arg L, interval_arg R>
    requires any_interval<L, R> && same_values<L, R> && relational_values<L, R>
    [[nodiscard]] friend constexpr interval_of_t<L>
    min(L&& lhs, R&& rhs)
    {
        gsl_ExpectsDebug(assigned(lhs) && assigned(rhs));

        return interval_of_t<L>{ intervals::min(lower(lhs), lower(rhs)), intervals::min(upper(lhs), upper(rhs)) };
    }
    template <interval_arg L, interval_arg R>
    requires any_interval<L, R> && same_values<L, R> && relational_values<L, R>
    [[nodiscard]] friend constexpr interval_of_t<L>
    max(L&& lhs, R&& rhs)
    {
        gsl_ExpectsDebug(assigned(lhs) && assigned(rhs));

        return interval_of_t<L>{ intervals::max(lower(lhs), lower(rhs)), intervals::max(upper(lhs), upper(rhs)) };
    }

    template <arithmetic_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    operator +(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        return x;
    }
    template <arithmetic_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    operator -(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        return interval_t<X>{ -upper(x), -lower(x) };
    }

    template <arithmetic_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    square(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        auto lo = lower(x);
        auto hi = upper(x);
        return interval_t<X>{
            lo <= 0 && hi >= 0  // 0 ∈ [a, b]
                ? 0
                : intervals::min(lo*lo, hi*hi),
            intervals::max(lo*lo, hi*hi)
        };
    }
    template <arithmetic_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    cube(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        auto lo = lower(x);
        auto hi = upper(x);
        return interval_t<X>{ lo*lo*lo, hi*hi*hi };
    }

    template <arithmetic_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    abs(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        auto lo = lower(x);
        auto hi = upper(x);
        return lo <= 0 && hi >= 0                             // if  0 ∈ [a,b] :
                 ? interval_t<X>{ 0, intervals::max(-lo, hi) }  //     → [0, max{-a,b}]
             : lo < 0                                         // else if  a,b < 0 :
                 ? interval_t<X>{ -hi, -lo }                  //     → [-b,-a]
               : x;                                           // else → [a,b]
    }

    template <arithmetic_interval X>
    [[nodiscard]] friend constexpr set<sign>
    sgn(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        auto lo = lower(x);
        auto hi = upper(x);
        auto result = set<sign>{ };
        if (hi > 0)
        {
            result.assign(positive_sign);
        }
        if (lo < 0)
        {
            result.assign(negative_sign);
        }
        if (lo <= 0 && hi >= 0)
        {
            result.assign(zero_sign);
        }
        return result;
    }

    template <typename X, typename Y>
    requires any_interval<X, Y> && arithmetic_operands<X, Y>
    [[nodiscard]] friend constexpr common_interval_t<X, Y>
    operator +(X&& x, Y&& y)
    {
        gsl_ExpectsDebug(assigned(x) && assigned(y));

        if constexpr (floating_point_operands<X, Y>)
        {
            if ((is_negative_inf(lower(x)) && is_positive_inf(upper(y)))
                || (is_negative_inf(lower(y)) && is_positive_inf(upper(x))))
            {
                return detail::nan_interval<X, Y>();
            }
        }
        return common_interval_t<X, Y>{ lower(x) + lower(y), upper(x) + upper(y) };
    }
    template <typename X, typename Y>
    requires any_interval<X, Y> && arithmetic_operands<X, Y>
    [[nodiscard]] friend constexpr common_interval_t<X, Y>
    operator -(X&& x, Y&& y)
    {
        gsl_ExpectsDebug(assigned(x) && assigned(y));

        if constexpr (floating_point_operands<X, Y>)
        {
            if ((is_negative_inf(lower(x)) && is_negative_inf(lower(y)))
                || (is_positive_inf(upper(y)) && is_positive_inf(upper(x))))
            {
                return detail::nan_interval<X, Y>();
            }
        }
        return common_interval_t<X, Y>{ lower(x) - upper(y), upper(x) - lower(y) };
    }
    template <typename X, typename Y>
    requires any_interval<X, Y> && arithmetic_operands<X, Y>
    [[nodiscard]] friend constexpr common_interval_t<X, Y>
    operator *(X&& x, Y&& y)
    {
        gsl_ExpectsDebug(assigned(x) && assigned(y));

        using T = common_interval_value_t<X, Y>;
        if constexpr (floating_point_operands<X, Y>)
        {
            if (((is_negative_inf(lower(x)) || is_positive_inf(upper(x))) && contains(y, 0))
                || (contains(x, 0) && (is_negative_inf(lower(y)) || is_positive_inf(upper(y)))))
            {
                return detail::nan_interval<X, Y>();
            }
        }
        if constexpr (interval_value<X>)
        {
            T v1 = x*lower(y);
            T v2 = x*upper(y);
            return common_interval_t<X, Y>{ intervals::min(v1, v2), intervals::max(v1, v2) };
        }
        else if constexpr (interval_value<Y>)
        {
            T v1 = lower(x)*y;
            T v2 = upper(x)*y;
            return common_interval_t<X, Y>{ intervals::min(v1, v2), intervals::max(v1, v2) };
        }
        else
        {
            T v1 = lower(x)*lower(y);
            T v2 = lower(x)*upper(y);
            T v3 = upper(x)*lower(y);
            T v4 = upper(x)*upper(y);
            return common_interval_t<X, Y>{ intervals::min(intervals::min(v1, v2), intervals::min(v3, v4)), intervals::max(intervals::max(v1, v2), intervals::max(v3, v4)) };
        }
    }

    template <arithmetic_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    operator *(sign x, X&& y)
    {
        return int(x)*y;
    }
    template <arithmetic_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    operator *(X&& x, sign y)
    {
        return x*int(y);
    }
    template <arithmetic_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    operator *(set<sign> x, X&& y)
    {
        auto result = interval_t<X>{ };
        if (x.contains(positive_sign))
        {
            result.assign(y);
        }
        if (x.contains(negative_sign))
        {
            result.assign(-y);
        }
        if (x.contains(zero_sign))
        {
            result.assign(0);
        }
        return result;
    }
    template <arithmetic_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    operator *(X&& x, set<sign> y)
    {
        return y*x;
    }

    template <typename X, typename Y>
    requires any_interval<X, Y> && arithmetic_operands<X, Y>
    [[nodiscard]] friend constexpr common_interval_t<X, Y>
    operator /(X&& x, Y&& y)
    {
        gsl_ExpectsDebug(assigned(x) && assigned(y));

        using T = common_interval_value_t<X, Y>;
        if constexpr (interval_value<X>)
        {
            if constexpr (floating_point_operands<X, Y>)
            {
                if ((is_negative_inf(x) || is_positive_inf(x))  // x = -∞ ∨ x = ∞
                    && (is_negative_inf(lower(y)) || is_positive_inf(upper(y))))  // c = -∞ ∨ d = ∞
                {
                        // ∞/∞ = NaN
                    return detail::nan_interval<X, Y>();
                }
                if (x == 0 && lower(y) <= 0 && 0 <= upper(y))  // x = 0 ∧ c ≤ 0 ≤ d
                {
                        // 0/0 = NaN
                    return detail::nan_interval<X, Y>();
                }
                if (x != 0 && lower(y) < 0 && 0 < upper(y))  // x ≠ 0 ∧ c < 0 < d
                {
                        // [a,b]/0 = [-∞,∞]
                    return detail::inf_interval<X, Y>();
                }
            }
            else  // integral_operands<X, Y>
            {
                if (lower(y) <= 0 && 0 <= upper(y))  // c ≤ 0 ≤ d
                {
                        // divide by 0: force trap
                    volatile int zero = 0;
                    return common_interval_t<X, Y>{ 0/zero, 0 };
                }
            }
            T v1 = x/lower(y);
            T v2 = x/upper(y);
            return common_interval_t<X, Y>{ intervals::min(v1, v2), intervals::max(v1, v2) };
        }
        else if constexpr (interval_value<Y>)
        {
            if constexpr (floating_point_operands<X, Y>)
            {
                if ((is_negative_inf(lower(x)) || is_positive_inf(upper(x)))  // a = -∞ ∨ b = ∞
                    && (is_negative_inf(y) || is_positive_inf(y)))  // x = -∞ ∨ x = ∞
                {
                        // ∞/∞ = NaN
                    return detail::nan_interval<X, Y>();
                }
                if (lower(x) <= 0 && 0 <= upper(x) && y == 0)  // a ≤ 0 ≤ b ∧ y = 0
                {
                        // 0/0 = NaN
                    return detail::nan_interval<X, Y>();
                }
            }

            T v1 = lower(x)/y;
            T v2 = upper(x)/y;
            return common_interval_t<X, Y>{ intervals::min(v1, v2), intervals::max(v1, v2) };
        }
        else
        {
            if constexpr (floating_point_operands<X, Y>)
            {
                if ((is_negative_inf(lower(x)) || is_positive_inf(upper(x)))  // a = -∞ ∨ b = ∞
                    && (is_negative_inf(lower(y)) || is_positive_inf(upper(y))))  // c = -∞ ∨ d = ∞
                {
                        // ∞/∞ = NaN
                    return detail::nan_interval<X, Y>();
                }
                if (lower(x) <= 0 && 0 <= upper(x) && lower(y) <= 0 && 0 <= upper(y))  // a ≤ 0 ≤ b ∧ c ≤ 0 ≤ d
                {
                        // 0/0 = NaN
                    return detail::nan_interval<X, Y>();
                }
                if ((0 < lower(x) || upper(x) < 0) && lower(y) < 0 && 0 < upper(y))  // 0 ∉ [a,b] ∧ c < 0 < d
                {
                        // [a,b]/0 = [-∞,∞]
                    return detail::inf_interval<X, Y>();
                }
            }
            else  // integral_operands<X, Y>
            {
                if (lower(y) <= 0 && 0 <= upper(y))  // c ≤ 0 ≤ d
                {
                        // divide by 0: force trap
                    volatile int zero = 0;
                    return common_interval_t<X, Y>{ 0/zero, 0 };
                }
            }
            T v1 = lower(x)/lower(y);
            T v2 = lower(x)/upper(y);
            T v3 = upper(x)/lower(y);
            T v4 = upper(x)/upper(y);
            return common_interval_t<X, Y>{ intervals::min(intervals::min(v1, v2), intervals::min(v3, v4)), intervals::max(intervals::max(v1, v2), intervals::max(v3, v4)) };
        }
    }

    template <floating_point_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    sqrt(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        return interval_t<X>{ intervals::sqrt(lower(x)), intervals::sqrt(upper(x)) };
    }
    template <floating_point_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    cbrt(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        return interval{ intervals::cbrt(lower(x)), intervals::cbrt(upper(x)) };
    }
    template <floating_point_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    log(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        return interval{ intervals::log(lower(x)), intervals::log(upper(x)) };
    }
    template <floating_point_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    exp(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        return interval{ intervals::exp(lower(x)), intervals::exp(upper(x)) };
    }
    template <interval_arg X, interval_arg Y>
    requires any_interval<X, Y> && floating_point_operands<X, Y>
    [[nodiscard]] friend constexpr common_interval_t<X, Y>
    pow(X&& x, Y&& y)
    {
        gsl_ExpectsDebug(assigned(x) && assigned(y));

        using std::max;
        using std::log;

        using XV = interval_arg_value_t<X>;
        using YV = interval_arg_value_t<Y>;
        auto result = common_interval_t<X, Y>{ };
        if (intervals::possibly(x >= 0))
        {
            result.assign(exp(_multiply_0(y, log(max(XV(0), x)))));
        }
        if (intervals::possibly(x < 0))
        {
            YV ylo = lower(y);
            YV yhi = upper(y);
            auto yi = gsl::narrow_cast<long long>(ylo);
            if (ylo == yhi && gsl::narrow_cast<YV>(yi) == ylo)  // y ∈ ℤ
            {
                int sign = yi % 2 == 0 ? 1 : -1;
                result.assign(sign*exp(_multiply_0(ylo, log(max(XV(0), -x)))));
            }
            else
            {
                return detail::nan_interval<X, Y>();
            }
        }
        return result;
    }

    template <floating_point_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    cos(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        using T = interval_value_t<X>;
        auto lo = intervals::wraparound(lower(x), -std::numbers::pi_v<T>, std::numbers::pi_v<T>);
        auto delta = lo - lower(x);
        auto hi = upper(x) + delta;
        if (lo <= T(0))
        {
            if (hi <= T(0))
            {
                return interval_t<X>{ intervals::cos(lo), intervals::cos(hi) };
            }
            else if (hi <= std::numbers::pi_v<T>)
            {
                return interval_t<X>{ intervals::min(intervals::cos(lo), intervals::cos(hi)), 1 };
            }
        }
        else  // 0 < lo < π
        {
            if (hi <= std::numbers::pi_v<T>)
            {
                return interval_t<X>{ intervals::cos(hi), intervals::cos(lo) };
            }
            else if (hi <= 2*std::numbers::pi_v<T>)
            {
                return interval_t<X>{ -1, intervals::max(intervals::cos(lo), intervals::cos(hi)) };
            }
        }
        return interval{ T(-1), T(1) };
    }
    template <floating_point_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    sin(X&& x)
    {
        using T = interval_value_t<X>;
        return cos(x - std::numbers::pi_v<T>/2);
    }
    template <floating_point_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    tan(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        using T = interval_value_t<X>;
        auto lo = intervals::wraparound(lower(x), -std::numbers::pi_v<T>/2, std::numbers::pi_v<T>/2);
        auto delta = lo - lower(x);
        auto hi = upper(x) + delta;
        if (hi - lo >= std::numbers::pi_v<T>)
        {
            return detail::inf_interval<T>();
        }
        return interval_t<X>{ intervals::tan(lo), intervals::tan(hi) };
    }
    template <floating_point_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    acos(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        return interval_t<X>{ intervals::acos(upper(x)), intervals::acos(lower(x)) };
    }
    template <floating_point_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    asin(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        return interval_t<X>{ intervals::asin(lower(x)), intervals::asin(upper(x)) };
    }
    template <floating_point_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    atan(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        return interval_t<X>{ intervals::atan(lower(x)), intervals::atan(upper(x)) };
    }
    template <typename Y, typename X>
    requires any_interval<X, Y> && floating_point_operands<Y, X>
    [[nodiscard]] friend constexpr common_interval_t<Y, X>
    atan2(Y&& y, X&& x)
    {
        gsl_ExpectsDebug(assigned(y) && assigned(x));

        if (lower(x) <= 0 && contains(y, 0))
        {
            return detail::nan_interval<Y, X>();
        }
        if constexpr (interval_value<Y>)
        {
            auto v1 = intervals::atan2(y, lower(x));
            auto v2 = intervals::atan2(y, upper(x));
            return common_interval_t<Y, X>{ intervals::min(v1, v2), intervals::max(v1, v2) };
        }
        else if constexpr (interval_value<X>)
        {
            auto v1 = intervals::atan2(lower(y), x);
            auto v2 = intervals::atan2(upper(y), x);
            return common_interval_t<Y, X>{ intervals::min(v1, v2), intervals::max(v1, v2) };
        }
        else
        {
            auto v1 = intervals::atan2(lower(y), lower(x));
            auto v2 = intervals::atan2(lower(y), upper(x));
            auto v3 = intervals::atan2(upper(y), lower(x));
            auto v4 = intervals::atan2(upper(y), upper(x));
            return common_interval_t<Y, X>{ intervals::min(intervals::min(v1, v2), intervals::min(v3, v4)), intervals::max(intervals::max(v1, v2), intervals::max(v3, v4)) };
        }
    }

    template <floating_point_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    floor(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        return interval_t<X>{ intervals::floor(lower(x)), intervals::floor(upper(x)) };
    }
    template <floating_point_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    ceil(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        return interval_t<X>{ intervals::ceil(lower(x)), intervals::ceil(upper(x)) };
    }
    template <floating_point_interval X>
    [[nodiscard]] friend constexpr interval_t<X>
    frac(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        using T = interval_value_t<X>;
        T lfloor = intervals::floor(lower(x));
        T ufloor = intervals::floor(upper(x));
        if (lfloor != ufloor)
        {
            return interval_t<X>{ 0, 1 };
        }
        return interval_t<X>{ lower(x) - lfloor, upper(x) - ufloor };
    }

    template <typename A, typename B>
    requires any_interval<A, B> && floating_point_operands<A, B>
    [[nodiscard]] friend constexpr std::pair<common_interval_t<A, B>, common_interval_t<A, B>>
    fractional_weights(A&& a, B&& b)
    {
        gsl_ExpectsDebug(assigned(a) && assigned(b));
        gsl_ExpectsDebug(intervals::always(a >= 0));
        gsl_ExpectsDebug(intervals::always(b >= 0));
        gsl_ExpectsDebug(intervals::always(a + b > 0));

        return {
            common_interval_t<A, B>{ lower(a)/(lower(a) + upper(b)), upper(a)/(upper(a) + lower(b)) },
            common_interval_t<A, B>{ lower(b)/(upper(a) + lower(b)), upper(b)/(lower(a) + upper(b)) }
        };
    }

    template <typename AB, typename XY>
    requires any_interval<AB, XY> && floating_point_operands<AB, XY>
    [[nodiscard]] friend constexpr common_interval_t<AB, XY>
    blend_linear(AB&& a, AB&& b, XY&& x, XY&& y)
    {
        gsl_ExpectsDebug(assigned(a) && assigned(b) && assigned(x) && assigned(y));
        gsl_ExpectsDebug(intervals::always(a >= 0));
        gsl_ExpectsDebug(intervals::always(b >= 0));
        gsl_ExpectsDebug(intervals::always(a + b > 0));

            // q = 1/(1 + b/a)  with  0 ≤ q ≤ 1
        auto alo = lower(a);
        auto ahi = upper(a);
        auto blo = lower(b);
        auto bhi = upper(b);
        auto qlo = 1/(1 + bhi/alo);
        auto qhi = 1/(1 + blo/ahi);

            // r = a/(a + b) x + b/(a + b) y
            //   = q x + (1 - q) y
        auto xlo = lower(x);
        auto xhi = upper(x);
        auto ylo = lower(y);
        auto yhi = upper(y);
        auto rlo = xlo > ylo
            ? qlo*xlo + (1 - qlo)*ylo
            : qhi*xlo + (1 - qhi)*ylo;
        auto rhi = xhi < yhi
            ? qlo*xhi + (1 - qlo)*yhi
            : qhi*xhi + (1 - qhi)*yhi;

            // We use `min()` and `max()` to mitigate rounding errors.
        return common_interval_t<AB, XY>{
            std::min(rlo, rhi),
            std::max(rlo, rhi)
        };
    }

    template <floating_point_interval X>
    [[nodiscard]] friend constexpr set<bool>
    isinf(X&& x)
    {
        using std::isinf;

        gsl_ExpectsDebug(x.assigned());

        auto result = set<bool>{ };
        if (is_negative_inf(lower(x)) || is_positive_inf(upper(x)))
        {
            result.assign(true);
        }
        if (!is_negative_inf(upper(x)) && !is_positive_inf(lower(x)))
        {
            result.assign(false);
        }
        return result;
    }
    template <floating_point_interval X>
    [[nodiscard]] friend constexpr set<bool>
    isfinite(X&& x)
    {
        return !isinf(x);
    }
    template <floating_point_interval X>
    [[nodiscard]] friend constexpr set<bool>
    isnan(X&& x)
    {
        gsl_ExpectsDebug(x.assigned());

        if (std::isnan(lower(x)) || std::isnan(upper(x)))
        {
            return set{ false, true };
        }
        return set{ false };
    }

    template <iterator_interval_arg L, integral_interval_arg R>
    requires any_interval<L, R>
    [[nodiscard]] friend constexpr interval_of_t<L>
    operator +(L&& lhs, R&& rhs)
    {
        gsl_ExpectsDebug(assigned(lhs) && assigned(rhs));

        return interval_of_t<L>{ lower(lhs) + lower(rhs), upper(lhs) + upper(rhs) };
    }
    template <integral_interval_arg L, iterator_interval_arg R>
    requires any_interval<L, R>
    [[nodiscard]] friend constexpr interval_of_t<R>
    operator +(L&& lhs, R&& rhs)
    {
        gsl_ExpectsDebug(assigned(lhs) && assigned(rhs));

        return interval_of_t<R>{ lower(lhs) + lower(rhs), upper(lhs) + upper(rhs) };
    }
    template <iterator_interval_arg L, integral_interval_arg R>
    requires any_interval<L, R>
    [[nodiscard]] friend constexpr interval_of_t<L>
    operator -(L&& lhs, R&& rhs)
    {
        gsl_ExpectsDebug(assigned(lhs) && assigned(rhs));

        return interval_of_t<L>{ lower(lhs) - upper(rhs), upper(lhs) - lower(rhs) };
    }

    template <iterator_interval_arg L, iterator_interval_arg R>
    requires any_interval<L, R> && std::same_as<interval_arg_value_t<L>, interval_arg_value_t<R>>
    [[nodiscard]] friend constexpr interval<std::iter_difference_t<interval_arg_value_t<L>>>
    operator -(L&& lhs, R&& rhs)
    {
        gsl_ExpectsDebug(assigned(lhs) && assigned(rhs));

        using Result = interval<std::iter_difference_t<interval_arg_value_t<L>>>;
        return Result{ lower(lhs) - upper(rhs), upper(lhs) - lower(rhs) };
    }

    template <iterator_interval T>
    [[nodiscard]] friend constexpr interval_of_t<T>
    prev(T&& x)
    {
        return interval_of_t<T>{ pred(lower(x)), pred(upper(x)) };
    }
    template <iterator_interval T>
    [[nodiscard]] friend constexpr interval_of_t<T>
    next(T&& x)
    {
        return interval_of_t<T>{ succ(lower(x)), succ(upper(x)) };
    }


    //    // Given an interval  [a,b] , determines the interval classification as per Hickey et al. (2001).
    //enum class hickey_classification : unsigned
    //{
    //    Z  = 0,  // a = 0 ∧ b = 0
    //    M  = 1,  // a < 0 ∧ b > 0
    //    P0 = 2,  // a = 0 ∧ b > 0
    //    P1 = 3,  // a > 0 ∧ b > 0
    //    N0 = 4,  // a < 0 ∧ b = 0
    //    N1 = 5   // a < 0 ∧ b < 0
    //};
    //
    //hickey_classification
    //_classify() const
    //{
    //    gsl_Assert(lower_ <= upper_);  // this traps NaNs and uninitialized states
    //
    //    if (lower_ < T{ })
    //    {
    //        if (upper_ > T{ }) return hickey_classification::M;
    //        else if (upper_ == T{ }) return hickey_classification::N0;
    //        else return hickey_classification::N1;
    //    }
    //    else if (lower_ == T{ })
    //    {
    //        if (upper_ == T{ }) return hickey_classification::Z;
    //        else return hickey_classification::P0;
    //    }
    //    else return hickey_classification::P1;
    //}

};

template <typename T>
class interval_base : public interval_functions
{
    static_assert(!std::is_const_v<T>);

private:
    T lower_;
    T upper_;

protected:
    constexpr interval_base(T _lower, T _upper) noexcept
        : lower_(_lower), upper_(_upper)
    {
    }

    constexpr void
    _reset(T _lower, T _upper) noexcept
    {
        lower_ = _lower;
        upper_ = _upper;
    }
    constexpr void
    _assign(T _lower, T _upper) noexcept
    {
        lower_ = intervals::min(lower_, _lower);
        upper_ = intervals::max(upper_, _upper);
    }

public:
    using value_type = T;

    constexpr bool
    assigned() const noexcept
    {
        return !(lower_ > upper_);
    }

    interval_base(interval_base const&) = default;
    interval_base& operator =(interval_base const&) = delete;

    [[nodiscard]] constexpr T
    lower_unchecked() const noexcept
    {
        return lower_;
    }
    [[nodiscard]] constexpr T
    upper_unchecked() const noexcept
    {
        return upper_;
    }

    [[nodiscard]] constexpr T
    lower() const
    {
        gsl_ExpectsDebug(assigned());

        return lower_;
    }
    [[nodiscard]] constexpr T
    upper() const
    {
        gsl_ExpectsDebug(assigned());

        return upper_;
    }
    [[nodiscard]] constexpr T
    value() const
    {
        if (lower_ == upper_)
        {
            return lower_;
        }
        gsl_FailFast();
    }

    [[nodiscard]] constexpr bool
    contains(T value) const
    {
        return lower_ <= value && value <= upper_;
    }
    [[nodiscard]] constexpr bool
    contains(interval_base const& rhs) const
    {
        return assigned() && rhs.lower_ >= lower_ && rhs.upper_ <= upper_;
    }
    [[nodiscard]] constexpr bool
    encloses(T value) const
    {
        return lower_ < value && value < upper_;
    }
    [[nodiscard]] constexpr bool
    encloses(interval_base const& rhs) const
    {
        return rhs.lower_ > lower_ && rhs.upper_ < upper_;
    }
    [[nodiscard]] constexpr bool
    matches(T value) const
    {
        return lower_ == value && upper_ == value;
    }
    [[nodiscard]] constexpr bool
    matches(interval_base const& rhs) const
    {
        return lower_ == rhs.lower_ && upper_ == rhs.upper_;
    }
};

template <typename T>
class arithmetic_interval_base : public interval_base<T>
{
    using base = interval_base<T>;

protected:
    using base::base;
};

template <typename T>
class iterator_interval_base : public interval_base<T>
{
    using base = interval_base<T>;

protected:
    using base::base;

public:
    [[nodiscard]] constexpr interval<std::iter_value_t<T>>
    operator *() const
    {
        auto result = interval<std::iter_value_t<T>>{ };
        for (T pos = this->lower_unchecked(), end = this->upper_unchecked(); pos <= end; ++pos)
        {
            result.assign(*pos);
        }
        return result;
    }
    [[nodiscard]] constexpr interval<std::iter_value_t<T>>
    operator [](std::ptrdiff_t d) const
    {
        return *(*this + d);
    }
    [[nodiscard]] constexpr interval<std::iter_value_t<T>>
    operator [](interval_base<std::ptrdiff_t> const& d) const
    {
        return *(*this + d);
    }
};


} // namespace detail


    // Algebraic type representing a bounded scalar.
template <typename T>
class interval;
template <detail::interval_value T>
interval(interval<T>) -> interval<T>;
template <detail::interval_value T>
interval(T) -> interval<T>;
template <detail::interval_value T1, detail::interval_value T2>
interval(T1, T2) -> interval<std::common_type_t<T1, T2>>;
template <std::floating_point T>
class interval<T> : public detail::arithmetic_interval_base<T>
{
    using basebase = detail::interval_base<T>;

    static constexpr T inf_ = std::numeric_limits<T>::infinity();

public:
    using base = detail::arithmetic_interval_base<T>;
    using interval_type = interval;

    constexpr interval() noexcept
        : base(inf_, -inf_)
    {
    }
    constexpr interval(T value) noexcept
        : base(value, value)
    {
    }
    explicit constexpr interval(T _lower, T _upper)
        : base(_lower, _upper)
    {
        gsl_Expects(!(_lower > _upper));  // does not trigger for NaNs
    }
    template <std::convertible_to<T> U>
    constexpr interval(detail::interval_base<U> const& rhs)
        : base(inf_, -inf_)
    {
        if (rhs.assigned())
        {
            this->_reset(rhs.lower_unchecked(), rhs.upper_unchecked());
        }
    }

    constexpr interval&
    reset() noexcept
    {
        this->_reset(inf_, -inf_);
        return *this;
    }
    constexpr interval&
    reset(T rhs) noexcept
    {
        this->_reset(rhs, rhs);
        return *this;
    }
    constexpr interval&
    reset(basebase const& rhs) noexcept
    {
        this->_reset(rhs.lower_unchecked(), rhs.upper_unchecked());
        return *this;
    }
    constexpr interval&
    assign(T rhs) noexcept
    {
        this->_assign(rhs, rhs);
        return *this;
    }
    constexpr interval&
    assign(basebase const& rhs)
    {
        gsl_Expects(rhs.assigned());

        this->_assign(rhs.lower_unchecked(), rhs.upper_unchecked());
        return *this;
    }
};
template <std::integral T>
class interval<T> : public detail::arithmetic_interval_base<T>
{
    static_assert(std::is_signed_v<T>);

    using basebase = detail::interval_base<T>;

    static constexpr T min_ = std::numeric_limits<T>::min();
    static constexpr T max_ = std::numeric_limits<T>::max();

public:
    using base = detail::arithmetic_interval_base<T>;
    using interval_type = interval;

    constexpr interval() noexcept
        : base(max_, min_)
    {
    }
    constexpr interval(T value) noexcept
        : base(value, value)
    {
    }
    explicit constexpr interval(T _lower, T _upper)
        : base(_lower, _upper)
    {
        gsl_Expects(!(_lower > _upper));  // does not trigger for NaNs
    }
    template <std::convertible_to<T> U>
    constexpr interval(detail::interval_base<U> const& rhs)
        : base(max_, min_)
    {
        if (rhs.assigned())
        {
            this->_reset(rhs.lower_unchecked(), rhs.upper_unchecked());
        }
    }

    constexpr interval&
    reset() noexcept
    {
        this->_reset(max_, min_);
        return *this;
    }
    constexpr interval&
    reset(T rhs) noexcept
    {
        this->_reset(rhs, rhs);
        return *this;
    }
    constexpr interval&
    reset(basebase const& rhs) noexcept
    {
        this->_reset(rhs.lower_unchecked(), rhs.upper_unchecked());
        return *this;
    }
    constexpr interval&
    assign(T rhs) noexcept
    {
        this->_assign(rhs, rhs);
        return *this;
    }
    constexpr interval&
    assign(basebase const& rhs)
    {
        gsl_Expects(rhs.assigned());

        this->_assign(rhs.lower_unchecked(), rhs.upper_unchecked());
        return *this;
    }
};
template <std::random_access_iterator T>
class interval<T> : public detail::iterator_interval_base<T>
{
    using basebase = detail::interval_base<T>;

public:
    using base = detail::iterator_interval_base<T>;
    using interval_type = interval;

    /*constexpr interval() noexcept
        : base(max_, min_)
    {
    }*/
    constexpr interval(T value) noexcept
        : base(value, value)
    {
    }
    explicit constexpr interval(T _lower, T _upper)
        : base(_lower, _upper)
    {
        gsl_Expects(!(_lower > _upper));
    }
    template <std::convertible_to<T> U>
    constexpr interval(detail::interval_base<U> const& rhs)
        : base(rhs.lower_unchecked(), rhs.upper_unchecked())
    {
        gsl_Expects(rhs.assigned());
    }

    /*constexpr interval&
    reset()
    {
        this->_reset(max_, min_);
        return *this;
    }*/
    constexpr interval&
    reset(T rhs) noexcept
    {
        this->_reset(rhs, rhs);
        return *this;
    }
    constexpr interval&
    reset(basebase const& rhs) noexcept
    {
        this->_reset(rhs.lower_unchecked(), rhs.upper_unchecked());
        return *this;
    }
    constexpr interval&
    assign(T rhs) noexcept
    {
        this->_assign(rhs, rhs);
        return *this;
    }
    constexpr interval&
    assign(basebase const& rhs)
    {
        gsl_Expects(rhs.assigned());

        this->_assign(rhs.lower_unchecked(), rhs.upper_unchecked());
        return *this;
    }
};


template <typename T> struct set_of;
template <> struct set_of<bool> { using type = set<bool>; };
template <detail::enum_ T> struct set_of<T> { using type = set<T>; };
template <std::floating_point T> struct set_of<T> { using type = interval<T>; };
template <std::integral T> struct set_of<T> { using type = interval<T>; };
template <std::random_access_iterator T> struct set_of<T> { using type = interval<T>; };
template <typename T, typename ReflectorT> struct set_of<set<T, ReflectorT>> { using type = set<T, ReflectorT>; };
template <std::derived_from<detail::interval_functions> IntervalT> struct set_of<IntervalT> { using type = interval<typename IntervalT::value_type>; };
template <typename T> using set_of_t = typename set_of<T>::type;

template <typename S, typename T> struct propagate_set { using type = T; };
template <std::derived_from<detail::interval_functions> IntervalT, typename T> struct propagate_set<IntervalT, T> { using type = set_of_t<T>; };
template <typename U, typename T> struct propagate_set<set<U>, T> { using type = set_of_t<T>; };
template <typename S, typename T> using propagate_set_t = typename propagate_set<S, T>::type;


template <detail::any_interval T, detail::interval_arg U>
requires std::convertible_to<detail::interval_arg_value_t<U>, detail::interval_value_t<T>>
[[nodiscard]] constexpr inline T
narrow_cast(U&& rhs) noexcept
{
    using TV = detail::interval_value_t<T>;
    if (detail::assigned(rhs))
    {
        return T{ gsl::narrow_cast<TV>(detail::lower(rhs)), gsl::narrow_cast<TV>(detail::upper(rhs)) };
    }
    return T{ };
}
template <detail::any_interval T, detail::interval_arg U>
requires std::convertible_to<detail::interval_arg_value_t<U>, detail::interval_value_t<T>>
[[nodiscard]] constexpr inline T
narrow(U&& rhs)
{
    using TV = detail::interval_value_t<T>;
    if (detail::assigned(rhs))
    {
        return T{ gsl::narrow<TV>(detail::lower(rhs)), gsl::narrow<TV>(detail::upper(rhs)) };
    }
    return T{ };
}
template <detail::any_interval T, detail::interval_arg U>
requires std::convertible_to<detail::interval_arg_value_t<U>, detail::interval_value_t<T>>
[[nodiscard]] constexpr inline T
narrow_failfast(U&& rhs)
{
    using TV = detail::interval_value_t<T>;
    if (detail::assigned(rhs))
    {
        return T{ gsl::narrow_failfast<TV>(detail::lower(rhs)), gsl::narrow_failfast<TV>(detail::upper(rhs)) };
    }
    return T{ };
}


template <typename ElemT, typename TraitsT, typename T>
std::basic_ostream<ElemT, TraitsT>&
operator <<(std::basic_ostream<ElemT, TraitsT>& stream, detail::interval_base<T> const& x)
{
    gsl_ExpectsDebug(x.assigned());
    if (x.lower_unchecked() == x.upper_unchecked())
    {
        return stream << x.lower_unchecked();
    }
    else
    {
        static constexpr char const* cs = ", ";
        static constexpr ElemT ecs[] = { ElemT(cs[0]), ElemT(cs[1]), 0 };
        return stream << ElemT('[') << x.lower_unchecked() << ecs << x.upper_unchecked() << ElemT(']');
    }
}


template <std::size_t I, typename T>
[[nodiscard]] constexpr T
get(detail::interval_base<T> const& bound)
{
    static_assert(I < 2, "tuple index out of range");
    if constexpr (I == 0) return bound.lower();
    if constexpr (I == 1) return bound.upper();
}


template <detail::arithmetic T>
[[nodiscard]] constexpr interval<T>
operator *(T lhs, set<sign> rhs)
{
    auto result = interval<T>{ };
    if (rhs.contains(positive_sign))
    {
        result.assign(lhs);
    }
    if (rhs.contains(zero_sign))
    {
        result.assign(T{ });
    }
    if (rhs.contains(negative_sign))
    {
        result.assign(-lhs);
    }
    return result;
}
template <detail::arithmetic T>
[[nodiscard]] constexpr interval<T>
operator *(set<sign> lhs, T rhs)
{
    return rhs*lhs;
}


template <typename T>
constexpr void
assign(interval<T>& lhs, gsl::type_identity_t<interval<T>> const& rhs)
{
    gsl_Expects(!lhs.assigned());
    lhs.reset(rhs);
}
template <typename T>
constexpr void
assign_partial(interval<T>& lhs, gsl::type_identity_t<interval<T>> const& rhs)
{
    lhs.assign(rhs);
}
template <detail::any_interval IntervalT>
constexpr void
reset(IntervalT& lhs, gsl::type_identity_t<detail::interval_t<IntervalT>> const& rhs)
{
    lhs.reset(rhs);
}

    // Expression does not constrain given interval.
template <detail::any_interval IntervalT>
constexpr IntervalT const&
constrain(IntervalT const& x, set<bool>)
{
    static_assert(makeshift::dependent_false<IntervalT>, "conditional expression does not constrain given interval");
    return x;
}
template <detail::any_interval IntervalT>
constexpr IntervalT const&&
constrain(IntervalT const&& x, set<bool>)
{
    static_assert(makeshift::dependent_false<IntervalT>, "conditional expression does not constrain given interval");
    return std::move(x);
}

template <detail::any_interval IntervalT, std::derived_from<detail::condition> ConditionT>
[[nodiscard]] constexpr detail::as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, ConditionT const& c)
{
    bool constraintApplied = false;
    auto xc = detail::constrain(x, c, constraintApplied);
    gsl_Assert(constraintApplied && "conditional expression does not constrain given interval");
    return xc;
}
template <detail::any_interval IntervalT, std::derived_from<detail::condition> ConditionT>
[[nodiscard]] constexpr IntervalT const&&
constrain(IntervalT const&& x, ConditionT const&)
{
    static_assert(makeshift::dependent_false<IntervalT>, "rvalue interval cannot be constrained");
    return std::move(x);
}


template <detail::interval_value T>
[[nodiscard]] constexpr interval<T>
if_else(set<bool> cond, detail::interval_base<T> const& resultIfTrue, detail::interval_base<T> const& resultIfFalse)
{
    interval<T> result;
    if (intervals::possibly(cond))
    {
        result.assign(resultIfTrue);
    }
    if (intervals::possibly_not(cond))
    {
        result.assign(resultIfFalse);
    }
    return result;
}
template <detail::interval_value T>
[[nodiscard]] constexpr interval<T>
if_else(set<bool> cond, T resultIfTrue, detail::interval_base<T> const& resultIfFalse)
{
    return intervals::if_else(cond, interval(resultIfTrue), resultIfFalse);
}
template <detail::interval_value T>
[[nodiscard]] constexpr interval<T>
if_else(set<bool> cond, detail::interval_base<T> const& resultIfTrue, T resultIfFalse)
{
    return intervals::if_else(cond, resultIfTrue, interval(resultIfFalse));
}
template <typename T>
requires detail::not_interval<T> && detail::not_instantiation_of<T, set>
[[nodiscard]] constexpr set_of_t<T>
if_else(set<bool> cond, T resultIfTrue, T resultIfFalse)
{
    return intervals::if_else(cond, set_of_t<T>(resultIfTrue), set_of_t<T>(resultIfFalse));
}


} // namespace intervals


    // Implement tuple-like protocol for intervals.
template <intervals::detail::any_interval IntervalT> class std::tuple_size<IntervalT> : public std::integral_constant<std::size_t, 2> { };
template <std::size_t I, intervals::detail::any_interval IntervalT> class std::tuple_element<I, IntervalT> { public: using type = typename IntervalT::value_type; };

    // Specialize `std::common_type<>` for intervals.
    // TODO: constraints are apparently inadequate here; we need to be more specific
//template <intervals::detail::any_interval L, intervals::detail::any_interval R>
//struct std::common_type<L, R>
//{
//    using type = intervals::detail::common_interval_t<L, R>;
//};
//template <intervals::detail::any_interval L, intervals::detail::interval_value R>
//struct std::common_type<L, R>
//{
//    using type = intervals::detail::common_interval_t<L, R>;
//};
//template <intervals::detail::interval_value L, intervals::detail::any_interval R>
//struct std::common_type<L, R>
//{
//    using type = intervals::detail::common_interval_t<L, R>;
//};


#endif // INCLUDED_INTERVALS_INTERVAL_HPP_
