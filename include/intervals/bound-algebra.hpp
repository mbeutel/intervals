
// Utilities for algebra with scalar bounds.


#ifndef INCLUDED_INTERVALS_INTERVAL_HPP_
#define INCLUDED_INTERVALS_INTERVAL_HPP_


#include <array>
#include <cmath>             // for sqrt()
#include <limits>
#include <bitset>
#include <iosfwd>
#include <cstddef>           // for size_t
#include <algorithm>         // for min(), max()
#include <initializer_list>

#include <makeshift/metadata.hpp>

#include <gsl-lite/gsl-lite.hpp>  // for type_identity<>, gsl_Expects(), gsl_ExpectsDebug(), gsl_FailFast()

#include <rpmc/tools/math.hpp>      // for sqrt(), cbrt(), log(), sgn(), Sign
#include <rpmc/tools/interval.hpp>  // for Interval<>


namespace rpmc {

namespace gsl = gsl_lite;

namespace detail {


template <typename T>
constexpr gsl::dim numValues = makeshift::metadata::values<T>(makeshift::metadata_v<T>).size();

template <typename T>
struct ValueStore
{
    static constexpr std::array<T, numValues<T>> values = makeshift::metadata::values<T>(makeshift::metadata_v<T>);
};
template <typename T>
constexpr std::array<T, numValues<T>> ValueStore<T>::values;

template <typename T>
constexpr gsl::index
findValueIndex(T value)
{
    for (gsl::index i = 0; i != numValues<T>; ++i)
    {
        if (ValueStore<T>::values[i] == value)
        {
            return i;
        }
    }
    gsl_FailFast();
}


template <typename T>
constexpr inline T
_sqrt(T const& x)
{
    using std::sqrt;
    return sqrt(x);
}
template <typename T>
constexpr inline T
_cbrt(T const& x)
{
    using std::cbrt;
    return cbrt(x);
}
template <typename T>
constexpr inline T
_exp(T const& x)
{
    using std::exp;
    return exp(x);
}
template <typename T>
constexpr inline T
_log(T const& x)
{
    using std::log;
    return log(x);
}
template <typename T>
constexpr inline T
_sin(T const& x)
{
    using std::sin;
    return sin(x);
}
template <typename T>
constexpr inline T
_cos(T const& x)
{
    using std::cos;
    return cos(x);
}
template <typename T>
constexpr inline T
_tan(T const& x)
{
    using std::tan;
    return tan(x);
}
template <typename T>
constexpr inline T
_asin(T const& x)
{
    using std::asin;
    return asin(x);
}
template <typename T>
constexpr inline T
_acos(T const& x)
{
    using std::acos;
    return acos(x);
}
template <typename T>
constexpr inline T
_atan(T const& x)
{
    using std::atan;
    return atan(x);
}
template <typename T>
constexpr inline T
_atan2(T const& y, T const& x)
{
    using std::atan2;
    return atan2(y, x);
}
//[[nodiscard]] constexpr inline float
//_min(float x, float y)
//{
//    return std::min(x, y);
//    //return x <= y ? x
//    //     : x > y ? y
//    //     : std::numeric_limits<float>::quiet_NaN();
//}
//[[nodiscard]] constexpr inline double
//_min(double x, double y)
//{
//    return std::min(x, y);
//    //return x <= y ? x
//    //     : x > y ? y
//    //     : std::numeric_limits<double>::quiet_NaN();
//}
template <typename T>
constexpr inline T
_min(T x, T y)
{
    using std::min;
    return min(x, y);
}
//[[nodiscard]] constexpr inline float
//_max(float x, float y)
//{
//    return x >= y ? x
//         : x < y ? y
//         : std::numeric_limits<float>::quiet_NaN();
//}
//[[nodiscard]] constexpr double
//_max(double x, double y)
//{
//    return x >= y ? x
//         : x < y ? y
//         : std::numeric_limits<double>::quiet_NaN();
//}
template <typename T>
constexpr inline T
_max(T x, T y)
{
    using std::max;
    return max(x, y);
}
template <typename T>
constexpr inline T
_floor(T const& x)
{
    using std::floor;
    return floor(x);
}
template <typename T>
constexpr inline T
_ceil(T const& x)
{
    using std::ceil;
    return ceil(x);
}
template <typename T>
constexpr inline T
_frac(T const& x)
{
    using std::floor;
    return x - floor(x);
}


// The 4-valued extension of the 3-valued Kleene logic represented by `Set<bool>` has the following states:
//
//     0 : uninitialized
//     1 : false
//     2 : true
//     3 : both
//
// The uninitialized state is infectious, i.e. every function of an undefined value yields an undefined value.
// (TODO: do we want infection traps? do we want them to be configurable?)
//
// Truth tables:
//
//      ⋀  |     y           ⋁  |     y           ⊻  |     y          ¬   |         =  |     y           ≠  |     y     
//         | 0  1  2  3         | 0  1  2  3         | 0  1  2  3     ————+———         | 0  1  2  3         | 0  1  2  3
//     ————+————————————    ————+————————————    ————+————————————      0 | 0 —    ————+————————————    ————+———————————
//       0 | 0  0  0  0       0 | 0  0  0  0       0 | 0  0  0  0     x 1 | 2        0 | 0  0  0  0       0 | 0  0  0  0
//     x 1 | 0  1  1  1     x 1 | 0  1  2  3     x 1 | 0  1  2  3       2 | 1      x 1 | 0  2  1  3     x 1 | 0  1  2  3
//       2 | 0  1  2  3       2 | 0  2  2  2       2 | 0  2  1  3       3 | 3        2 | 0  1  2  3       2 | 0  2  1  3
//       3 | 0  1  3  3       3 | 0  3  2  3       3 | 0  3  3  3                    3 | 0  3  3  3       3 | 0  3  3  3
//
// The following constants are essentially 32-bit look-up tables for truth values, which we use for efficient implementation
// of 4-valued logic.
//
//                                  y: |     3     |     2     |     1     |     0     |
//                                  x: | 3  2  1  0| 3  2  1  0| 3  2  1  0| 3  2  1  0|
constexpr std::uint32_t lut_4vK_and = 0b11'11'01'00'11'10'01'00'01'01'01'00'00'00'00'00u;
constexpr std::uint32_t lut_4vK_or  = 0b11'10'11'00'10'10'10'00'11'10'01'00'00'00'00'00u;
constexpr std::uint32_t lut_4vK_xor = 0b11'11'11'00'11'01'10'00'11'10'01'00'00'00'00'00u;
constexpr std::uint32_t lut_4vK_eq  = 0b11'11'11'00'11'10'01'00'11'01'10'00'00'00'00'00u;
constexpr std::uint32_t lut_4vK_neq = 0b11'11'11'00'11'01'10'00'11'10'01'00'00'00'00'00u;  // `== lut_4vK_xor`
//                                  x: | 3  2  1  0|
constexpr std::uint8_t  lut_4vK_not = 0b11'01'10'00u;


}  // namespace detail


    // Set of discrete values over a bounded type.
    // TODO: we should somehow handle flag enums here; a set of a flag enum is essentially the flag enum itself, albeit with
    // set-like assignment/comparison semantics.
template <typename T>
class Set
{
    template <typename T2>
    friend class Set;

private:
    std::bitset<detail::numValues<T>> state_;

    explicit Set(std::bitset<detail::numValues<T>> _state)
        : state_(_state)
    {
    }

public:
    constexpr Set()
    {
    }
    constexpr Set(T value)
    {
        state_.set(detail::findValueIndex(value));
    }
    explicit constexpr Set(std::initializer_list<T> values)
    {
        for (T value : values)
        {
            state_.set(detail::findValueIndex(value));
        }
    }
    static constexpr Set
    from_bits(unsigned long long bits)
    {
        return Set(std::bitset<detail::numValues<T>>(bits));
    }

    constexpr Set&
    reset()
    {
        state_ = { };
        return *this;
    }
    constexpr Set&
    reset(Set const& rhs)
    {
        state_ = rhs.state_;
        return *this;
    }

    constexpr bool
    assigned() const
    {
        return state_.any();
    }

    Set(Set const&) = default;
    Set& operator =(Set const&) = delete;
    constexpr Set&
    assign(Set const& rhs)
    {
        gsl_ExpectsDebug(rhs.assigned());
    
        state_ |= rhs.state_;
        return *this;
    }
    constexpr Set&
    assign(T value)
    {
        state_.set(detail::findValueIndex(value));
        return *this;
    }

    constexpr unsigned long long
    to_bits() const
    {
        return state_.to_ullong();
    }

    [[nodiscard]] constexpr bool
    contains(T value) const
    {
        return state_.test(detail::findValueIndex(value));
    }
    [[nodiscard]] constexpr bool
    contains(Set<T> const& set) const
    {
        return (state_ & set.state_) == set.state_;
    }
    [[nodiscard]] constexpr bool
    matches(T value) const
    {
        return matches(Set(value));
    }
    [[nodiscard]] constexpr bool
    matches(Set<T> const& set) const
    {
        return state_ == set.state_;
    }
    [[nodiscard]] constexpr T
    value() const
    {
        gsl_ExpectsDebug(assigned());

        auto lstate = state_;
        for (gsl::index i = 0; i < detail::numValues<T>; ++i)
        {
            bool bit0 = lstate.test(0);
            lstate >>= 1;
            if (bit0)
            {
                if (!lstate.any())
                {
                    return detail::ValueStore<T>::values[i];
                }
                break;
            }
        }
        gsl_FailFast();
    }

    [[nodiscard]] friend constexpr Set<bool>
    operator ==(Set const& lhs, Set const& rhs)
    {
        gsl_ExpectsDebug(lhs.assigned() && rhs.assigned());

        auto result = Set<bool>{ };
        bool anyMatch = (lhs.state_ & rhs.state_).any();
        if (anyMatch)
        {
            result.assign(true);
        }
        auto cl = lhs.state_.count();
        auto cr = rhs.state_.count();
        if (cl > 1 || cr > 1 || (cl == 1 && cr == 1 && !anyMatch))
        {
            result.assign(false);
        }
        return result;
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator !=(Set const& lhs, Set const& rhs)
    {
        gsl_ExpectsDebug(lhs.assigned() && rhs.assigned());

        auto result = Set<bool>{ };
        auto equal = lhs == rhs;
        if (equal.contains(true))
        {
            result.assign(false);
        }
        if (equal.contains(false))
        {
            result.assign(true);
        }
        return result;
    }
};
[[nodiscard]] constexpr Set<bool>
operator !(Set<bool> x)
{
    gsl_ExpectsDebug(x.assigned());

    return Set<bool>::from_bits((detail::lut_4vK_not >> (2*x.to_bits())) & 0b11u);
}

    // Instead of short-circuiting Boolean operators `&&` and `||`, use operators `&` and `|` for `Set<bool>`, and use `maybe()`,
    // `maybe_not()`, `definitely()`, `definitely_not()` to evaluate a `Set<bool>` as Boolean.
Set<bool> operator &&(Set<bool>, Set<bool>) = delete;
Set<bool> operator &&(bool, Set<bool>) = delete;
Set<bool> operator &&(Set<bool>, bool) = delete;
Set<bool> operator ||(Set<bool>, Set<bool>) = delete;
Set<bool> operator ||(bool, Set<bool>) = delete;
Set<bool> operator ||(Set<bool>, bool) = delete;

[[nodiscard]] constexpr Set<bool>
operator &(Set<bool> x, Set<bool> y)
{
    gsl_ExpectsDebug(x.assigned() && y.assigned());

    return Set<bool>::from_bits((detail::lut_4vK_and >> (2*x.to_bits() + 8*y.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr Set<bool>
operator &(Set<bool> x, bool y)
{
    gsl_ExpectsDebug(x.assigned());

    return Set<bool>::from_bits((detail::lut_4vK_and >> (2*x.to_bits() + 8*(1 + int(y)))) & 0b11u);
}
[[nodiscard]] constexpr Set<bool>
operator &(bool x, Set<bool> y)
{
    gsl_ExpectsDebug(y.assigned());

    return Set<bool>::from_bits((detail::lut_4vK_and >> (2*y.to_bits() + 8*(1 + int(x)))) & 0b11u);
}
[[nodiscard]] constexpr Set<bool>
operator |(Set<bool> x, Set<bool> y)
{
    gsl_ExpectsDebug(x.assigned() && y.assigned());

    return Set<bool>::from_bits((detail::lut_4vK_or >> (2*x.to_bits() + 8*y.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr Set<bool>
operator |(Set<bool> x, bool y)
{
    gsl_ExpectsDebug(x.assigned());

    return Set<bool>::from_bits((detail::lut_4vK_or >> (2*x.to_bits() + 8*(1 + int(y)))) & 0b11u);
}
[[nodiscard]] constexpr Set<bool>
operator |(bool x, Set<bool> y)
{
    gsl_ExpectsDebug(y.assigned());

    return Set<bool>::from_bits((detail::lut_4vK_or >> (2*y.to_bits() + 8*(1 + int(x)))) & 0b11u);
}
[[nodiscard]] constexpr Set<bool>
operator ^(Set<bool> x, Set<bool> y)
{
    gsl_ExpectsDebug(x.assigned() && y.assigned());

    return Set<bool>::from_bits((detail::lut_4vK_xor >> (2*x.to_bits() + 8*y.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr Set<bool>
operator ^(Set<bool> x, bool y)
{
    gsl_ExpectsDebug(x.assigned());

    return Set<bool>::from_bits((detail::lut_4vK_xor >> (2*x.to_bits() + 8*(1 + int(y)))) & 0b11u);
}
[[nodiscard]] constexpr Set<bool>
operator ^(bool x, Set<bool> y)
{
    gsl_ExpectsDebug(y.assigned());

    return Set<bool>::from_bits((detail::lut_4vK_xor >> (2*y.to_bits() + 8*(1 + int(x)))) & 0b11u);
}

[[nodiscard]] constexpr bool
maybe(Set<bool> x) noexcept
{
    return x.contains(true);
}
[[nodiscard]] constexpr bool
maybe_not(Set<bool> x) noexcept
{
    return x.contains(false);
}
[[nodiscard]] constexpr bool
definitely(Set<bool> x) noexcept
{
    return x.matches(true);
}
[[nodiscard]] constexpr bool
definitely_not(Set<bool> x) noexcept
{
    return x.matches(false);
}
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


    // Algebraic type representing a bounded scalar.
template <typename T>
class Bound
{
    static constexpr T inf = std::numeric_limits<T>::infinity();
    static constexpr T nan = std::numeric_limits<T>::quiet_NaN();

private:
    T lower_;
    T upper_;

        // Given an interval  [a,b] , determines the interval classification as per Hickey et al. (2001).
    enum class HickeyClassification : unsigned
    {
        Z  = 0,  // a = 0 ∧ b = 0
        M  = 1,  // a < 0 ∧ b > 0
        P0 = 2,  // a = 0 ∧ b > 0
        P1 = 3,  // a > 0 ∧ b > 0
        N0 = 4,  // a < 0 ∧ b = 0
        N1 = 5   // a < 0 ∧ b < 0
    };

    HickeyClassification
    _classify() const
    {
        gsl_Assert(lower_ <= upper_);  // this traps NaNs and uninitialized states

        if (lower_ < T{ })
        {
            if (upper_ > T{ }) return HickeyClassification::M;
            else if (upper_ == T{ }) return HickeyClassification::N0;
            else return HickeyClassification::N1;
        }
        else if (lower_ == T{ })
        {
            if (upper_ == T{ }) return HickeyClassification::Z;
            else return HickeyClassification::P0;
        }
        else return HickeyClassification::P1;
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
        T result = x*y;
        if (std::isnan(result) && (std::isinf(x) && y == T(0.) || std::isinf(y) && x == T(0)))
        {
            return T(0);
        }
        return result;
    }
    static constexpr Bound
    _multiply_0(Bound const& lhs, Bound const& rhs)
    {
        T v1 = _multiply_0(lhs.lower_, rhs.lower_);
        T v2 = _multiply_0(lhs.lower_, rhs.upper_);
        T v3 = _multiply_0(lhs.upper_, rhs.lower_);
        T v4 = _multiply_0(lhs.upper_, rhs.upper_);
        return Bound{ detail::_min(detail::_min(v1, v2), detail::_min(v3, v4)), detail::_max(detail::_max(v1, v2), detail::_max(v3, v4)) };
    }
    static constexpr Bound
    _multiply_0(T lhs, Bound const& rhs)
    {
        T v1 = _multiply_0(lhs, rhs.lower_);
        T v2 = _multiply_0(lhs, rhs.upper_);
        return Bound{ detail::_min(v1, v2), detail::_max(v1, v2) };
    }


public:
    constexpr Bound()
        : lower_(inf), upper_(-inf)
    {
    }
    constexpr Bound(T value)
        : lower_(value), upper_(value)
    {
    }
    explicit constexpr Bound(T _lower, T _upper)
        : lower_(_lower), upper_(_upper)
    {
        gsl_Expects(!(_lower > _upper));  // does not trigger for NaNs
    }

    Bound(Bound const&) = default;
    Bound& operator =(Bound const&) = delete;
    constexpr Bound&
    assign(Bound const& rhs)
    {
        gsl_ExpectsDebug(rhs._assigned());

        lower_ = detail::_min(lower_, rhs.lower_);
        upper_ = detail::_max(upper_, rhs.upper_);
        return *this;
    }
    constexpr Bound&
    assign(T value)
    {
        lower_ = detail::_min(lower_, value);
        upper_ = detail::_max(upper_, value);
        return *this;
    }

    constexpr Bound&
    reset()
    {
        lower_ = inf;
        upper_ = -inf;
        return *this;
    }
    constexpr Bound&
    reset(Bound const& rhs)
    {
        lower_ = rhs.lower_;
        upper_ = rhs.upper_;
        return *this;
    }

    [[nodiscard]] constexpr T
    lowerUnchecked() const
    {
        return lower_;
    }
    [[nodiscard]] constexpr T
    upperUnchecked() const
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
    contains(Bound const& rhs) const
    {
        return _assigned() && rhs.lower_ >= lower_ && rhs.upper_ <= upper_;
    }
    [[nodiscard]] constexpr bool
    encloses(T value) const
    {
        return lower_ < value && value < upper_;
    }
    [[nodiscard]] constexpr bool
    encloses(Bound const& rhs) const
    {
        return rhs.lower_ > lower_ && rhs.upper_ < upper_;
    }
    [[nodiscard]] constexpr bool
    matches(T value) const
    {
        return lower_ == value && upper_ == value;
    }
    [[nodiscard]] constexpr bool
    matches(Bound const& rhs) const
    {
        return lower_ == rhs.lower_ && upper_ == rhs.upper_;
    }

    [[nodiscard]] constexpr Interval<T>
    asInterval() const
    {
        gsl_ExpectsDebug(_assigned());

        return Interval{ lower_, upper_ };
    }

    // TODO: we could put more `Interval<>` member functions here, e.g. `overlapsWith()`, `overlapInterval()`, but let's see if
    // we actually want them here, or if we'd rather keep the two different semantics separate.

    [[nodiscard]] friend constexpr Set<bool>
    operator ==(Bound const& lhs, Bound const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        if (lhs.matches(rhs))
        {
            return Set{ true };
        }
        else if (rhs.upper_ >= lhs.lower_ && rhs.lower_ <= lhs.upper_)  // overlapping intervals
        {
            return Set{ false, true };
        }
        else
        {
            return Set{ false };
        }
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator ==(Bound const& lhs, T rhs)
    {
        gsl_ExpectsDebug(lhs._assigned());

        if (lhs.matches(rhs))
        {
            return Set{ true };
        }
        else if (lhs.contains(rhs))  // overlapping intervals
        {
            return Set{ false, true };
        }
        else
        {
            return Set{ false };
        }
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator ==(T lhs, Bound const& rhs)
    {
        return rhs == lhs;
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator !=(Bound const& lhs, Bound const& rhs)
    {
        return !(lhs == rhs);
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator !=(Bound const& lhs, T rhs)
    {
        return !(lhs == rhs);
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator !=(T lhs, Bound const& rhs)
    {
        return !(rhs == lhs);
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator <(Bound const& lhs, Bound const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        auto result = Set<bool>{ };
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
    [[nodiscard]] friend constexpr Set<bool>
    operator <(Bound const& lhs, T rhs)
    {
        return lhs < Bound(rhs);
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator <(T lhs, Bound const& rhs)
    {
        return Bound(lhs) < rhs;
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator >(Bound const& lhs, Bound const& rhs)
    {
        return rhs < lhs;
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator >(Bound const& lhs, T rhs)
    {
        return rhs < lhs;
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator >(T lhs, Bound const& rhs)
    {
        return rhs < lhs;
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator <=(Bound const& lhs, Bound const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        auto result = Set<bool>{ };
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
    [[nodiscard]] friend constexpr Set<bool>
    operator <=(Bound const& lhs, T rhs)
    {
        return lhs <= Bound(rhs);
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator <=(T lhs, Bound const& rhs)
    {
        return Bound(lhs) <= rhs;
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator >=(Bound const& lhs, Bound const& rhs)
    {
        return rhs <= lhs;
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator >=(Bound const& lhs, T rhs)
    {
        return Bound(rhs) <= lhs;
    }
    [[nodiscard]] friend constexpr Set<bool>
    operator >=(T lhs, Bound const& rhs)
    {
        return rhs <= Bound(lhs);
    }

    [[nodiscard]] friend constexpr Bound
    min(Bound const& lhs, Bound const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        return Bound{ detail::_min(lhs.lower_, rhs.lower_), detail::_min(lhs.upper_, rhs.upper_) };
    }
    [[nodiscard]] friend constexpr Bound
    min(Bound const& lhs, T rhs)
    {
        gsl_ExpectsDebug(lhs._assigned());

        return Bound{ detail::_min(lhs.lower_, rhs), detail::_min(lhs.upper_, rhs) };
    }
    [[nodiscard]] friend constexpr Bound
    min(T lhs, Bound const& rhs)
    {
        return min(rhs, lhs);
    }
    [[nodiscard]] friend constexpr Bound
    max(Bound const& lhs, Bound const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        return Bound{ detail::_max(lhs.lower_, rhs.lower_), detail::_max(lhs.upper_, rhs.upper_) };
    }
    [[nodiscard]] friend constexpr Bound
    max(Bound const& lhs, T rhs)
    {
        gsl_ExpectsDebug(lhs._assigned());

        return Bound{ detail::_max(lhs.lower_, rhs), detail::_max(lhs.upper_, rhs) };
    }
    [[nodiscard]] friend constexpr Bound
    max(T lhs, Bound const& rhs)
    {
        return max(rhs, lhs);
    }

    [[nodiscard]] constexpr Bound
    operator +() const
    {
        gsl_ExpectsDebug(_assigned());

        return *this;
    }
    [[nodiscard]] constexpr Bound
    operator -() const
    {
        gsl_ExpectsDebug(_assigned());

        return Bound{ -upper_, -lower_ };
    }

    [[nodiscard]] friend constexpr Bound
    operator +(Bound const& lhs, Bound const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        if ((lhs.lower_ == -inf && rhs.upper_ == inf)
            || ((rhs.lower_ == -inf && lhs.upper_ == inf)))
        {
            return Bound{ nan, nan };
        }
        return Bound{ lhs.lower_ + rhs.lower_, lhs.upper_ + rhs.upper_ };
    }
    [[nodiscard]] friend constexpr Bound
    operator +(T lhs, Bound const& rhs)
    {
        return Bound(lhs) + rhs;
    }
    [[nodiscard]] friend constexpr Bound
    operator +(Bound const& lhs, T rhs)
    {
        return lhs + Bound(rhs);
    }
    [[nodiscard]] friend constexpr Bound
    operator -(Bound const& lhs, Bound const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        if ((lhs.lower_ == -inf && rhs.lower_ == -inf)
            || ((lhs.upper_ == inf && rhs.upper_ == inf)))
        {
            return Bound{ nan, nan };
        }
        return Bound{ lhs.lower_ - rhs.upper_, lhs.upper_ - rhs.lower_ };
    }
    [[nodiscard]] friend constexpr Bound
    operator -(T lhs, Bound const& rhs)
    {
        gsl_ExpectsDebug(rhs._assigned());

        return Bound{ lhs - rhs.upper_, lhs - rhs.lower_ };
    }
    [[nodiscard]] friend constexpr Bound
    operator -(Bound const& lhs, T rhs)
    {
        gsl_ExpectsDebug(lhs._assigned());

        return Bound{ lhs.lower_ - rhs, lhs.upper_ - rhs };
    }
    [[nodiscard]] friend constexpr Bound
    operator *(Bound const& lhs, Bound const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());

        if (((lhs.lower_ == -inf || lhs.upper_ == inf) && rhs.contains(T(0)))
            || (lhs.contains(T(0)) && (rhs.lower_ == -inf || rhs.upper_ == inf)))
        {
            return Bound{ nan, nan };
        }
        T v1 = lhs.lower_*rhs.lower_;
        T v2 = lhs.lower_*rhs.upper_;
        T v3 = lhs.upper_*rhs.lower_;
        T v4 = lhs.upper_*rhs.upper_;
        return Bound{ detail::_min(detail::_min(v1, v2), detail::_min(v3, v4)), detail::_max(detail::_max(v1, v2), detail::_max(v3, v4)) };
    }
    [[nodiscard]] friend constexpr Bound
    operator *(T lhs, Bound const& rhs)
    {
        gsl_ExpectsDebug(rhs._assigned());

        if (((lhs == -inf || lhs == inf) && rhs.contains(T(0)))
            || (lhs == T(0) && (rhs.lower_ == -inf || rhs.upper_ == inf)))
        {
            return Bound{ nan, nan };
        }
        T v1 = lhs*rhs.lower_;
        T v2 = lhs*rhs.upper_;
        return Bound{ detail::_min(v1, v2), detail::_max(v1, v2) };
    }
    [[nodiscard]] friend constexpr Bound
    operator *(Bound const& lhs, T rhs)
    {
        return rhs*lhs;
    }
    [[nodiscard]] friend constexpr Bound
    operator *(Sign lhs, Bound const& rhs)
    {
        return int(lhs)*rhs;
    }
    [[nodiscard]] friend constexpr Bound
    operator *(Bound const& lhs, Sign rhs)
    {
        return lhs*int(rhs);
    }
    [[nodiscard]] friend constexpr Bound
    operator *(Set<Sign> lhs, Bound const& rhs)
    {
        Bound result;
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
    [[nodiscard]] friend constexpr Bound
    operator *(Bound const& lhs, Set<Sign> rhs)
    {
        return rhs*lhs;
    }
    [[nodiscard]] friend constexpr Bound
    operator /(Bound const& lhs, Bound const& rhs)
    {
        gsl_ExpectsDebug(lhs._assigned() && rhs._assigned());
        
        if ((lhs.lower_ == -inf || lhs.upper_ == inf)  // a = -∞ ∨ b = ∞
            && (rhs.lower_ == -inf || rhs.upper_ == inf))  // c = -∞ ∨ d = ∞
        {
                // ∞/∞ = NaN
            return Bound{ nan, nan };
        }
        if (lhs.lower_ <= T(0) && T(0) <= lhs.upper_ && rhs.lower_ <= T(0) && T(0) <= rhs.upper_)  // a ≤ 0 ≤ b ∧ c ≤ 0 ≤ d
        {
                // 0/0 = NaN
            return Bound{ nan, nan };
        }
        if ((T(0) < lhs.lower_ || lhs.upper_ < T(0)) && rhs.lower_ < T(0) && T(0) < rhs.upper_)  // 0 ∉ [a,b] ∧ c < 0 < d
        {
                // [a,b]/0 = [-∞,∞]
            return Bound{ -inf, inf };
        }
        T v1 = lhs.lower_/rhs.lower_;
        T v2 = lhs.lower_/rhs.upper_;
        T v3 = lhs.upper_/rhs.lower_;
        T v4 = lhs.upper_/rhs.upper_;
        return Bound{ detail::_min(detail::_min(v1, v2), detail::_min(v3, v4)), detail::_max(detail::_max(v1, v2), detail::_max(v3, v4)) };

        //    // Implement functional division as per Hickey et al. (2001).
        //auto c1 = lhs._classify();
        //auto c2 = rhs._classify();
        //// TODO
    }
    [[nodiscard]] friend constexpr Bound
    operator /(T lhs, Bound const& rhs)
    {
        gsl_ExpectsDebug(rhs._assigned());

        if (std::isinf(lhs) && (rhs.lower_ == -inf || rhs.upper_ == inf))  // x = ±∞ ∧ (c = -∞ ∨ d = ∞)
        {
                // ∞/∞ = NaN
            return Bound{ nan, nan };
        }
        if (lhs == T(0) && rhs.lower_ <= T(0) && T(0) <= rhs.upper_)  // x = 0 ∧ c ≤ 0 ≤ d
        {
                // 0/0 = NaN
            return Bound{ nan, nan };
        }
        if (lhs != T(0) && rhs.lower_ < T(0) && T(0) < rhs.upper_)  // x ≠ 0 ∧ c < 0 < d
        {
                // x/0 = [-∞,∞]
            return Bound{ -inf, inf };
        }
        T v1 = lhs/rhs.lower_;
        T v2 = lhs/rhs.upper_;
        return Bound{ detail::_min(v1, v2), detail::_max(v1, v2) };
    }
    [[nodiscard]] friend constexpr Bound
    operator /(Bound const& lhs, T rhs)
    {
        gsl_ExpectsDebug(lhs._assigned());

        if ((lhs.lower_ == -inf || lhs.upper_ == inf) && std::isinf(rhs))  // (a = -∞ ∨ b = ∞) ∧ y = ±∞
        {
                // ∞/∞ = NaN
            return Bound{ nan, nan };
        }
        if (lhs.lower_ <= T(0) && T(0) <= lhs.upper_ && rhs == T(0))  // a ≤ 0 ≤ b ∧ y = 0
        {
                // 0/0 = NaN
            return Bound{ nan, nan };
        }
        T v1 = lhs.lower_/rhs;
        T v2 = lhs.upper_/rhs;
        return Bound{ detail::_min(v1, v2), detail::_max(v1, v2) };
    }

    [[nodiscard]] friend constexpr Bound
    square(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return Bound{
            x.lower_ <= T{ } && x.upper_ >= T{ }  // 0 ∈ [a, b]
                ? T{ }
                : std::min(x.lower_*x.lower_, x.upper_*x.upper_),
            std::max(x.lower_*x.lower_, x.upper_*x.upper_)
        };
    }
    [[nodiscard]] friend constexpr Bound
    sqrt(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return Bound{ detail::_sqrt(x.lower_), detail::_sqrt(x.upper_) };
    }
    [[nodiscard]] friend constexpr Bound
    cbrt(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return Bound{ detail::_cbrt(x.lower_), detail::_cbrt(x.upper_) };
    }
    [[nodiscard]] friend constexpr Bound
    log(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return Bound{ detail::_log(x.lower_), detail::_log(x.upper_) };
    }
    [[nodiscard]] friend constexpr Bound
    exp(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return Bound{ detail::_exp(x.lower_), detail::_exp(x.upper_) };
    }
    [[nodiscard]] friend constexpr Bound
    pow(Bound const& x, Bound const& y)
    {
        gsl_ExpectsDebug(x._assigned() && y._assigned());

        Bound result;
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
                return Bound{ nan, nan };
            }
        }
        return result;
    }
    [[nodiscard]] friend constexpr Bound
    pow(Bound const& x, T y)
    {
        gsl_ExpectsDebug(x._assigned());

        Bound result;
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
                return Bound{ nan, nan };
            }
        }
        return result;
    }
    [[nodiscard]] friend constexpr Bound
    pow(T x, Bound const& y)
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
                return Bound(sign*detail::_exp(_multiply_0(y.lower_, detail::_log(detail::_max(T(0), -x)))));
            }
            else
            {
                return Bound{ nan, nan };
            }
        }
    }

    [[nodiscard]] friend constexpr Bound
    cos(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        auto lo = rpmc::wraparound(x.lower_, T(-pi), T(pi));
        auto delta = lo - x.lower_;
        auto hi = x.upper_ + delta;
        if (lo <= T(0))
        {
            if (hi <= T(0))
            {
                return Bound{ detail::_cos(lo), detail::_cos(hi) };
            }
            else if (hi <= T(pi))
            {
                return Bound{ detail::_min(detail::_cos(lo), detail::_cos(hi)), T(1) };
            }
        }
        else  // 0 < lo < π
        {
            if (hi <= T(pi))
            {
                return Bound{ detail::_cos(hi), detail::_cos(lo) };
            }
            else if (hi <= T(2*pi))
            {
                return Bound{ T(-1), detail::_max(detail::_cos(lo), detail::_cos(hi)) };
            }
        }
        return Bound{ T(-1), T(1) };
    }
    [[nodiscard]] friend constexpr Bound
    sin(Bound const& x)
    {
        return cos(x - T(pi/2));
    }
    [[nodiscard]] friend constexpr Bound
    tan(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        auto lo = rpmc::wraparound(x.lower_, T(-pi/2), T(pi/2));
        auto delta = lo - x.lower_;
        auto hi = x.upper_ + delta;
        if (hi - lo >= T(pi))
        {
            return Bound{ -inf, inf };
        }
        return Bound{ detail::_tan(lo), detail::_tan(hi) };
    }
    [[nodiscard]] friend constexpr Bound
    acos(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return Bound{ detail::_acos(x.upper_), detail::_acos(x.lower_) };
    }
    [[nodiscard]] friend constexpr Bound
    asin(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return Bound{ detail::_asin(x.lower_), detail::_asin(x.upper_) };
    }
    [[nodiscard]] friend constexpr Bound
    atan(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return Bound{ detail::_atan(x.lower_), detail::_atan(x.upper_) };
    }
    [[nodiscard]] friend constexpr Bound
    atan2(Bound const& y, Bound const& x)
    {
        gsl_ExpectsDebug(y._assigned() && x._assigned());

        if (x.lower_ <= T(0) && y.contains(T(0)))
        {
            return Bound{ nan, nan };
        }
        auto v1 = detail::_atan2(y.lower_, x.lower_);
        auto v2 = detail::_atan2(y.lower_, x.upper_);
        auto v3 = detail::_atan2(y.upper_, x.lower_);
        auto v4 = detail::_atan2(y.upper_, x.upper_);
        return Bound{ detail::_min(detail::_min(v1, v2), detail::_min(v3, v4)), detail::_max(detail::_max(v1, v2), detail::_max(v3, v4)) };
    }
    [[nodiscard]] friend constexpr Bound
    atan2(Bound const& y, T x)
    {
        gsl_ExpectsDebug(y._assigned());

        if (x <= T(0) && y.contains(T(0)))
        {
            return Bound{ nan, nan };
        }
        auto v1 = detail::_atan2(y.lower_, x);
        auto v2 = detail::_atan2(y.upper_, x);
        return Bound{ detail::_min(v1, v2), detail::_max(v1, v2) };
    }
    [[nodiscard]] friend constexpr Bound
    atan2(T y, Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        if (x.lower_ <= T(0) && y == T(0))
        {
            return Bound{ nan, nan };
        }
        auto v1 = detail::_atan2(y, x.lower_);
        auto v2 = detail::_atan2(y, x.upper_);
        return Bound{ detail::_min(v1, v2), detail::_max(v1, v2) };
    }

    [[nodiscard]] friend constexpr Bound
    floor(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return Bound{ detail::_floor(x.lower_), detail::_floor(x.upper_) };
    }
    [[nodiscard]] friend constexpr Bound
    ceil(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return Bound{ detail::_ceil(x.lower_), detail::_ceil(x.upper_) };
    }
    [[nodiscard]] friend constexpr Bound
    frac(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        T lfloor = detail::_floor(x.lower_);
        T ufloor = detail::_floor(x.upper_);
        if (lfloor != ufloor)
        {
            return Bound{ T(0), T(1) };
        }
        return Bound{ x.lower_ - lfloor, x.upper_ - ufloor };
    }

    [[nodiscard]] friend constexpr std::pair<Bound, Bound>
    fractional_weights(Bound a, Bound b)
    {
        gsl_ExpectsDebug(a._assigned() && b._assigned());
        gsl_ExpectsDebug((a >= 0).matches(true));
        gsl_ExpectsDebug((b >= 0).matches(true));
        gsl_ExpectsDebug((a + b > 0).matches(true));

        return {
            Bound{ a.lower_/(a.lower_ + b.upper_), a.upper_/(a.upper_ + b.lower_) },
            Bound{ b.lower_/(a.upper_ + b.lower_), b.upper_/(a.lower_ + b.upper_) }
        };
    }

    [[nodiscard]] friend constexpr Bound
    abs(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        return x.lower_ <= T(0) && x.upper_ >= T(0)             // if  0 ∈ [a,b] :
            ? Bound{ T(0), detail::_max(-x.lower_, x.upper_) }  //     → [0, max{-a,b}]
            : x.lower_ < T(0)                                   // else if  a,b < 0 :
            ? Bound{ -x.upper_, -x.lower_ }                     //     → [-b,-a]
            : x;                                                // else → [a,b]
    }

    [[nodiscard]] friend constexpr Set<Sign>
    sgn(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        auto result = Set<Sign>{ };
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

    [[nodiscard]] friend constexpr Set<bool>
    isinf(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        auto result = Set<bool>{ };
        if (std::isinf(x.lower_) || std::isinf(x.upper_))
        {
            result.assign(true);
        }
        if (x.lower_ < x.upper_ || (x.lower_ == x.upper_ && !std::isinf(x.lower_)))
        {
            result.assign(false);
        }
        return result;
    }
    [[nodiscard]] friend constexpr Set<bool>
    isfinite(Bound const& x)
    {
        return !isinf(x);
    }
    [[nodiscard]] friend constexpr Set<bool>
    isnan(Bound const& x)
    {
        gsl_ExpectsDebug(x._assigned());

        if (std::isnan(x.lower_) || std::isnan(x.upper_))
        {
            return Set{ false, true };
        }
        return Set{ false };
    }
};

template <typename ElemT, typename TraitsT, typename T>
std::basic_ostream<ElemT, TraitsT>&
operator <<(std::basic_ostream<ElemT, TraitsT>& stream, Bound<T> const& x)
{
    static constexpr char const* cs = ", ";
    static constexpr ElemT ecs[] = { ElemT(cs[0]), ElemT(cs[1]), 0 };
    return stream << ElemT('[') << x.lower() << ecs << x.upper() << ElemT(']');
}


template <std::size_t I, typename T>
constexpr T
get(Bound<T> const& bound) noexcept
{
    static_assert(I < 2, "tuple index out of range");
    if constexpr (I == 0) return bound.lower();
    if constexpr (I == 1) return bound.upper();
}


constexpr Set<Sign>
operator +(Set<Sign> x)
{
    return x;
}
constexpr Set<Sign>
operator -(Set<Sign> x)
{
    Set<Sign> result;
    if (x.contains(positiveSign))
    {
        result.assign(negativeSign);
    }
    if (x.contains(negativeSign))
    {
        result.assign(positiveSign);
    }
    if (x.contains(zeroSign))
    {
        result.assign(zeroSign);
    }
    return result;
}
constexpr Set<Sign>
operator *(Set<Sign> lhs, Set<Sign> rhs)
{
    bool lp = lhs.contains(positiveSign);
    bool lz = lhs.contains(zeroSign);
    bool ln = lhs.contains(negativeSign);
    bool rp = rhs.contains(positiveSign);
    bool rz = rhs.contains(zeroSign);
    bool rn = rhs.contains(negativeSign);
    Set<Sign> result;
    if ((lp && rn) || (ln && rp))
    {
        result.assign(negativeSign);
    }
    if ((lp && rp) || (ln && rn))
    {
        result.assign(positiveSign);
    }
    if (lz || rz)
    {
        result.assign(zeroSign);
    }
    return result;
}
constexpr Set<Sign>
operator *(Set<Sign> lhs, Sign rhs)
{
    return lhs*Set<Sign>(rhs);
}
constexpr Set<Sign>
operator *(Sign lhs, Set<Sign> rhs)
{
    return rhs*Set<Sign>(lhs);
}
template <Arithmetic T>
constexpr Bound<T>
operator *(T lhs, Set<Sign> rhs)
{
    Bound<T> result;
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
template <Arithmetic T>
constexpr Bound<T>
operator *(Set<Sign> lhs, T rhs)
{
    return rhs*lhs;
}


template <typename T, typename U>
constexpr Set<T>&
assign(Set<T>& x, U&& u)
{
    return x.assign(std::forward<U>(u));
}
template <typename T, typename U>
constexpr Bound<T>&
assign(Bound<T>& x, U&& u)
{
    return x.assign(std::forward<U>(u));
}
template <typename T, typename U,
          std::enable_if_t<std::is_assignable_v<T&, U&&>, int> = 0>
constexpr T&&
assign(T&& x, U&& u)
{
    return std::forward<T>(x = std::forward<U>(u));
}
template <typename T, typename U,
          std::enable_if_t<std::conjunction_v<std::negation<std::is_assignable<T&, U&&>>, makeshift::is_tuple_like<std::remove_cvref_t<T>>, makeshift::is_tuple_like<std::remove_cvref_t<U>>>, int> = 0>
constexpr T&&
assign(T&& x, U&& u)
{
    makeshift::template_for(
        [](auto& lhs, auto&& rhs)
        {
            assign(lhs, std::forward<decltype(rhs)>(rhs));
        },
        x, std::forward<U>(u));
    return std::forward<T>(x);
}


template <typename T, typename U>
constexpr Set<T>&
reset(Set<T>& x, U&& u)
{
    return x.reset(std::forward<U>(u));
}
template <typename T, typename U>
constexpr Bound<T>&
reset(Bound<T>& x, U&& u)
{
    return x.reset(std::forward<U>(u));
}
template <typename T, typename U,
          std::enable_if_t<std::is_assignable_v<T&, U&&>, int> = 0>
constexpr T&&
reset(T&& x, U&& u)
{
    return std::forward<T>(x = std::forward<U>(u));
}
template <typename T, typename U,
          std::enable_if_t<std::conjunction_v<std::negation<std::is_assignable<T&, U&&>>, makeshift::is_tuple_like<std::remove_cvref_t<T>>, makeshift::is_tuple_like<std::remove_cvref_t<U>>>, int> = 0>
constexpr T&&
reset(T&& x, U&& u)
{
    makeshift::template_for(
        [](auto& lhs, auto&& rhs)
        {
            reset(lhs, std::forward<decltype(rhs)>(rhs));
        },
        x, std::forward<U>(u));
    return std::forward<T>(x);
}


template <typename T>
class Regular
{
private:
    T value_;

    constexpr void
    _construct_inplace(T& rhs) noexcept  // use `noexcept` to enforce termination if anything goes wrong
    {
        value_.~T();
        new (&value_) T(std::move(rhs));
    }

public:
    Regular() = default;
    Regular(Regular const&) = default;
    Regular(Regular&&) = default;

    constexpr Regular(T _value)
        : value_(std::move(_value))
    {
    }

    constexpr Regular&
    operator =(Regular const& rhs)
    {
        auto copy = T(rhs.value_);  // make sure any exception is thrown before attempting to reconstruct in-place
        _construct_inplace(copy);
        return *this;
    }
    constexpr Regular&
    operator =(Regular&& rhs) noexcept
    {
        _construct_inplace(rhs.value_);
        return *this;
    }

    [[nodiscard]] constexpr T&
    operator ()() & noexcept
    {
        return value_;
    }
    [[nodiscard]] constexpr T&&
    operator ()() && noexcept
    {
        return std::move(value_);
    }
    [[nodiscard]] constexpr T const&
    operator ()() const & noexcept
    {
        return value_;
    }
};


template <typename T, typename = void>
struct BoundType_;
template <>
struct BoundType_<bool>
{
    using type = Set<bool>;
};
template <typename T>
struct BoundType_<T, std::enable_if_t<std::is_enum_v<T>>>
{
    using type = Set<T>;
};
template <typename T>
struct BoundType_<T, std::enable_if_t<std::is_floating_point_v<T>>>
{
    using type = Bound<T>;
};
template <typename T>
struct BoundType_<Set<T>>
{
    using type = Set<T>;
};
template <typename T>
struct BoundType_<Bound<T>>
{
    using type = Bound<T>;
};
// TODO: implement bound type for integers
template <typename T>
using BoundType = typename BoundType_<T>::type;


    // TODO: should use `BoundType<>` here, but with inference; also support integers
template <typename T>
[[nodiscard]] constexpr T
if_else(bool cond, T resultIfTrue, T resultIfFalse)
{
    return cond ? resultIfTrue : resultIfFalse;
}
template <typename T>
[[nodiscard]] constexpr Bound<T>
if_else(Set<bool> cond, Bound<T> resultIfTrue, Bound<T> resultIfFalse)
{
    Bound<T> result;
    if (maybe(cond))
    {
        result.assign(resultIfTrue);
    }
    if (maybe_not(cond))
    {
        result.assign(resultIfFalse);
    }
    return result;
}
template <typename T>
[[nodiscard]] constexpr Bound<T>
if_else(Set<bool> cond, T resultIfTrue, Bound<T> resultIfFalse)
{
    return rpmc::if_else(cond, Bound(resultIfTrue), resultIfFalse);
}
template <typename T>
[[nodiscard]] constexpr Bound<T>
if_else(Set<bool> cond, Bound<T> resultIfTrue, T resultIfFalse)
{
    return rpmc::if_else(cond, resultIfTrue, Bound(resultIfFalse));
}
template <typename T>
[[nodiscard]] constexpr Set<T>
if_else(Set<bool> cond, Set<T> resultIfTrue, Set<T> resultIfFalse)
{
    Set<T> result;
    if (maybe(cond))
    {
        result.assign(resultIfTrue);
    }
    if (maybe_not(cond))
    {
        result.assign(resultIfFalse);
    }
    return result;
}
template <typename T>
[[nodiscard]] constexpr Set<T>
if_else(Set<bool> cond, T resultIfTrue, Set<T> resultIfFalse)
{
    return rpmc::if_else(cond, Set(resultIfTrue), resultIfFalse);
}
template <typename T>
[[nodiscard]] constexpr Set<T>
if_else(Set<bool> cond, Set<T> resultIfTrue, T resultIfFalse)
{
    return rpmc::if_else(cond, resultIfTrue, Set(resultIfFalse));
}
template <typename T>
[[nodiscard]] constexpr BoundType<T>
if_else(Set<bool> cond, T resultIfTrue, T resultIfFalse)
{
    return rpmc::if_else(cond, BoundType<T>(resultIfTrue), BoundType<T>(resultIfFalse));
}


template <typename T>
[[nodiscard]] constexpr T
constrain(T value, T lo, T hi)
{
        // Check that bounds are ordered.
    gsl_Expects(lo <= hi);

        // Check that bound overlaps with constraint interval.
    gsl_Expects(value >= lo && value <= hi);

    return value;
}
template <typename T>
[[nodiscard]] constexpr Bound<T>
constrain(Bound<T> value, T lo, T hi)
{
    auto [vlo, vhi] = value;

        // Check that bounds are ordered.
    gsl_Expects(lo <= hi);

        // Check that bound overlaps with constraint interval.
    gsl_Expects(vhi >= lo && vlo <= hi);
    
        // Narrow bound to constraint interval.
    return Bound(vlo < lo ? lo : vlo, vhi > hi ? hi : vhi);
}
template <typename T>
[[nodiscard]] constexpr Bound<T>
constrain(Bound<T> value, Bound<T> lo, Bound<T> hi)
{
    return rpmc::constrain(value, lo.lower(), hi.upper());
}

template <typename T>
[[nodiscard]] constexpr T
constrain_from_below(T value, T lo)
{
    gsl_Expects(value >= lo);

    return value;
}
template <typename T>
[[nodiscard]] constexpr Bound<T>
constrain_from_below(Bound<T> value, T lo)
{
    auto [vlo, vhi] = value;

        // Check that bound overlaps with constraint interval.
    gsl_Expects(vhi >= lo);
    
        // Narrow bound to constraint interval.
    return Bound(vlo < lo ? lo : vlo, vhi);
}
template <typename T>
[[nodiscard]] constexpr Bound<T>
constrain_from_below(Bound<T> value, Bound<T> lo)
{
    return rpmc::constrain_from_below(value, lo.lower());
}

template <typename T>
[[nodiscard]] constexpr T
constrain_from_above(T value, T hi)
{
    gsl_Expects(value <= hi);

    return value;
}
template <typename T>
[[nodiscard]] constexpr Bound<T>
constrain_from_above(Bound<T> value, T hi)
{
    auto [vlo, vhi] = value;

        // Check that bound overlaps with constraint interval.
    gsl_Expects(vlo <= hi);
    
        // Narrow bound to constraint interval.
    return Bound(vlo, vhi > hi ? hi : vhi);
}
template <typename T>
[[nodiscard]] constexpr Bound<T>
constrain_from_above(Bound<T> value, Bound<T> hi)
{
    return rpmc::constrain_from_above(value, hi.upper());
}


// algebra + hierarchical ordering + in/out data = reduced 𝒪(n²) process
// for buckets we need:
// - particle → (classification, bucket size)  (maybe try R or some combined quantity instead of √<e²>?)
// - particle → particle data
// - (particle data, particle data) → interaction radius
//
// Idea for final workflow:
// - ∀ class, class': discard if superescape; otherwise:
//   - ∀ bucket ∈ class, bucket' ∈ class': discard if superescape;
//     otherwise:
//     - 
// ? ∀ class, bucket' ∈ class': discard if superescape
// ? ∀ bucket ∈ class, bucket' ∈ class': discard if superescape
// - 
// - ∀ particle ∈ bucket ∈ class, bucket' ∈ class': compute interaction radius


} // namespace rpmc


    // Implement tuple-like protocol for `Bound<>`.
template <typename T> class std::tuple_size<rpmc::Bound<T>> : public std::integral_constant<std::size_t, 2> { };
template <std::size_t I, typename T> class std::tuple_element<I, rpmc::Bound<T>> { public: using type = T; };


#endif // INCLUDED_RPMC_TOOLS_BOUND_ALGEBRA_HPP_
