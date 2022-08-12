
#ifndef INCLUDED_INTERVALS_SET_HPP_
#define INCLUDED_INTERVALS_SET_HPP_


#include <array>
#include <bitset>
#include <utility>  // for pair<>

#include <gsl-lite/gsl-lite.hpp>  // for type_identity<>, ssize()

#include <makeshift/metadata.hpp>  // for reflector

#include <intervals/sign.hpp>

#include <intervals/detail/set.hpp>
#include <intervals/detail/math.hpp>   // to make maybe() et al. for lvalue references available
#include <intervals/detail/logic.hpp>  // to make maybe() et al. for Boolean arguments available


namespace intervals {

namespace gsl = gsl_lite;


    //
    // Set of discrete values over a bounded type.
    //
    // TODO: we should somehow handle flag enums here; a set of a flag enum is essentially the flag enum itself, albeit with
    // set-like assignment/comparison semantics.
template <typename T, typename ReflectorT = makeshift::reflector>
class set
{
public:
    using value_type = T;

    static constexpr std::array values = makeshift::metadata::values<T, ReflectorT>();

private:
    static constexpr gsl::dim num_values = gsl::ssize(values);

    std::bitset<num_values> state_;

    explicit set(std::bitset<num_values> _state)
        : state_(_state)
    {
    }

    constexpr static gsl::index
    find_value_index(T value)
    {
        for (gsl::index i = 0; i != num_values; ++i)
        {
            if (values[i] == value)
            {
                return i;
            }
        }
        gsl_FailFast();
    }
    constexpr static std::pair<bool, bool>
    compare_eq(set const& lhs, set const& rhs)
    {
        bool anyMatch = (lhs.state_ & rhs.state_).any();
        auto cl = lhs.state_.count();
        auto cr = rhs.state_.count();
        bool anyPossibleMismatch = cl > 1 || cr > 1 || (cl == 1 && cr == 1 && !anyMatch);
        return { anyPossibleMismatch, anyMatch };
    }

public:
    constexpr set()
    {
    }
    constexpr set(T value)
    {
        state_.set(detail::find_value_index(value));
    }
    explicit constexpr set(std::initializer_list<T> values)
    {
        for (T value : values)
        {
            state_.set(detail::find_value_index(value));
        }
    }
    static constexpr set
    from_bits(unsigned long long bits)
    {
        return set(std::bitset<num_values>(bits));
    }

    constexpr set&
    reset()
    {
        state_ = { };
        return *this;
    }
    constexpr set&
    reset(set const& rhs)
    {
        state_ = rhs.state_;
        return *this;
    }

    constexpr bool
    assigned() const
    {
        return state_.any();
    }

    set(set const&) = default;
    set& operator =(set const&) = delete;
    constexpr set&
    assign(set const& rhs)
    {
        gsl_ExpectsDebug(rhs.assigned());
    
        state_ |= rhs.state_;
        return *this;
    }
    constexpr set&
    assign(T value)
    {
        state_.set(detail::find_value_index(value));
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
        return state_.test(detail::find_value_index(value));
    }
    [[nodiscard]] constexpr bool
    contains(set<T> const& _set) const
    {
        return (state_ & _set.state_) == _set.state_;
    }
    [[nodiscard]] constexpr bool
    matches(T value) const
    {
        return matches(set(value));
    }
    [[nodiscard]] constexpr bool
    matches(set<T> const& _set) const
    {
        return state_ == _set.state_;
    }
    [[nodiscard]] constexpr T
    value() const
    {
        gsl_ExpectsDebug(assigned());

        auto lstate = state_;
        for (gsl::index i = 0; i != num_values; ++i)
        {
            bool bit0 = lstate.test(0);
            lstate >>= 1;
            if (bit0)
            {
                if (!lstate.any())
                {
                    return values[i];
                }
                break;
            }
        }
        gsl_FailFast();
    }

    [[nodiscard]] friend constexpr set<bool>
    operator ==(set const& lhs, set const& rhs)
    {
        gsl_ExpectsDebug(lhs.assigned() && rhs.assigned());

        auto [anyPossibleMismatch, anyMatch] = compare_eq(lhs, rhs);
        auto result = set<bool>{ };
        if (anyMatch)
        {
            result.assign(true);
        }
        if (anyPossibleMismatch)
        {
            result.assign(false);
        }
        return result;
    }
    [[nodiscard]] friend constexpr set<bool>
    operator !=(set const& lhs, set const& rhs)
    {
        gsl_ExpectsDebug(lhs.assigned() && rhs.assigned());

        auto [anyPossibleMismatch, anyMatch] = compare_eq(lhs, rhs);
        auto result = set<bool>{ };
        if (anyMatch)
        {
            result.assign(false);
        }
        if (anyPossibleMismatch)
        {
            result.assign(true);
        }
        return result;
    }
};
template <typename T, typename ReflectorT>
constexpr std::array set<T, ReflectorT>::values;


[[nodiscard]] constexpr set<bool>
operator !(set<bool> x)
{
    gsl_ExpectsDebug(x.assigned());

    return set<bool>::from_bits((detail::lut_4vK_not >> (2*x.to_bits())) & 0b11u);
}

    // Instead of short-circuiting Boolean operators `&&` and `||`, use operators `&` and `|` for `set<bool>`, and use `maybe()`,
    // `maybe_not()`, `definitely()`, `definitely_not()` to evaluate a `set<bool>` as Boolean.
set<bool> operator &&(set<bool>, set<bool>) = delete;
set<bool> operator &&(bool, set<bool>) = delete;
set<bool> operator &&(set<bool>, bool) = delete;
set<bool> operator ||(set<bool>, set<bool>) = delete;
set<bool> operator ||(bool, set<bool>) = delete;
set<bool> operator ||(set<bool>, bool) = delete;

[[nodiscard]] constexpr set<bool>
operator &(set<bool> x, set<bool> y)
{
    gsl_ExpectsDebug(x.assigned() && y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_and >> (2*x.to_bits() + 8*y.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator &(set<bool> x, bool y)
{
    gsl_ExpectsDebug(x.assigned());

    return set<bool>::from_bits((detail::lut_4vK_and >> (2*x.to_bits() + 8*(1 + int(y)))) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator &(bool x, set<bool> y)
{
    gsl_ExpectsDebug(y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_and >> (2*y.to_bits() + 8*(1 + int(x)))) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator |(set<bool> x, set<bool> y)
{
    gsl_ExpectsDebug(x.assigned() && y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_or >> (2*x.to_bits() + 8*y.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator |(set<bool> x, bool y)
{
    gsl_ExpectsDebug(x.assigned());

    return set<bool>::from_bits((detail::lut_4vK_or >> (2*x.to_bits() + 8*(1 + int(y)))) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator |(bool x, set<bool> y)
{
    gsl_ExpectsDebug(y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_or >> (2*y.to_bits() + 8*(1 + int(x)))) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator ^(set<bool> x, set<bool> y)
{
    gsl_ExpectsDebug(x.assigned() && y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_xor >> (2*x.to_bits() + 8*y.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator ^(set<bool> x, bool y)
{
    gsl_ExpectsDebug(x.assigned());

    return set<bool>::from_bits((detail::lut_4vK_xor >> (2*x.to_bits() + 8*(1 + int(y)))) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator ^(bool x, set<bool> y)
{
    gsl_ExpectsDebug(y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_xor >> (2*y.to_bits() + 8*(1 + int(x)))) & 0b11u);
}

[[nodiscard]] constexpr bool
maybe(set<bool> x) noexcept
{
    return x.contains(true);
}
[[nodiscard]] constexpr bool
maybe_not(set<bool> x) noexcept
{
    return x.contains(false);
}
[[nodiscard]] constexpr bool
definitely(set<bool> x) noexcept
{
    return x.matches(true);
}
[[nodiscard]] constexpr bool
definitely_not(set<bool> x) noexcept
{
    return x.matches(false);
}
[[nodiscard]] constexpr bool
contingent(set<bool> x) noexcept
{
    return x.matches(set{ false, true });
}


    // TODO: find different names?
template <typename T, typename ReflectorT>
[[nodiscard]] constexpr detail::member_assigner<set<T, ReflectorT>>
maybe(set<T, ReflectorT>& x) noexcept
{
    return detail::member_assigner<set<T, ReflectorT>>(x);
}
template <typename T, typename ReflectorT>
[[nodiscard]] constexpr detail::member_resetter<set<T, ReflectorT>>
definitely(set<T, ReflectorT>& x) noexcept
{
    return detail::member_resetter<set<T, ReflectorT>>(x);
}


[[nodiscard]] constexpr set<sign>
operator +(set<sign> x)
{
    return x;
}
[[nodiscard]] constexpr set<sign>
operator -(set<sign> x)
{
    set<sign> result;
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
[[nodiscard]] constexpr set<sign>
operator *(set<sign> lhs, set<sign> rhs)
{
    bool lp = lhs.contains(positiveSign);
    bool lz = lhs.contains(zeroSign);
    bool ln = lhs.contains(negativeSign);
    bool rp = rhs.contains(positiveSign);
    bool rz = rhs.contains(zeroSign);
    bool rn = rhs.contains(negativeSign);
    set<sign> result;
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
[[nodiscard]] constexpr set<sign>
operator *(set<sign> lhs, sign rhs)
{
    return lhs*set<sign>(rhs);
}
[[nodiscard]] constexpr set<sign>
operator *(sign lhs, set<sign> rhs)
{
    return rhs*set<sign>(lhs);
}


} // namespace intervals


#endif // INCLUDED_INTERVALS_SET_HPP_
