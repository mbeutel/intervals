
#ifndef INCLUDED_INTERVALS_DETAIL_INTERVAL_HPP_
#define INCLUDED_INTERVALS_DETAIL_INTERVAL_HPP_


#include <cmath>
#include <memory>     // for addressof()
#include <concepts>   // for derived_from<>
#include <algorithm>  // for min(), max()

#include <gsl-lite/gsl-lite.hpp>  // for gsl_AssertDebug()

#include <intervals/set.hpp>

#include <intervals/detail/math.hpp>


namespace intervals {

namespace detail {


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
template <typename T>
constexpr inline T
_min(T x, T y)
{
    using std::min;
    return min(x, y);
}
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


struct condition
{
    set<bool> value_;
};
template <typename IntervalT>
struct ordering_constraint_lr : condition  // lhs ≤ rhs
{
    IntervalT const& lhs_;
    typename IntervalT::value_type rhs_;
};
template <typename IntervalT>
struct ordering_constraint_ll : condition  // lhs ≤ rhs
{
    IntervalT const& lhs_;
    IntervalT const& rhs_;
};
template <typename IntervalT>
struct ordering_constraint_rl : condition  // lhs ≤ rhs
{
    typename IntervalT::value_type lhs_;
    IntervalT const& rhs_;
};

template <std::derived_from<condition> L, std::derived_from<condition> R>
struct constraint_conjunction : condition  // lhs ∧ rhs
{
    L lhs_;
    R rhs_;
};
template <std::derived_from<condition> L, std::derived_from<condition> R>
struct constraint_disjunction : condition  // lhs ∨ rhs
{
    L lhs_;
    R rhs_;
};

    // Instead of short-circuiting Boolean operators `&&` and `||`, use operators `&` and `|` for `Set<bool>`, and use `maybe()`,
    // `maybe_not()`, `definitely()`, `definitely_not()` to evaluate a `Set<bool>` as Boolean.
condition operator &&(condition, condition) = delete;
condition operator &&(bool, condition) = delete;
condition operator &&(condition, bool) = delete;
condition operator ||(condition, condition) = delete;
condition operator ||(bool, condition) = delete;
condition operator ||(condition, bool) = delete;

constexpr condition
operator &(condition lhs, condition rhs)
{
    return { lhs.value_ & rhs.value_ };
}
template <std::derived_from<condition> L, std::derived_from<condition> R>
constexpr constraint_conjunction<L, R>
operator &(L lhs, R rhs)
{
    return { { lhs.value_ & rhs.value_ }, lhs, rhs };
}
constexpr condition
operator &(condition lhs, bool rhs)
{
    return { lhs.value_ & rhs };
}
template <std::derived_from<condition> L>
constexpr constraint_conjunction<L, condition>
operator &(L lhs, bool rhs)
{
    return { { lhs.value_ & rhs }, lhs, { rhs } };
}
constexpr condition
operator &(bool lhs, condition rhs)
{
    return { lhs & rhs.value_ };
}
template <std::derived_from<condition> R>
constexpr constraint_conjunction<condition, R>
operator &(bool lhs, R rhs)
{
    return { { lhs & rhs.value_ }, { lhs }, rhs };
}

constexpr condition
operator |(condition lhs, condition rhs)
{
    return { lhs.value_ | rhs.value_ };
}
template <std::derived_from<condition> L, std::derived_from<condition> R>
constexpr constraint_disjunction<L, R>
operator |(L lhs, R rhs)
{
    return { { lhs.value_ | rhs.value_ }, lhs, rhs };
}
constexpr condition
operator |(condition lhs, bool rhs)
{
    return { lhs.value_ | rhs };
}
template <std::derived_from<condition> L>
constexpr constraint_disjunction<L, condition>
operator |(L lhs, bool rhs)
{
    return { { lhs.value_ | rhs }, lhs, { rhs } };
}
constexpr condition
operator |(bool lhs, condition rhs)
{
    return { lhs | rhs.value_ };
}
template <std::derived_from<condition> R>
constexpr constraint_disjunction<condition, R>
operator |(bool lhs, R rhs)
{
    return { { lhs | rhs.value_ }, { lhs }, rhs };
}

constexpr condition
operator !(condition c)
{
    return { !c.value_ };
}
template <typename IntervalT>
constexpr ordering_constraint_rl<IntervalT>
operator !(ordering_constraint_lr<IntervalT> c)
{
    return { { !c.value_ }, rhs_, lhs_ };
}
template <typename IntervalT>
constexpr ordering_constraint_lr<IntervalT>
operator !(ordering_constraint_rl<IntervalT> c)
{
    return { { !c.value_ }, rhs_, lhs_ };
}
template <typename IntervalT>
constexpr ordering_constraint_ll<IntervalT>
operator !(ordering_constraint_ll<IntervalT> c)
{
    return { { !c.value_ }, rhs_, lhs_ };
}
template <std::derived_from<condition> L, std::derived_from<condition> R>
constexpr auto
operator !(constraint_conjunction<L, R> c)
{
    return constraint_disjunction{ { !c.value_ }, detail::operator !(c.lhs_), detail::operator !(c.rhs_) };
}
template <std::derived_from<condition> L, std::derived_from<condition> R>
constexpr auto
operator !(constraint_disjunction<L, R> c)
{
    return constraint_conjunction{ !c.value_, detail::operator !(c.lhs_), detail::operator !(c.rhs_) };
}

template <typename IntervalT>
constexpr IntervalT
apply_constraint(IntervalT const& x, condition)
{
    return x;
}
template <typename IntervalT>
constexpr IntervalT
apply_constraint(IntervalT const& x, ordering_constraint_lr<IntervalT> const& c)
{
    using std::min;

    if (std::addressof(x) == std::addressof(c.lhs_))
    {
            // Apply constraint  x ≤ rhs .
        gsl_AssertDebug(x.lower_unchecked() <= c.rhs_);
        return IntervalT{ x.lower_unchecked(), min(x.upper_unchecked(), c.rhs_) };
    }
    else return x;
}
template <typename IntervalT>
constexpr IntervalT
apply_constraint(IntervalT const& x, ordering_constraint_rl<IntervalT> const& c)
{
    using std::max;

    if (std::addressof(x) == std::addressof(c.rhs_))
    {
            // Apply constraint  lhs ≤ x .
        gsl_AssertDebug(c.lhs_ <= x.upper_unchecked());
        return IntervalT{ std::max(c.lhs_, x.lower_unchecked()), x.upper_unchecked() };
    }
    else return x;
}
template <typename IntervalT>
constexpr IntervalT
apply_constraint(IntervalT const& x, ordering_constraint_ll<IntervalT> const& c)
{
    using std::min;
    using std::max;

    if (std::addressof(x) == std::addressof(c.lhs_))
    {
            // Apply constraint  x ≤ rhs⁺ .
        auto rhi = c.rhs_.upper();
        gsl_AssertDebug(x.lower_unchecked() <= rhi);
        return IntervalT{ x.lower_unchecked(), std::min(x.upper_unchecked(), rhi) };
    }
    else if (std::addressof(x) == std::addressof(c.rhs_))
    {
            // Apply constraint  lhs⁻ ≤ x .
        auto llo = c.lhs_.lower();
        gsl_AssertDebug(llo <= x.upper_unchecked());
        return IntervalT{ std::max(llo, x.lower_unchecked()), x.upper_unchecked() };
    }
    else return x;
}
template <typename IntervalT, std::derived_from<condition> L, std::derived_from<condition> R>
constexpr IntervalT
apply_constraint(IntervalT const& x, constraint_conjunction<L, R> const& c)
{
    using std::min;
    using std::max;

    auto xl = detail::apply_constraint(x, c.lhs_);
    auto xr = detail::apply_constraint(x, c.rhs_);

        // Make sure that the two constraints overlap.
    gsl_AssertDebug(xl.upper_unchecked() >= xr.lower_unchecked() && xl.lower_unchecked() <= xr.upper_unchecked());

    return { std::max(xl.lower_unchecked(), xr.lower_unchecked()), std::min(xl.upper_unchecked(), xr.upper_unchecked()) };
}
template <typename IntervalT, std::derived_from<condition> L, std::derived_from<condition> R>
constexpr IntervalT
apply_constraint(IntervalT const& x, constraint_disjunction<L, R> const& c)
{
    using std::min;
    using std::max;

    auto xl = detail::apply_constraint(x, c.lhs_);
    auto xr = detail::apply_constraint(x, c.rhs_);

    return { std::min(xl.lower_unchecked(), xr.lower_unchecked()), std::max(xl.upper_unchecked(), xr.upper_unchecked()) };
}


}  // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_INTERVAL_HPP_
