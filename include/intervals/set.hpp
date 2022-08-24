
#ifndef INCLUDED_INTERVALS_SET_HPP_
#define INCLUDED_INTERVALS_SET_HPP_


#include <array>
#include <iosfwd>
#include <bitset>
#include <utility>           // for pair<>
#include <initializer_list>

#include <gsl-lite/gsl-lite.hpp>  // for type_identity<>, ssize()

#include <makeshift/metadata.hpp>  // for reflector

#include <intervals/sign.hpp>

#include <intervals/math.hpp>   // to make assign*() and reset() available
#include <intervals/logic.hpp>  // to make if_else(), maybe() et al. for Boolean arguments available

#include <intervals/detail/set.hpp>


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
        state_.set(find_value_index(value));
    }
    explicit constexpr set(std::initializer_list<T> _values)
    {
        for (T value : _values)
        {
            state_.set(find_value_index(value));
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
        state_.set(find_value_index(value));
        return *this;
    }

    constexpr unsigned long long
    to_bits() const
    {
        return state_.to_ullong();
    }

    [[nodiscard]] constexpr bool
    contains_index(gsl::index i) const
    {
        gsl_Expects(i >= 0 && i < num_values);

        return state_.test(i);
    }
    [[nodiscard]] constexpr bool
    contains(T value) const
    {
        return state_.test(find_value_index(value));
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
//template <typename T, typename ReflectorT>
//constexpr std::array set<T, ReflectorT>::values;

template <typename T0, typename... Ts>
set(T0, Ts...) -> set<typename detail::enforce_same<T0, Ts...>::type>;


template <typename ElemT, typename TraitsT, typename T, typename ReflectorT>
std::basic_ostream<ElemT, TraitsT>&
operator <<(std::basic_ostream<ElemT, TraitsT>& stream, set<T, ReflectorT> const& x)
{
    stream << ElemT('{') << ElemT(' ');
    bool first = true;
    for (gsl::index i = 0, n = x.values.size(); i != n; ++i)
    {
        if (x.contains_index(i))
        {
            if (first)
            {
                first = false;
            }
            else
            {
                stream << ElemT(',') << ElemT(' ');
            }
            stream << x.values[i];
        }
    }
    stream << ElemT(' ') << ElemT('}');
    return stream;
}


[[nodiscard]] constexpr set<bool>
operator !(set<bool> x)
{
    gsl_ExpectsDebug(x.assigned());

    return set<bool>::from_bits((detail::lut_4vK_not >> (2*x.to_bits())) & 0b11u);
}

    // Instead of short-circuiting Boolean operators `&&` and `||`, use operators `&` and `|` for `set<bool>`,
    // and use `maybe()`, `maybe_not()`, `definitely()`, `definitely_not()`, `contingent()` to evaluate a `set<bool>` as Boolean.
set<bool> operator &&(set<bool>, set<bool>) = delete;
set<bool> operator &&(bool, set<bool>) = delete;
set<bool> operator &&(set<bool>, bool) = delete;
set<bool> operator ||(set<bool>, set<bool>) = delete;
set<bool> operator ||(bool, set<bool>) = delete;
set<bool> operator ||(set<bool>, bool) = delete;

[[nodiscard]] constexpr set<bool>
operator >=(set<bool> x, set<bool> y)
{
    gsl_ExpectsDebug(x.assigned() && y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_geq >> (2*x.to_bits() + 8*y.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator >=(set<bool> x, bool y)
{
    gsl_ExpectsDebug(x.assigned());

    return set<bool>::from_bits((detail::lut_4vK_geq >> (2*x.to_bits() + 8*(1 + int(y)))) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator >=(bool x, set<bool> y)
{
    gsl_ExpectsDebug(y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_geq >> (2*(1 + int(x)) + 8*y.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator >(set<bool> x, set<bool> y)
{
    gsl_ExpectsDebug(x.assigned() && y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_gt >> (2*x.to_bits() + 8*y.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator >(set<bool> x, bool y)
{
    gsl_ExpectsDebug(x.assigned());

    return set<bool>::from_bits((detail::lut_4vK_gt >> (2*x.to_bits() + 8*(1 + int(y)))) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator >(bool x, set<bool> y)
{
    gsl_ExpectsDebug(y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_gt >> (2*(1 + int(x)) + 8*y.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator <=(set<bool> x, set<bool> y)
{
    gsl_ExpectsDebug(x.assigned() && y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_geq >> (2*y.to_bits() + 8*x.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator <=(set<bool> x, bool y)
{
    gsl_ExpectsDebug(x.assigned());

    return set<bool>::from_bits((detail::lut_4vK_geq >> (2*(1 + int(y)) + 8*x.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator <=(bool x, set<bool> y)
{
    gsl_ExpectsDebug(y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_geq >> (2*y.to_bits() + 8*(1 + int(x)))) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator <(set<bool> x, set<bool> y)
{
    gsl_ExpectsDebug(x.assigned() && y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_gt >> (2*y.to_bits() + 8*x.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator <(set<bool> x, bool y)
{
    gsl_ExpectsDebug(x.assigned());

    return set<bool>::from_bits((detail::lut_4vK_gt >> (2*(1 + int(y)) + 8*x.to_bits())) & 0b11u);
}
[[nodiscard]] constexpr set<bool>
operator <(bool x, set<bool> y)
{
    gsl_ExpectsDebug(y.assigned());

    return set<bool>::from_bits((detail::lut_4vK_gt >> (2*y.to_bits() + 8*(1 + int(x)))) & 0b11u);
}
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


template <typename T, typename R>
constexpr void
assign(set<T, R>& lhs, gsl::type_identity_t<set<T, R>> const& rhs)
{
    gsl_Expects(!lhs.assigned());
    lhs.reset(rhs);
}
template <typename T, typename R>
constexpr void
assign_partial(set<T, R>& lhs, gsl::type_identity_t<set<T, R>> const& rhs)
{
    lhs.assign(rhs);
}
template <typename T, typename R>
constexpr void
reset(set<T, R>& lhs, gsl::type_identity_t<set<T, R>> const& rhs)
{
    lhs.reset(rhs);
}
template <typename T, typename R>
constexpr void
assign_if(set<bool> cond, set<T, R>& lhs, gsl::type_identity_t<set<T, R>> const& rhs)
{
    if (cond.matches(true))
    {
        gsl_Expects(!lhs.assigned());
        lhs.reset(rhs);
    }
    else if (cond.matches(set{ false, true }))
    {
        lhs.assign(rhs);
    }
    else
    {
        //gsl_Expects(!cond.matches(false));
        // but just do nothing if `cond.matches(set{ })`
    }
}
template <typename T, typename R>
constexpr void
assign_if_not(set<bool> cond, set<T, R>& lhs, gsl::type_identity_t<set<T, R>> const& rhs)
{
    if (cond.matches(false))
    {
        gsl_Expects(!lhs.assigned());
        lhs.reset(rhs);
    }
    else if (cond.matches(set{ false, true }))
    {
        lhs.assign(rhs);
    }
    else
    {
        //gsl_Expects(!cond.matches(true));
        // but just do nothing if `cond.matches(set{ })`
    }
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
    if (x.contains(positive_sign))
    {
        result.assign(negative_sign);
    }
    if (x.contains(negative_sign))
    {
        result.assign(positive_sign);
    }
    if (x.contains(zero_sign))
    {
        result.assign(zero_sign);
    }
    return result;
}
[[nodiscard]] constexpr set<sign>
operator *(set<sign> lhs, set<sign> rhs)
{
    bool lp = lhs.contains(positive_sign);
    bool lz = lhs.contains(zero_sign);
    bool ln = lhs.contains(negative_sign);
    bool rp = rhs.contains(positive_sign);
    bool rz = rhs.contains(zero_sign);
    bool rn = rhs.contains(negative_sign);
    set<sign> result;
    if ((lp && rn) || (ln && rp))
    {
        result.assign(negative_sign);
    }
    if ((lp && rp) || (ln && rn))
    {
        result.assign(positive_sign);
    }
    if (lz || rz)
    {
        result.assign(zero_sign);
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


template <typename T>
[[nodiscard]] constexpr set<T>
if_else(set<bool> cond, set<T> resultIfTrue, set<T> resultIfFalse)
{
    set<T> result;
    if (intervals::maybe(cond))
    {
        result.assign(resultIfTrue);
    }
    if (intervals::maybe_not(cond))
    {
        result.assign(resultIfFalse);
    }
    return result;
}
template <typename T>
[[nodiscard]] constexpr set<T>
if_else(set<bool> cond, T resultIfTrue, set<T> resultIfFalse)
{
    return intervals::if_else(cond, set(resultIfTrue), resultIfFalse);
}
template <typename T>
[[nodiscard]] constexpr set<T>
if_else(set<bool> cond, set<T> resultIfTrue, T resultIfFalse)
{
    return intervals::if_else(cond, resultIfTrue, set(resultIfFalse));
}


} // namespace intervals


#endif // INCLUDED_INTERVALS_SET_HPP_
