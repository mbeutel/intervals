
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


struct condition : set<bool>
{
    constexpr set<bool> const&
    as_set() const noexcept
    {
        return *this;
    }
};
template <typename IntervalT>
struct ordering_constraint_lr : condition  // lhs ≤ rhs
{
    IntervalT const& lhs_;
    IntervalT rhs_;
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
    IntervalT lhs_;
    IntervalT const& rhs_;
};
template <typename IntervalT>
struct equality_constraint_lr : condition  // lhs = rhs
{
    IntervalT const& lhs_;
    IntervalT rhs_;
};
template <typename IntervalT>
struct equality_constraint_ll : condition  // lhs = rhs
{
    IntervalT const& lhs_;
    IntervalT const& rhs_;
};
template <typename IntervalT>
struct inequality_constraint_lr : condition  // lhs ≠ rhs
{
    IntervalT const& lhs_;
    IntervalT rhs_;
};
template <typename IntervalT>
struct inequality_constraint_ll : condition  // lhs ≠ rhs
{
    IntervalT const& lhs_;
    IntervalT const& rhs_;
};
template <std::derived_from<condition> L, std::derived_from<condition> R>
struct constraint_conjunction : condition  // lhs ∧ rhs
{
    L lhs_;
    R rhs_;
};
template <std::derived_from<condition> L, std::derived_from<condition> R>
constraint_conjunction(set<bool>, L, R) -> constraint_conjunction<L, R>;
template <std::derived_from<condition> L, std::derived_from<condition> R>
struct constraint_disjunction : condition  // lhs ∨ rhs
{
    L lhs_;
    R rhs_;
};
template <std::derived_from<condition> L, std::derived_from<condition> R>
constraint_disjunction(set<bool>, L, R) -> constraint_disjunction<L, R>;

template <std::derived_from<condition> L, std::derived_from<condition> R>
constexpr constraint_conjunction<L, R>
operator &(L const& lhs, R const& rhs)
{
    return { { lhs.as_set() & rhs.as_set() }, lhs, rhs };
}
template <std::derived_from<condition> L>
constexpr constraint_conjunction<L, set<bool>>
operator &(L const& lhs, bool rhs)
{
    return { { lhs.as_set() & rhs }, lhs, { rhs } };
}
template <std::derived_from<condition> R>
constexpr constraint_conjunction<set<bool>, R>
operator &(bool lhs, R const& rhs)
{
    return { { lhs & rhs.as_set() }, { lhs }, rhs };
}

template <std::derived_from<condition> L, std::derived_from<condition> R>
constexpr constraint_disjunction<L, R>
operator |(L const& lhs, R const& rhs)
{
    return { { lhs.as_set() | rhs.as_set() }, lhs, rhs };
}
template <std::derived_from<condition> L>
constexpr constraint_disjunction<L, set<bool>>
operator |(L const& lhs, bool rhs)
{
    return { { lhs.as_set() | rhs }, lhs, { rhs } };
}
template <std::derived_from<condition> R>
constexpr constraint_disjunction<set<bool>, R>
operator |(bool lhs, R const& rhs)
{
    return { { lhs | rhs.as_set() }, { lhs }, rhs };
}

template <typename IntervalT>
constexpr ordering_constraint_rl<IntervalT>
operator !(ordering_constraint_lr<IntervalT> const& c)
{
    return { { !c.as_set() }, c.rhs_, c.lhs_ };
}
template <typename IntervalT>
constexpr ordering_constraint_lr<IntervalT>
operator !(ordering_constraint_rl<IntervalT> const& c)
{
    return { { !c.as_set() }, c.rhs_, c.lhs_ };
}
template <typename IntervalT>
constexpr ordering_constraint_ll<IntervalT>
operator !(ordering_constraint_ll<IntervalT> const& c)
{
    return { { !c.as_set() }, c.rhs_, c.lhs_ };
}
template <typename IntervalT>
constexpr inequality_constraint_lr<IntervalT>
operator !(equality_constraint_lr<IntervalT> const& c)
{
    return { { !c.as_set() }, c.lhs_, c.rhs_ };
}
template <typename IntervalT>
constexpr inequality_constraint_ll<IntervalT>
operator !(equality_constraint_ll<IntervalT> const& c)
{
    return { { !c.as_set() }, c.lhs_, c.rhs_ };
}
template <typename IntervalT>
constexpr equality_constraint_lr<IntervalT>
operator !(inequality_constraint_lr<IntervalT> const& c)
{
    return { { !c.as_set() }, c.lhs_, c.rhs_ };
}
template <typename IntervalT>
constexpr equality_constraint_ll<IntervalT>
operator !(inequality_constraint_ll<IntervalT> const& c)
{
    return { { !c.as_set() }, c.lhs_, c.rhs_ };
}
template <std::derived_from<condition> L, std::derived_from<condition> R>
constexpr auto
operator !(constraint_conjunction<L, R> const& c)
{
    return constraint_disjunction{ { !c.as_set() }, !c.lhs_, !c.rhs_ };
}
template <std::derived_from<condition> L, std::derived_from<condition> R>
constexpr auto
operator !(constraint_disjunction<L, R> const& c)
{
    return constraint_conjunction{ { !c.as_set() }, !c.lhs_, !c.rhs_ };
}


template <typename LIntervalT>
constexpr LIntervalT
constrain(LIntervalT const& x, set<bool>, bool&)
{
    return x;
}
template <typename LIntervalT, typename IntervalT>
constexpr LIntervalT
constrain(LIntervalT const& x, detail::ordering_constraint_lr<IntervalT> const& c, bool& constraintApplied)
{
    using std::min;

    if (c.contains(true))
    {
        if (std::addressof(x) == std::addressof(c.lhs_))
        {
                // Apply constraint  x ≤ rhs⁺ .
            auto xlo = x.lower_unchecked();
            auto xhi = x.upper_unchecked();
            auto rhi = c.rhs_.upper_unchecked();
            gsl_AssertDebug(xlo <= rhi);
            constraintApplied = true;
            return LIntervalT{ xlo, min(xhi, rhi) };
        }
    }
    return x;
}
template <typename LIntervalT, typename IntervalT>
constexpr LIntervalT
constrain(LIntervalT const& x, detail::ordering_constraint_rl<IntervalT> const& c, bool& constraintApplied)
{
    using std::max;

    if (c.contains(true))
    {
        if (std::addressof(x) == std::addressof(c.rhs_))
        {
            constraintApplied = true;

                // Apply constraint  lhs⁻ ≤ x .
            auto xlo = x.lower_unchecked();
            auto xhi = x.upper_unchecked();
            auto llo = c.lhs_.lower_unchecked();
            gsl_AssertDebug(llo <= xhi);
            constraintApplied = true;
            return LIntervalT{ std::max(llo, xlo), xhi };
        }
    }
    return x;
}
template <typename LIntervalT, typename IntervalT>
constexpr LIntervalT
constrain(LIntervalT const& x, detail::ordering_constraint_ll<IntervalT> const& c, bool& constraintApplied)
{
    using std::min;
    using std::max;

    if (c.contains(true))
    {
        if (std::addressof(x) == std::addressof(c.lhs_))
        {
                // Apply constraint  x ≤ rhs⁺ .
            auto xlo = x.lower_unchecked();
            auto xhi = x.upper_unchecked();
            auto rhi = c.rhs_.upper_unchecked();
            gsl_AssertDebug(xlo <= rhi);
            constraintApplied = true;
            return LIntervalT{ xlo, std::min(xhi, rhi) };
        }
        else if (std::addressof(x) == std::addressof(c.rhs_))
        {
            constraintApplied = true;

                // Apply constraint  lhs⁻ ≤ x .
            auto xlo = x.lower_unchecked();
            auto xhi = x.upper_unchecked();
            auto llo = c.lhs_.lower_unchecked();
            gsl_AssertDebug(llo <= xhi);
            constraintApplied = true;
            return LIntervalT{ std::max(llo, xlo), xhi };
        }
    }
    return x;
}
template <typename LIntervalT, typename IntervalT>
constexpr LIntervalT
constrain(LIntervalT const& x, detail::equality_constraint_lr<IntervalT> const& c, bool& constraintApplied)
{
    using std::min;
    using std::max;

    if (c.contains(true))
    {
        if (std::addressof(x) == std::addressof(c.lhs_))
        {
            constraintApplied = true;

                // Apply constraint  x = rhs .
            auto xlo = x.lower_unchecked();
            auto xhi = x.upper_unchecked();
            auto rlo = c.rhs_.lower_unchecked();
            auto rhi = c.rhs_.upper_unchecked();
            gsl_AssertDebug(rhi >= xlo && rlo <= xhi);  // intervals overlap
            constraintApplied = true;
            return LIntervalT{ max(xlo, rlo), min(xhi, rhi) };  // overlapping interval
        }
    }
    return x;
}
template <typename LIntervalT, typename IntervalT>
constexpr LIntervalT
constrain(LIntervalT const& x, detail::equality_constraint_ll<IntervalT> const& c, bool& constraintApplied)
{
    using std::min;
    using std::max;

    if (c.contains(true))
    {
        if (std::addressof(x) == std::addressof(c.lhs_) || std::addressof(x) == std::addressof(c.rhs_))
        {
                // Apply constraint  lhs = rhs .
            auto llo = c.lhs_.lower_unchecked();
            auto lhi = c.lhs_.upper_unchecked();
            auto rlo = c.rhs_.lower_unchecked();
            auto rhi = c.rhs_.upper_unchecked();
            gsl_AssertDebug(rhi >= llo && rlo <= lhi);  // intervals overlap
            constraintApplied = true;
            return LIntervalT{ max(llo, rlo), min(lhi, rhi) };  // overlapping interval
        }
    }
    return x;
}
template <typename LIntervalT, typename IntervalT>
constexpr LIntervalT
constrain(LIntervalT const& x, detail::inequality_constraint_lr<IntervalT> const& c, bool& constraintApplied)
{
    using std::min;
    using std::max;

    if (c.contains(true))
    {
        if (std::addressof(x) == std::addressof(c.lhs_))
        {
                // Apply constraint  x ≠ rhs .
            auto xlo = x.lower_unchecked();
            auto xhi = x.upper_unchecked();
            auto rlo = c.rhs_.lower_unchecked();
            auto rhi = c.rhs_.upper_unchecked();
            gsl_AssertDebug(xlo != rhi || xhi != rlo);  // intervals not identical
            constraintApplied = true;
        }
    }
    return x;
}
template <typename LIntervalT, typename IntervalT>
constexpr LIntervalT
constrain(LIntervalT const& x, detail::inequality_constraint_ll<IntervalT> const& c, bool& constraintApplied)
{
    using std::min;
    using std::max;

    if (c.contains(true))
    {
        if (std::addressof(x) == std::addressof(c.lhs_) || std::addressof(x) == std::addressof(c.rhs_))
        {
                // Apply constraint  lhs ≠ rhs .
            auto llo = c.lhs_.lower_unchecked();
            auto lhi = c.lhs_.upper_unchecked();
            auto rlo = c.rhs_.lower_unchecked();
            auto rhi = c.rhs_.upper_unchecked();
            gsl_AssertDebug(llo != rhi || lhi != rlo);  // intervals not identical
            constraintApplied = true;
        }
    }
    return x;
}
template <typename LIntervalT, std::derived_from<detail::condition> L, std::derived_from<detail::condition> R>
constexpr LIntervalT
constrain(LIntervalT const& x, detail::constraint_conjunction<L, R> const& c, bool& constraintApplied)
{
    using std::min;
    using std::max;

    if (c.contains(true))
    {
        auto xl = detail::constrain(x, c.lhs_, constraintApplied);
        auto xr = detail::constrain(x, c.rhs_, constraintApplied);

            // Make sure that the two constraints overlap.
        gsl_AssertDebug(xl.upper_unchecked() >= xr.lower_unchecked() && xl.lower_unchecked() <= xr.upper_unchecked());

        return LIntervalT{ std::max(xl.lower_unchecked(), xr.lower_unchecked()), std::min(xl.upper_unchecked(), xr.upper_unchecked()) };
    }
    return x;
}
template <typename LIntervalT, std::derived_from<detail::condition> L, std::derived_from<detail::condition> R>
constexpr LIntervalT
constrain(LIntervalT const& x, detail::constraint_disjunction<L, R> const& c, bool& constraintApplied)
{
    using std::min;
    using std::max;

    if (c.contains(true))
    {
        auto result = LIntervalT{ };
        bool constraint1Applied = false;
        auto xl = detail::constrain(x, c.lhs_, constraint1Applied);
        bool constraint2Applied = false;
        auto xr = detail::constrain(x, c.rhs_, constraint2Applied);
        if (constraint1Applied)
        {
            result.assign(xl);
            constraintApplied = true;
        }
        if (constraint2Applied)
        {
            result.assign(xr);
            constraintApplied = true;
        }
        if (constraint1Applied || constraint2Applied)
        {
            return result;
        }
    }
    return x;
}


}  // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_INTERVAL_HPP_
