
#ifndef INCLUDED_INTERVALS_INTERVAL_HPP_
#define INCLUDED_INTERVALS_INTERVAL_HPP_


#include <cmath>
#include <array>
#include <bitset>
#include <limits>
#include <utility>      // for pair<>
#include <type_traits>  // for is_const<>

#include <gsl-lite/gsl-lite.hpp>  // for gsl_Assert(), gsl_Expects(), gsl_ExpectsDebug(), gsl_FailFast()

#include <makeshift/concepts.hpp>  // for tuple_like<>

#include <intervals/set.hpp>
#include <intervals/sign.hpp>
#include <intervals/math.hpp>

#include <intervals/detail/math.hpp>      // for floating_point<>, and to make maybe() et al. for lvalue references available
#include <intervals/detail/logic.hpp>     // to make maybe() et al. for Boolean arguments available
#include <intervals/detail/interval.hpp>


namespace intervals {

namespace gsl = gsl_lite;


[[nodiscard]] constexpr bool
maybe(detail::condition x) noexcept
{
    return x.value_.contains(true);
}
[[nodiscard]] constexpr bool
maybe_not(detail::condition x) noexcept
{
    return x.value_.contains(false);
}
[[nodiscard]] constexpr bool
definitely(detail::condition x) noexcept
{
    return x.value_.matches(true);
}
[[nodiscard]] constexpr bool
definitely_not(detail::condition x) noexcept
{
    return x.value_.matches(false);
}
[[nodiscard]] constexpr bool
contingent(detail::condition x) noexcept
{
    return x.value_.matches(set{ false, true });
}


    // Algebraic type representing a bounded scalar.
template <detail::floating_point T>
class interval
{
    static_assert(!std::is_const_v<T>);

    static constexpr T inf = std::numeric_limits<T>::infinity();
    static constexpr T nan = std::numeric_limits<T>::quiet_NaN();

private:
    T lower_;
    T upper_;

        // Given an interval  [a,b] , determines the interval classification as per Hickey et al. (2001).
    enum class hickey_classification : unsigned
    {
        Z  = 0,  // a = 0 ∧ b = 0
        M  = 1,  // a < 0 ∧ b > 0
        P0 = 2,  // a = 0 ∧ b > 0
        P1 = 3,  // a > 0 ∧ b > 0
        N0 = 4,  // a < 0 ∧ b = 0
        N1 = 5   // a < 0 ∧ b < 0
    };

    hickey_classification
    _classify() const
    {
        gsl_Assert(lower_ <= upper_);  // this traps NaNs and uninitialized states

        if (lower_ < T{ })
        {
            if (upper_ > T{ }) return hickey_classification::M;
            else if (upper_ == T{ }) return hickey_classification::N0;
            else return hickey_classification::N1;
        }
        else if (lower_ == T{ })
        {
            if (upper_ == T{ }) return hickey_classification::Z;
            else return hickey_classification::P0;
        }
        else return hickey_classification::P1;
    }

    constexpr bool
    _assigned() const
    {
        return !(lower_ > upper_);
    }

        // Compute  x⋅y  but impose that  0⋅∞ = 0 .
    static constexpr T
    _multiply_0(T x, T y)
    {
        using std::isnan;
        using std::isinf;

        T result = x*y;
        if (isnan(result) && (isinf(x) && y == T(0.) || isinf(y) && x == T(0)))
        {
            return T(0);
        }
        return result;
    }
    static constexpr interval
    _multiply_0(interval const& lhs, interval const& rhs)
    {
        T v1 = _multiply_0(lhs.lower_, rhs.lower_);
        T v2 = _multiply_0(lhs.lower_, rhs.upper_);
        T v3 = _multiply_0(lhs.upper_, rhs.lower_);
        T v4 = _multiply_0(lhs.upper_, rhs.upper_);
        return interval{ detail::_min(detail::_min(v1, v2), detail::_min(v3, v4)), detail::_max(detail::_max(v1, v2), detail::_max(v3, v4)) };
    }
    static constexpr interval
    _multiply_0(T lhs, interval const& rhs)
    {
        T v1 = _multiply_0(lhs, rhs.lower_);
        T v2 = _multiply_0(lhs, rhs.upper_);
        return interval{ detail::_min(v1, v2), detail::_max(v1, v2) };
    }


public:
    using value_type = T;

    constexpr interval()
        : lower_(inf), upper_(-inf)
    {
    }
    constexpr interval(T value)
        : lower_(value), upper_(value)
    {
    }
    explicit constexpr interval(T _lower, T _upper)
        : lower_(_lower), upper_(_upper)
    {
        gsl_Expects(!(_lower > _upper));  // does not trigger for NaNs
    }

    interval(interval const&) = default;
    interval& operator =(interval const&) = delete;
    constexpr interval&
    assign(interval const& rhs)
    {
        gsl_ExpectsDebug(rhs._assigned());

        lower_ = detail::_min(lower_, rhs.lower_);
        upper_ = detail::_max(upper_, rhs.upper_);
        return *this;
    }
    constexpr interval&
    assign(T value)
    {
        lower_ = detail::_min(lower_, value);
        upper_ = detail::_max(upper_, value);
        return *this;
    }

    constexpr interval&
    reset()
    {
        lower_ = inf;
        upper_ = -inf;
        return *this;
    }
    constexpr interval&
    reset(interval const& rhs)
    {
        lower_ = rhs.lower_;
        upper_ = rhs.upper_;
        return *this;
    }

    [[nodiscard]] constexpr T
    lower_unchecked() const
    {
        return lower_;
    }
    [[nodiscard]] constexpr T
    upper_unchecked() const
    {
        return upper_;
    }

    [[nodiscard]] constexpr T
    lower() const
    {
        gsl_ExpectsDebug(_assigned());

        return lower_;
    }
    [[nodiscard]] constexpr T
    upper() const
    {
        gsl_ExpectsDebug(_assigned());

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
    contains(interval const& rhs) const
    {
        return _assigned() && rhs.lower_ >= lower_ && rhs.upper_ <= upper_;
    }
    [[nodiscard]] constexpr bool
    encloses(T value) const
    {
        return lower_ < value && value < upper_;
    }
    [[nodiscard]] constexpr bool
    encloses(interval const& rhs) const
    {
        return rhs.lower_ > lower_ && rhs.upper_ < upper_;
    }
    [[nodiscard]] constexpr bool
    matches(T value) const
    {
        return lower_ == value && upper_ == value;
    }
    [[nodiscard]] constexpr bool
    matches(interval const& rhs) const
    {
        return lower_ == rhs.lower_ && upper_ == rhs.upper_;
    }

    // TODO: we could put more interval functions here, e.g. `overlaps_with()`, `intersect()`, but let's see if we actually
    // want them here, or if we'd rather keep the two different semantics separate.

    //[[nodiscard]] friend constexpr set<bool>
    //operator ==(interval const& lhs, interval const& rhs)
    //{
    //    gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());
    //
    //    if (lhs.matches(rhs))
    //    {
    //        return set{ true };
    //    }
    //    else if (rhs.upper_ >= lhs.lower_ && rhs.lower_ <= lhs.upper_)  // overlapping intervals
    //    {
    //        return set{ false, true };
    //    }
    //    else
    //    {
    //        return set{ false };
    //    }
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator ==(interval const& lhs, T rhs)
    //{
    //    gsl_ExpectsDebug(lhs._assigned());
    //
    //    if (lhs.matches(rhs))
    //    {
    //        return set{ true };
    //    }
    //    else if (lhs.contains(rhs))  // overlapping intervals
    //    {
    //        return set{ false, true };
    //    }
    //    else
    //    {
    //        return set{ false };
    //    }
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator ==(T lhs, interval const& rhs)
    //{
    //    return rhs == lhs;
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator !=(interval const& lhs, interval const& rhs)
    //{
    //    return !(lhs == rhs);
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator !=(interval const& lhs, T rhs)
    //{
    //    return !(lhs == rhs);
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator !=(T lhs, interval const& rhs)
    //{
    //    return !(rhs == lhs);
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator <(interval const& lhs, interval const& rhs)
    //{
    //    gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());
    //
    //    auto result = set<bool>{ };
    //    if (lhs.lower_ < rhs.upper_)
    //    {
    //        result.assign(true);
    //    }
    //    if (lhs.upper_ >= rhs.lower_)
    //    {
    //        result.assign(false);
    //    }
    //    return result;
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator <(interval const& lhs, T rhs)
    //{
    //    return lhs < interval(rhs);
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator <(T lhs, interval const& rhs)
    //{
    //    return interval(lhs) < rhs;
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator >(interval const& lhs, interval const& rhs)
    //{
    //    return rhs < lhs;
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator >(interval const& lhs, T rhs)
    //{
    //    return rhs < lhs;
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator >(T lhs, interval const& rhs)
    //{
    //    return rhs < lhs;
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator <=(interval const& lhs, interval const& rhs)
    //{
    //    gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());
    //
    //    auto result = set<bool>{ };
    //    if (lhs.lower_ <= rhs.upper_)
    //    {
    //        result.assign(true);
    //    }
    //    if (lhs.upper_ > rhs.lower_)
    //    {
    //        result.assign(false);
    //    }
    //    return result;
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator <=(interval const& lhs, T rhs)
    //{
    //    return lhs <= interval(rhs);
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator <=(T lhs, interval const& rhs)
    //{
    //    return interval(lhs) <= rhs;
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator >=(interval const& lhs, interval const& rhs)
    //{
    //    return rhs <= lhs;
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator >=(interval const& lhs, T rhs)
    //{
    //    return interval(rhs) <= lhs;
    //}
    //[[nodiscard]] friend constexpr set<bool>
    //operator >=(T lhs, interval const& rhs)
    //{
    //    return rhs <= interval(lhs);
    //}
    [[nodiscard]] friend constexpr set<bool>
    operator ==(interval const& lhs, interval const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        if (lhs.matches(rhs))
        {
            return set{ true };
        }
        else if (rhs.upper_ >= lhs.lower_ && rhs.lower_ <= lhs.upper_)  // overlapping intervals
        {
            return set{ false, true };
        }
        else
        {
            return set{ false };
        }
    }
    [[nodiscard]] friend constexpr set<bool>
    operator ==(interval const& lhs, T rhs)
    {
        gsl_ExpectsDebug(lhs._assigned());

        if (lhs.matches(rhs))
        {
            return set{ true };
        }
        else if (lhs.contains(rhs))  // overlapping intervals
        {
            return set{ false, true };
        }
        else
        {
            return set{ false };
        }
    }
    [[nodiscard]] friend constexpr set<bool>
    operator ==(T lhs, interval const& rhs)
    {
        return rhs == lhs;
    }
    [[nodiscard]] friend constexpr set<bool>
    operator !=(interval const& lhs, interval const& rhs)
    {
        return !(lhs == rhs);
    }
    [[nodiscard]] friend constexpr set<bool>
    operator !=(interval const& lhs, T rhs)
    {
        return !(lhs == rhs);
    }
    [[nodiscard]] friend constexpr set<bool>
    operator !=(T lhs, interval const& rhs)
    {
        return !(rhs == lhs);
    }
    [[nodiscard]] friend constexpr set<bool>
    operator <(interval const& lhs, interval const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        auto result = set<bool>{ };
        if (lhs.lower_ < rhs.upper_)
        {
            result.assign(true);
        }
        if (lhs.upper_ >= rhs.lower_)
        {
            result.assign(false);
        }
        return result;
    }
    [[nodiscard]] friend constexpr set<bool>
    operator <(interval const& lhs, T rhs)
    {
        return lhs < interval(rhs);
    }
    [[nodiscard]] friend constexpr set<bool>
    operator <(T lhs, interval const& rhs)
    {
        return interval(lhs) < rhs;
    }
    [[nodiscard]] friend constexpr set<bool>
    operator >(interval const& lhs, interval const& rhs)
    {
        return rhs < lhs;
    }
    [[nodiscard]] friend constexpr set<bool>
    operator >(interval const& lhs, T rhs)
    {
        return rhs < lhs;
    }
    [[nodiscard]] friend constexpr set<bool>
    operator >(T lhs, interval const& rhs)
    {
        return rhs < lhs;
    }
    [[nodiscard]] friend constexpr set<bool>
    operator <=(interval const& lhs, interval const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        auto result = set<bool>{ };
        if (lhs.lower_ <= rhs.upper_)
        {
            result.assign(true);
        }
        if (lhs.upper_ > rhs.lower_)
        {
            result.assign(false);
        }
        return result;
    }
    [[nodiscard]] friend constexpr set<bool>
    operator <=(interval const& lhs, T rhs)
    {
        return lhs <= interval(rhs);
    }
    [[nodiscard]] friend constexpr set<bool>
    operator <=(T lhs, interval const& rhs)
    {
        return interval(lhs) <= rhs;
    }
    [[nodiscard]] friend constexpr set<bool>
    operator >=(interval const& lhs, interval const& rhs)
    {
        return rhs <= lhs;
    }
    [[nodiscard]] friend constexpr set<bool>
    operator >=(interval const& lhs, T rhs)
    {
        return interval(rhs) <= lhs;
    }
    [[nodiscard]] friend constexpr set<bool>
    operator >=(T lhs, interval const& rhs)
    {
        return rhs <= interval(lhs);
    }

    [[nodiscard]] friend constexpr interval
    min(interval const& lhs, interval const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        return interval{ detail::_min(lhs.lower_, rhs.lower_), detail::_min(lhs.upper_, rhs.upper_) };
    }
    [[nodiscard]] friend constexpr interval
    min(interval const& lhs, T rhs)
    {
        gsl_ExpectsDebug(lhs._assigned());

        return interval{ detail::_min(lhs.lower_, rhs), detail::_min(lhs.upper_, rhs) };
    }
    [[nodiscard]] friend constexpr interval
    min(T lhs, interval const& rhs)
    {
        return min(rhs, lhs);
    }
    [[nodiscard]] friend constexpr interval
    max(interval const& lhs, interval const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        return interval{ detail::_max(lhs.lower_, rhs.lower_), detail::_max(lhs.upper_, rhs.upper_) };
    }
    [[nodiscard]] friend constexpr interval
    max(interval const& lhs, T rhs)
    {
        gsl_ExpectsDebug(lhs._assigned());

        return interval{ detail::_max(lhs.lower_, rhs), detail::_max(lhs.upper_, rhs) };
    }
    [[nodiscard]] friend constexpr interval
    max(T lhs, interval const& rhs)
    {
        return max(rhs, lhs);
    }

    [[nodiscard]] constexpr interval
    operator +() const
    {
        gsl_ExpectsDebug(_assigned());

        return *this;
    }
    [[nodiscard]] constexpr interval
    operator -() const
    {
        gsl_ExpectsDebug(_assigned());

        return interval{ -upper_, -lower_ };
    }

    [[nodiscard]] friend constexpr interval
    operator +(interval const& lhs, interval const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        if ((lhs.lower_ == -inf && rhs.upper_ == inf)
            || ((rhs.lower_ == -inf && lhs.upper_ == inf)))
        {
            return interval{ nan, nan };
        }
        return interval{ lhs.lower_ + rhs.lower_, lhs.upper_ + rhs.upper_ };
    }
    [[nodiscard]] friend constexpr interval
    operator +(T lhs, interval const& rhs)
    {
        return interval(lhs) + rhs;
    }
    [[nodiscard]] friend constexpr interval
    operator +(interval const& lhs, T rhs)
    {
        return lhs + interval(rhs);
    }
    [[nodiscard]] friend constexpr interval
    operator -(interval const& lhs, interval const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        if ((lhs.lower_ == -inf && rhs.lower_ == -inf)
            || ((lhs.upper_ == inf && rhs.upper_ == inf)))
        {
            return interval{ nan, nan };
        }
        return interval{ lhs.lower_ - rhs.upper_, lhs.upper_ - rhs.lower_ };
    }
    [[nodiscard]] friend constexpr interval
    operator -(T lhs, interval const& rhs)
    {
        gsl_ExpectsDebug(rhs._assigned());

        return interval{ lhs - rhs.upper_, lhs - rhs.lower_ };
    }
    [[nodiscard]] friend constexpr interval
    operator -(interval const& lhs, T rhs)
    {
        gsl_ExpectsDebug(lhs._assigned());

        return interval{ lhs.lower_ - rhs, lhs.upper_ - rhs };
    }
    [[nodiscard]] friend constexpr interval
    operator *(interval const& lhs, interval const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        if (((lhs.lower_ == -inf || lhs.upper_ == inf) && rhs.contains(T(0)))
            || (lhs.contains(T(0)) && (rhs.lower_ == -inf || rhs.upper_ == inf)))
        {
            return interval{ nan, nan };
        }
        T v1 = lhs.lower_*rhs.lower_;
        T v2 = lhs.lower_*rhs.upper_;
        T v3 = lhs.upper_*rhs.lower_;
        T v4 = lhs.upper_*rhs.upper_;
        return interval{ detail::_min(detail::_min(v1, v2), detail::_min(v3, v4)), detail::_max(detail::_max(v1, v2), detail::_max(v3, v4)) };
    }
    [[nodiscard]] friend constexpr interval
    operator *(T lhs, interval const& rhs)
    {
        gsl_ExpectsDebug(rhs._assigned());

        if (((lhs == -inf || lhs == inf) && rhs.contains(T(0)))
            || (lhs == T(0) && (rhs.lower_ == -inf || rhs.upper_ == inf)))
        {
            return interval{ nan, nan };
        }
        T v1 = lhs*rhs.lower_;
        T v2 = lhs*rhs.upper_;
        return interval{ detail::_min(v1, v2), detail::_max(v1, v2) };
    }
    [[nodiscard]] friend constexpr interval
    operator *(interval const& lhs, T rhs)
    {
        return rhs*lhs;
    }
    [[nodiscard]] friend constexpr interval
    operator *(sign lhs, interval const& rhs)
    {
        return int(lhs)*rhs;
    }
    [[nodiscard]] friend constexpr interval
    operator *(interval const& lhs, sign rhs)
    {
        return lhs*int(rhs);
    }
    [[nodiscard]] friend constexpr interval
    operator *(set<sign> lhs, interval const& rhs)
    {
        interval result;
        if (lhs.contains(positiveSign))
        {
            result.assign(rhs);
        }
        if (lhs.contains(negativeSign))
        {
            result.assign(-rhs);
        }
        if (lhs.contains(zeroSign))
        {
            result.assign(0);
        }
        return result;
    }
    [[nodiscard]] friend constexpr interval
    operator *(interval const& lhs, set<sign> rhs)
    {
        return rhs*lhs;
    }
    [[nodiscard]] friend constexpr interval
    operator /(interval const& lhs, interval const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());
        
        if ((lhs.lower_ == -inf || lhs.upper_ == inf)  // a = -∞ ∨ b = ∞
            && (rhs.lower_ == -inf || rhs.upper_ == inf))  // c = -∞ ∨ d = ∞
        {
                // ∞/∞ = NaN
            return interval{ nan, nan };
        }
        if (lhs.lower_ <= T(0) && T(0) <= lhs.upper_ && rhs.lower_ <= T(0) && T(0) <= rhs.upper_)  // a ≤ 0 ≤ b ∧ c ≤ 0 ≤ d
        {
                // 0/0 = NaN
            return interval{ nan, nan };
        }
        if ((T(0) < lhs.lower_ || lhs.upper_ < T(0)) && rhs.lower_ < T(0) && T(0) < rhs.upper_)  // 0 ∉ [a,b] ∧ c < 0 < d
        {
                // [a,b]/0 = [-∞,∞]
            return interval{ -inf, inf };
        }
        T v1 = lhs.lower_/rhs.lower_;
        T v2 = lhs.lower_/rhs.upper_;
        T v3 = lhs.upper_/rhs.lower_;
        T v4 = lhs.upper_/rhs.upper_;
        return interval{ detail::_min(detail::_min(v1, v2), detail::_min(v3, v4)), detail::_max(detail::_max(v1, v2), detail::_max(v3, v4)) };

        //    // Implement functional division as per Hickey et al. (2001).
        //auto c1 = lhs._classify();
        //auto c2 = rhs._classify();
        //// TODO
    }
    [[nodiscard]] friend constexpr interval
    operator /(T lhs, interval const& rhs)
    {
        using std::isinf;

        gsl_ExpectsDebug(rhs._assigned());

        if (isinf(lhs) && (rhs.lower_ == -inf || rhs.upper_ == inf))  // x = ±∞ ∧ (c = -∞ ∨ d = ∞)
        {
                // ∞/∞ = NaN
            return interval{ nan, nan };
        }
        if (lhs == T(0) && rhs.lower_ <= T(0) && T(0) <= rhs.upper_)  // x = 0 ∧ c ≤ 0 ≤ d
        {
                // 0/0 = NaN
            return interval{ nan, nan };
        }
        if (lhs != T(0) && rhs.lower_ < T(0) && T(0) < rhs.upper_)  // x ≠ 0 ∧ c < 0 < d
        {
                // x/0 = [-∞,∞]
            return interval{ -inf, inf };
        }
        T v1 = lhs/rhs.lower_;
        T v2 = lhs/rhs.upper_;
        return interval{ detail::_min(v1, v2), detail::_max(v1, v2) };
    }
    [[nodiscard]] friend constexpr interval
    operator /(interval const& lhs, T rhs)
    {
        using std::isinf;

        gsl_ExpectsDebug(lhs._assigned());

        if ((lhs.lower_ == -inf || lhs.upper_ == inf) && isinf(rhs))  // (a = -∞ ∨ b = ∞) ∧ y = ±∞
        {
                // ∞/∞ = NaN
            return interval{ nan, nan };
        }
        if (lhs.lower_ <= T(0) && T(0) <= lhs.upper_ && rhs == T(0))  // a ≤ 0 ≤ b ∧ y = 0
        {
                // 0/0 = NaN
            return interval{ nan, nan };
        }
        T v1 = lhs.lower_/rhs;
        T v2 = lhs.upper_/rhs;
        return interval{ detail::_min(v1, v2), detail::_max(v1, v2) };
    }

    [[nodiscard]] friend constexpr interval
    square(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return interval{
            x.lower_ <= T{ } && x.upper_ >= T{ }  // 0 ∈ [a, b]
                ? T{ }
                : detail::_min(x.lower_*x.lower_, x.upper_*x.upper_),
            detail::_max(x.lower_*x.lower_, x.upper_*x.upper_)
        };
    }
    [[nodiscard]] friend constexpr interval
    sqrt(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return interval{ detail::_sqrt(x.lower_), detail::_sqrt(x.upper_) };
    }
    [[nodiscard]] friend constexpr interval
    cbrt(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return interval{ detail::_cbrt(x.lower_), detail::_cbrt(x.upper_) };
    }
    [[nodiscard]] friend constexpr interval
    log(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return interval{ detail::_log(x.lower_), detail::_log(x.upper_) };
    }
    [[nodiscard]] friend constexpr interval
    exp(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return interval{ detail::_exp(x.lower_), detail::_exp(x.upper_) };
    }
    [[nodiscard]] friend constexpr interval
    pow(interval const& x, interval const& y)
    {
        gsl_ExpectsDebug(x._assigned() && y._assigned());

        interval result;
        if (maybe(x >= T(0)))
        {
            result.assign(exp(_multiply_0(y, log(max(T(0), x)))));
        }
        if (maybe(x < T(0)))
        {
            if (y.lower_ == y.upper_ && gsl::narrow_cast<T>(gsl::narrow_cast<long long>(y.lower_) == y.lower_))  // y ∈ ℤ
            {
                auto yi = gsl::narrow_cast<long long>(y.lower_);
                int sign = yi % 2 == 0 ? 1 : -1;
                result.assign(sign*exp(_multiply_0(y.lower_, log(max(T(0), -x)))));
            }
            else
            {
                return interval{ nan, nan };
            }
        }
        return result;
    }
    [[nodiscard]] friend constexpr interval
    pow(interval const& x, T y)
    {
        gsl_ExpectsDebug(x._assigned());

        interval result;
        if (maybe(x >= T(0)))
        {
            result.assign(exp(_multiply_0(y, log(max(T(0), x)))));
        }
        if (maybe(x < T(0)))
        {
            if (gsl::narrow_cast<T>(gsl::narrow_cast<long long>(y) == y))  // y ∈ ℤ
            {
                auto yi = gsl::narrow_cast<long long>(y);
                int sign = yi % 2 == 0 ? 1 : -1;
                result.assign(sign*exp(_multiply_0(y, log(max(T(0), -x)))));
            }
            else
            {
                return interval{ nan, nan };
            }
        }
        return result;
    }
    [[nodiscard]] friend constexpr interval
    pow(T x, interval const& y)
    {
        gsl_ExpectsDebug(y._assigned());

        if (x >= T(0))
        {
            return exp(_multiply_0(detail::_log(detail::_max(T(0), x)), y));
        }
        else // x < T(0)
        {
            if (y.lower_ == y.upper_ && gsl::narrow_cast<T>(gsl::narrow_cast<long long>(y.lower_) == y.lower_))  // y ∈ ℤ
            {
                auto yi = gsl::narrow_cast<long long>(y.lower_);
                int sign = yi % 2 == 0 ? 1 : -1;
                return interval(sign*detail::_exp(_multiply_0(y.lower_, detail::_log(detail::_max(T(0), -x)))));
            }
            else
            {
                return interval{ nan, nan };
            }
        }
    }

    [[nodiscard]] friend constexpr interval
    cos(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        auto lo = intervals::wraparound(x.lower_, T(-pi), T(pi));
        auto delta = lo - x.lower_;
        auto hi = x.upper_ + delta;
        if (lo <= T(0))
        {
            if (hi <= T(0))
            {
                return interval{ detail::_cos(lo), detail::_cos(hi) };
            }
            else if (hi <= T(pi))
            {
                return interval{ detail::_min(detail::_cos(lo), detail::_cos(hi)), T(1) };
            }
        }
        else  // 0 < lo < π
        {
            if (hi <= T(pi))
            {
                return interval{ detail::_cos(hi), detail::_cos(lo) };
            }
            else if (hi <= T(2*pi))
            {
                return interval{ T(-1), detail::_max(detail::_cos(lo), detail::_cos(hi)) };
            }
        }
        return interval{ T(-1), T(1) };
    }
    [[nodiscard]] friend constexpr interval
    sin(interval const& x)
    {
        return cos(x - T(pi/2));
    }
    [[nodiscard]] friend constexpr interval
    tan(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        auto lo = intervals::wraparound(x.lower_, T(-pi/2), T(pi/2));
        auto delta = lo - x.lower_;
        auto hi = x.upper_ + delta;
        if (hi - lo >= T(pi))
        {
            return interval{ -inf, inf };
        }
        return interval{ detail::_tan(lo), detail::_tan(hi) };
    }
    [[nodiscard]] friend constexpr interval
    acos(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return interval{ detail::_acos(x.upper_), detail::_acos(x.lower_) };
    }
    [[nodiscard]] friend constexpr interval
    asin(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return interval{ detail::_asin(x.lower_), detail::_asin(x.upper_) };
    }
    [[nodiscard]] friend constexpr interval
    atan(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return interval{ detail::_atan(x.lower_), detail::_atan(x.upper_) };
    }
    [[nodiscard]] friend constexpr interval
    atan2(interval const& y, interval const& x)
    {
        gsl_ExpectsDebug(y._assigned() && x._assigned());

        if (x.lower_ <= T(0) && y.contains(T(0)))
        {
            return interval{ nan, nan };
        }
        auto v1 = detail::_atan2(y.lower_, x.lower_);
        auto v2 = detail::_atan2(y.lower_, x.upper_);
        auto v3 = detail::_atan2(y.upper_, x.lower_);
        auto v4 = detail::_atan2(y.upper_, x.upper_);
        return interval{ detail::_min(detail::_min(v1, v2), detail::_min(v3, v4)), detail::_max(detail::_max(v1, v2), detail::_max(v3, v4)) };
    }
    [[nodiscard]] friend constexpr interval
    atan2(interval const& y, T x)
    {
        gsl_ExpectsDebug(y._assigned());

        if (x <= T(0) && y.contains(T(0)))
        {
            return interval{ nan, nan };
        }
        auto v1 = detail::_atan2(y.lower_, x);
        auto v2 = detail::_atan2(y.upper_, x);
        return interval{ detail::_min(v1, v2), detail::_max(v1, v2) };
    }
    [[nodiscard]] friend constexpr interval
    atan2(T y, interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        if (x.lower_ <= T(0) && y == T(0))
        {
            return interval{ nan, nan };
        }
        auto v1 = detail::_atan2(y, x.lower_);
        auto v2 = detail::_atan2(y, x.upper_);
        return interval{ detail::_min(v1, v2), detail::_max(v1, v2) };
    }

    [[nodiscard]] friend constexpr interval
    floor(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return interval{ detail::_floor(x.lower_), detail::_floor(x.upper_) };
    }
    [[nodiscard]] friend constexpr interval
    ceil(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return interval{ detail::_ceil(x.lower_), detail::_ceil(x.upper_) };
    }
    [[nodiscard]] friend constexpr interval
    frac(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        T lfloor = detail::_floor(x.lower_);
        T ufloor = detail::_floor(x.upper_);
        if (lfloor != ufloor)
        {
            return interval{ T(0), T(1) };
        }
        return interval{ x.lower_ - lfloor, x.upper_ - ufloor };
    }

    [[nodiscard]] friend constexpr std::pair<interval, interval>
    fractional_weights(interval a, interval b)
    {
        gsl_ExpectsDebug(a._assigned() && b._assigned());
        gsl_ExpectsDebug((a >= 0).matches(true));
        gsl_ExpectsDebug((b >= 0).matches(true));
        gsl_ExpectsDebug((a + b > 0).matches(true));

        return {
            interval{ a.lower_/(a.lower_ + b.upper_), a.upper_/(a.upper_ + b.lower_) },
            interval{ b.lower_/(a.upper_ + b.lower_), b.upper_/(a.lower_ + b.upper_) }
        };
    }

    [[nodiscard]] friend constexpr interval
    abs(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return x.lower_ <= T(0) && x.upper_ >= T(0)             // if  0 ∈ [a,b] :
            ? interval{ T(0), detail::_max(-x.lower_, x.upper_) }  //     → [0, max{-a,b}]
            : x.lower_ < T(0)                                   // else if  a,b < 0 :
            ? interval{ -x.upper_, -x.lower_ }                     //     → [-b,-a]
            : x;                                                // else → [a,b]
    }

    [[nodiscard]] friend constexpr set<sign>
    sgn(interval const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        auto result = set<sign>{ };
        if (maybe(x > T(0)))
        {
            result.assign(positiveSign);
        }
        if (maybe(x < T(0)))
        {
            result.assign(negativeSign);
        }
        if (maybe(x == T(0)))
        {
            result.assign(zeroSign);
        }
        return result;
    }

    [[nodiscard]] friend constexpr set<bool>
    isinf(interval const& x)
    {
        using std::isinf;

        gsl_ExpectsDebug(x._assigned());

        auto result = set<bool>{ };
        if (isinf(x.lower_) || isinf(x.upper_))
        {
            result.assign(true);
        }
        if (x.lower_ < x.upper_ || (x.lower_ == x.upper_ && !isinf(x.lower_)))
        {
            result.assign(false);
        }
        return result;
    }
    [[nodiscard]] friend constexpr set<bool>
    isfinite(interval const& x)
    {
        return !isinf(x);
    }
    [[nodiscard]] friend constexpr set<bool>
    isnan(interval const& x)
    {
        using std::isnan;

        gsl_ExpectsDebug(x._assigned());

        if (isnan(x.lower_) || isnan(x.upper_))
        {
            return set{ false, true };
        }
        return set{ false };
    }
};


template <typename ElemT, typename TraitsT, typename T>
std::basic_ostream<ElemT, TraitsT>&
operator <<(std::basic_ostream<ElemT, TraitsT>& stream, interval<T> const& x)
{
    gsl_ExpectsDebug(x._assigned());

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
get(interval<T> const& bound)
{
    static_assert(I < 2, "tuple index out of range");
    if constexpr (I == 0) return bound.lower();
    if constexpr (I == 1) return bound.upper();
}


template <detail::floating_point T>
[[nodiscard]] constexpr interval<T>
operator *(T lhs, set<sign> rhs)
{
    interval<T> result;
    if (rhs.contains(positiveSign))
    {
        result.assign(lhs);
    }
    if (rhs.contains(zeroSign))
    {
        result.assign(0);
    }
    if (rhs.contains(negativeSign))
    {
        result.assign(-lhs);
    }
    return result;
}
template <detail::floating_point T>
[[nodiscard]] constexpr interval<T>
operator *(set<sign> lhs, T rhs)
{
    return rhs*lhs;
}


template <detail::floating_point T>
[[nodiscard]] constexpr detail::member_assigner<interval<T>>
branch_value(interval<T>& x) noexcept
{
    return detail::member_assigner<interval<T>>(x);
}
template <detail::floating_point T>
[[nodiscard]] constexpr detail::member_resetter<interval<T>>
uniform_value(interval<T>& x) noexcept
{
    return detail::member_resetter<interval<T>>(x);
}

template <makeshift::tuple_like T>
[[nodiscard]] constexpr detail::tuple_assigner<T>
branch_value(T& x) noexcept
requires detail::non_const<T>
{
    return detail::tuple_assigner<T>(x);
}
template <makeshift::tuple_like T>
[[nodiscard]] constexpr detail::tuple_resetter<T>
uniform_value(T& x) noexcept
requires detail::non_const<T>
{
    return detail::tuple_resetter<T>(x);
}


} // namespace intervals


    // Implement tuple-like protocol for `interval<>`.
template <typename T> class std::tuple_size<intervals::interval<T>> : public std::integral_constant<std::size_t, 2> { };
template <std::size_t I, typename T> class std::tuple_element<I, intervals::interval<T>> { public: using type = T; };


#endif // INCLUDED_INTERVALS_INTERVAL_HPP_