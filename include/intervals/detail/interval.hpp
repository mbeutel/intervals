
#ifndef INCLUDED_INTERVALS_DETAIL_INTERVAL_HPP_
#define INCLUDED_INTERVALS_DETAIL_INTERVAL_HPP_


#include <cmath>
#include <memory>     // for addressof()
#include <concepts>   // for derived_from<>
#include <iterator>   // for random_access_iterator<>
#include <algorithm>  // for min(), max()

#include <gsl-lite/gsl-lite.hpp>  // for gsl_AssertDebug()

#include <intervals/set.hpp>

#include <intervals/detail/math.hpp>


namespace intervals {


template <typename T> class interval;


namespace detail {


    // Natvis debugging aid
template <typename T> struct interval_arithmetic_specialization { };
template <typename T> struct interval_iterator_specialization { };

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


template <typename IntervalT>
class constrained_interval : public IntervalT::base
{
public:
    using base = typename IntervalT::base;

private:
    const IntervalT* const id_;

    explicit constexpr constrained_interval(IntervalT const& id, IntervalT const& value)
        : base(value), id_(&id)
    {
    }

public:
    using interval_type = IntervalT;

    constexpr bool
    matches_identity(IntervalT const& rhs) const
    {
        return &rhs == id_;
    }
    bool
    matches_identity(IntervalT const&& rhs) const = delete;
    constexpr bool
    matches_identity(constrained_interval const& rhs) const
    {
        return rhs.id_ == id_;
    }

    static constexpr constrained_interval
    make_with_identity(IntervalT const& id, IntervalT const& value)
    {
        return constrained_interval(id, value);
    }
    static constrained_interval
    make_with_identity(IntervalT const&& id, IntervalT const& value) = delete;
    static constexpr constrained_interval
    make_with_identity(constrained_interval const& id, IntervalT const& value)
    {
        return constrained_interval(*id.id_, value);
    }

    constrained_interval(constrained_interval const&) = default;
    constrained_interval(constrained_interval const&& rhs)
        : constrained_interval(rhs)
    {
    }
    constexpr constrained_interval(IntervalT const& interval)
        : base(interval), id_(&interval)
    {
    }
    constrained_interval(IntervalT const&& interval) = delete;

    constexpr constrained_interval&
    reset(IntervalT const& rhs)
    {
        gsl_Expects(&rhs == id_);
        this->_reset(rhs.lower_unchecked(), rhs.upper_unchecked());
        return *this;
    }
    constrained_interval&
    reset(IntervalT const&& rhs) = delete;
    constexpr constrained_interval&
    reset(constrained_interval const& rhs)
    {
        gsl_Expects(rhs.id_ == id_);
        this->_reset(rhs.lower_unchecked(), rhs.upper_unchecked());
        return *this;
    }
};


class interval_functions;

template <typename T>
concept floating_point_interval_value = std::floating_point<std::remove_cvref_t<T>>;
template <typename T>
concept integral_interval_value = std::integral<std::remove_cvref_t<T>>;
template <typename T>
concept arithmetic_interval_value = floating_point_interval_value<T> || integral_interval_value<T>;
template <typename T>
concept iterator_interval_value = std::random_access_iterator<std::remove_cvref_t<T>>;
template <typename T>
concept interval_value = arithmetic_interval_value<T> || iterator_interval_value<T>;

template <typename T>
concept not_interval = !std::derived_from<std::remove_cvref_t<T>, interval_functions>;
template <typename T>
concept any_interval = std::derived_from<std::remove_cvref_t<T>, interval_functions>;
template <any_interval T>
using interval_value_t = typename std::remove_cvref_t<T>::value_type;
template <typename T>
concept floating_point_interval = any_interval<T> && floating_point_interval_value<interval_value_t<T>>;
template <typename T>
concept integral_interval = any_interval<T> && integral_interval_value<interval_value_t<T>>;
template <typename T>
concept arithmetic_interval = any_interval<T> && arithmetic_interval_value<interval_value_t<T>>;
template <typename T>
concept iterator_interval = any_interval<T> && iterator_interval_value<interval_value_t<T>>;

template <typename T>
concept interval_arg = interval_value<T> || any_interval<T>;
template <typename T>
concept floating_point_interval_arg = floating_point_interval_value<T> || floating_point_interval<T>;
template <typename T>
concept integral_interval_arg = integral_interval_value<T> || integral_interval<T>;
template <typename T>
concept arithmetic_interval_arg = arithmetic_interval_value<T> || arithmetic_interval<T>;
template <typename T>
concept iterator_interval_arg = iterator_interval_value<T> || iterator_interval<T>;

template <interval_arg T> struct interval_arg_value_0_;
template <interval_value T> struct interval_arg_value_0_<T> { using type = T; };
template <std::derived_from<interval_functions> IntervalT> struct interval_arg_value_0_<IntervalT> { using type = typename IntervalT::value_type; };
template <interval_arg T> struct interval_arg_value : interval_arg_value_0_<std::remove_cvref_t<T>> { };
template <interval_arg T> using interval_arg_value_t = typename interval_arg_value<T>::type;

template <interval_arg T> using interval_of_t = interval<interval_arg_value_t<T>>;

template <any_interval T> using interval_t = typename T::interval_type;

template <typename L, typename R>
concept relational = requires(L const& lhs, R const& rhs) {
    { lhs < rhs } -> std::convertible_to<bool>;
    { lhs <= rhs } -> std::convertible_to<bool>;
    { lhs > rhs } -> std::convertible_to<bool>;
    { lhs >= rhs } -> std::convertible_to<bool>;
    { lhs == rhs } -> std::convertible_to<bool>;
    { lhs != rhs } -> std::convertible_to<bool>;
};
template <typename L, typename R>
concept relational_values =
    interval_arg<L> &&
    interval_arg<R> &&
    relational<interval_arg_value_t<L>, interval_arg_value_t<R>>;

template <typename L, typename R>
concept same_values =
    interval_arg<L> &&
    interval_arg<R> &&
    std::same_as<interval_arg_value_t<L>, interval_arg_value_t<R>>;

template <interval_arg... Ts>
using common_interval_value_t = std::common_type_t<interval_arg_value_t<Ts>...>;

template <interval_arg... Ts>
using common_interval_t = interval<common_interval_value_t<Ts...>>;

template <typename L, typename R>
concept arithmetic_operands =
    interval_arg<L> &&
    interval_arg<R> &&
    arithmetic_interval_value<common_interval_value_t<L, R>>;

template <typename L, typename R>
concept floating_point_operands =
    interval_arg<L> &&
    interval_arg<R> &&
    floating_point_interval_value<common_interval_value_t<L, R>>;

template <any_interval IntervalT> constexpr bool have_id_v = false;
template <any_interval IntervalT> constexpr bool have_id_v<constrained_interval<IntervalT>> = true;
template <any_interval IntervalT, any_interval T>
constexpr bool
can_compare_id_v = std::is_same_v<interval_t<IntervalT>, interval_t<T>> && have_id_v<T>;

template <typename IntervalT> using as_constrained_interval_t = constrained_interval<interval_t<IntervalT>>;


constexpr inline struct _is_negative_inf_t
{
    template <std::floating_point T>
    constexpr inline bool
    operator ()(T x) const noexcept
    {
        return x == -std::numeric_limits<T>::infinity();
    }
    template <std::integral T>
    constexpr inline bool
    operator ()(T) const noexcept
    {
        return false;
    }
} is_negative_inf;
constexpr inline struct _is_positive_inf_t
{
    template <std::floating_point T>
    constexpr inline bool
    operator ()(T x) const noexcept
    {
        return x == std::numeric_limits<T>::infinity();
    }
    template <std::integral T>
    constexpr inline bool
    operator ()(T) const noexcept
    {
        return false;
    }
} is_positive_inf;

constexpr inline struct _lower_t
{
    template <any_interval IntervalT>
    constexpr inline typename IntervalT::value_type
    operator ()(IntervalT const& arg) const noexcept
    {
        return arg.lower_unchecked();
    }
    template <interval_value T>
    constexpr inline T const&
    operator ()(T const& arg) const noexcept
    {
        return arg;
    }
} lower;
constexpr inline struct _upper_t
{
    template <any_interval IntervalT>
    constexpr inline typename IntervalT::value_type
    operator ()(IntervalT const& arg) const noexcept
    {
        return arg.upper_unchecked();
    }
    template <interval_value T>
    constexpr inline T const&
    operator ()(T const& arg) const noexcept
    {
        return arg;
    }
} upper;

constexpr inline struct _contains_t
{
    template <any_interval IntervalT>
    constexpr inline bool
    operator ()(IntervalT const& arg, typename IntervalT::value_type value) const noexcept
    {
        return arg.contains(value);
    }
    template <interval_value L, interval_value R>
    constexpr inline bool
    operator ()(L arg, R value) const noexcept
    {
        return arg == value;
    }
} contains;

constexpr inline struct _assigned_t
{
    template <any_interval IntervalT>
    constexpr inline bool
    operator ()(IntervalT const& arg) const noexcept
    {
        return arg.assigned();
    }
    template <interval_value T>
    constexpr inline bool
    operator ()(T const&) const noexcept
    {
        return true;
    }
} assigned;

constexpr inline struct _as_interval_t
{
    template <any_interval IntervalArgT>
    constexpr inline as_constrained_interval_t<IntervalArgT>
    operator ()(IntervalArgT const& x) const
    {
        return x;
    }
    template <any_interval IntervalArgT>
    constexpr inline IntervalArgT const&&
    operator ()(IntervalArgT const&& x) const
    {
        return std::move(x);
    }
    template <typename IntervalArgT>
    constexpr inline constrained_interval<IntervalArgT> const&
    operator ()(constrained_interval<IntervalArgT> const& x) const
    {
        return x;
    }
    template <typename IntervalArgT>
    constexpr inline constrained_interval<IntervalArgT> const&&
    operator ()(constrained_interval<IntervalArgT> const&& x) const
    {
        return std::move(x);
    }
    template <interval_value T>
    constexpr inline interval<T>
    operator ()(T const& arg) const noexcept
    {
        return arg;
    }
} as_interval;

constexpr inline struct _succ_t
{
    template <typename T>
    constexpr T
    operator ()(T value) const
    {
        if constexpr (std::integral<T> || std::random_access_iterator<T>)
        {
            return ++value;
        }
        else return value;
    }
} succ;
constexpr inline struct _pred_t
{
    template <typename T>
    constexpr T
    operator ()(T value) const
    {
        if constexpr (std::integral<T> || std::random_access_iterator<T>)
        {
            return --value;
        }
        else return value;
    }
} pred;

constexpr inline struct _matches_identity_t
{
    template <any_interval IntervalT, any_interval T>
    constexpr bool
    operator ()([[maybe_unused]] IntervalT const& x, [[maybe_unused]] T const& arg) const
    {
        if constexpr (can_compare_id_v<IntervalT, T>)
        {
            return arg.matches_identity(x);
        }
        else return false;
    }
    template <any_interval IntervalT, interval_value T>
    constexpr bool
    operator ()(IntervalT const&, T const&) const
    {
        return false;
    }
} matches_identity;


template <interval_arg L, interval_arg R = L>
constexpr inline common_interval_t<L, R>
nan_interval() noexcept
{
    using T = common_interval_value_t<L, R>;
    if constexpr (floating_point_operands<L, R>)
    {
        constexpr T nan = std::numeric_limits<T>::quiet_NaN();
        return common_interval_t<L, R>{ nan, nan };
    }
    else return common_interval_t<L, R>{ }; 
}

template <interval_arg L, interval_arg R = L>
constexpr inline common_interval_t<L, R>
inf_interval() noexcept
{
    using T = common_interval_value_t<L, R>;
    if constexpr (floating_point_operands<L, R>)
    {
        constexpr T inf = std::numeric_limits<T>::infinity();
        return common_interval_t<L, R>{ -inf, inf };
    }
    else return common_interval_t<L, R>{ }; 
}


struct condition : set<bool>
{
    constexpr set<bool> const&
    as_set() const noexcept
    {
        return *this;
    }
};
template <typename L, typename R>
struct less_equal_constraint : condition  // lhs ≤ rhs
{
    L lhs_;
    R rhs_;
};
template <typename L, typename R>
less_equal_constraint(condition, L, R) -> less_equal_constraint<L, R>;
template <typename L, typename R>
struct less_constraint : condition  // lhs < rhs
{
    L lhs_;
    R rhs_;
};
template <typename L, typename R>
struct equality_constraint : condition  // lhs = rhs
{
    L lhs_;
    R rhs_;
};
template <typename L, typename R>
struct inequality_constraint : condition  // lhs ≠ rhs
{
    L lhs_;
    R rhs_;
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

template <typename L, typename R>
constexpr less_equal_constraint<R, L>
operator !(less_constraint<L, R> const& c)
{
    return { { !c.as_set() }, c.rhs_, c.lhs_ };
}
template <typename L, typename R>
constexpr less_constraint<R, L>
operator !(less_equal_constraint<L, R> const& c)
{
    return { { !c.as_set() }, c.rhs_, c.lhs_ };
}
template <typename L, typename R>
constexpr inequality_constraint<L, R>
operator !(equality_constraint<L, R> const& c)
{
    return { { !c.as_set() }, c.lhs_, c.rhs_ };
}
template <typename L, typename R>
constexpr equality_constraint<L, R>
operator !(inequality_constraint<L, R> const& c)
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


template <typename IntervalT>
constexpr as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, set<bool>, bool&)
{
    using CInterval = as_constrained_interval_t<IntervalT>;
    return x;
}
template <typename IntervalT, typename L, typename R>
constexpr as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, less_equal_constraint<L, R> const& c, bool& constraintApplied)
{
    using CInterval = as_constrained_interval_t<IntervalT>;
    using UInterval = interval_t<IntervalT>;
    using T = common_interval_value_t<L, R>;

    if (c.contains(true))
    {
        if constexpr (can_compare_id_v<IntervalT, L>)
        {
            if (matches_identity(x, c.lhs_))
            {
                    // Apply constraint  x ≤ rhs⁺ .
                auto xlo = lower(x);
                auto xhi = upper(x);
                auto rhi = upper(c.rhs_);
                gsl_AssertDebug(xlo <= rhi);
                constraintApplied = true;
                return CInterval::make_with_identity(x, UInterval{ xlo, std::min<T>(xhi, rhi) });
            }
        }
        if constexpr (can_compare_id_v<IntervalT, R>)
        {
            if (matches_identity(x, c.rhs_))
            {
                    // Apply constraint  lhs⁻ ≤ x .
                auto xlo = lower(x);
                auto xhi = upper(x);
                auto llo = lower(c.lhs_);
                gsl_AssertDebug(llo <= xhi);
                constraintApplied = true;
                return CInterval::make_with_identity(x, UInterval{ std::max<T>(llo, xlo), xhi });
            }
        }
    }
    return x;
}
template <typename IntervalT, typename L, typename R>
constexpr as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, less_constraint<L, R> const& c, bool& constraintApplied)
{
    using CInterval = as_constrained_interval_t<IntervalT>;
    using UInterval = interval_t<IntervalT>;
    using T = common_interval_value_t<L, R>;

    if (c.contains(true))
    {
        if constexpr (can_compare_id_v<IntervalT, L>)
        {
            if (matches_identity(x, c.lhs_))
            {
                    // Apply constraint  x < rhs⁺ .
                auto xlo = lower(x);
                auto xhi = upper(x);
                auto rhi = upper(c.rhs_);
                gsl_AssertDebug(xlo < rhi);
                constraintApplied = true;
                return CInterval::make_with_identity(x, UInterval{ xlo, std::min<T>(xhi, pred(rhi)) });
            }
        }
        if constexpr (can_compare_id_v<IntervalT, R>)
        {
            if (matches_identity(x, c.rhs_))
            {
                    // Apply constraint  lhs⁻ < x .
                auto xlo = lower(x);
                auto xhi = upper(x);
                auto llo = lower(c.lhs_);
                gsl_AssertDebug(llo < xhi);
                constraintApplied = true;
                return CInterval::make_with_identity(x, UInterval{ std::max<T>(succ(llo), xlo), xhi });
            }
        }
    }
    return x;
}
template <typename IntervalT, typename L, typename R>
constexpr as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, equality_constraint<L, R> const& c, bool& constraintApplied)
{
    using CInterval = as_constrained_interval_t<IntervalT>;
    using UInterval = interval_t<IntervalT>;
    using T = common_interval_value_t<L, R>;

    if constexpr (can_compare_id_v<IntervalT, L> || can_compare_id_v<IntervalT, R>)
    {
        if (c.contains(true))
        {
            if (matches_identity(x, c.lhs_) || matches_identity(x, c.rhs_))
            {
                    // Apply constraint  lhs = rhs .
                auto llo = lower(c.lhs_);
                auto lhi = upper(c.lhs_);
                auto rlo = lower(c.rhs_);
                auto rhi = upper(c.rhs_);
                gsl_AssertDebug(rhi >= llo && rlo <= lhi);  // intervals overlap
                constraintApplied = true;
                return CInterval::make_with_identity(x, UInterval{ std::max<T>(llo, rlo), std::min<T>(lhi, rhi) });  // overlapping interval
            }
        }
        return x;
    }
    else return x;
}
template <typename IntervalT, typename L, typename R>
constexpr as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, inequality_constraint<L, R> const& c, bool& constraintApplied)
{
    using CInterval = as_constrained_interval_t<IntervalT>;
    using UInterval = interval_t<IntervalT>;
    using T = common_interval_value_t<L, R>;

    if constexpr (can_compare_id_v<IntervalT, L> || can_compare_id_v<IntervalT, R>)
    {
        if (c.contains(true))
        {
            if (matches_identity(x, c.lhs_) || matches_identity(x, c.rhs_))
            {
                    // Apply constraint  lhs ≠ rhs .
                auto llo = lower(c.lhs_);
                auto lhi = upper(c.lhs_);
                auto rlo = lower(c.rhs_);
                auto rhi = upper(c.rhs_);
                gsl_AssertDebug(llo != rhi || lhi != rlo);  // intervals not identical
                constraintApplied = true;
                if constexpr (std::is_integral_v<typename IntervalT::value_type> || std::random_access_iterator<typename IntervalT::value_type>)
                {
                    if (matches_identity(x, c.lhs_) && llo != lhi && rlo == rhi)
                    {
                        if (llo == rlo)
                        {
                            return CInterval::make_with_identity(x, UInterval{ succ(llo), lhi });
                        }
                        else if (lhi == rlo)
                        {
                            return CInterval::make_with_identity(x, UInterval{ llo, pred(lhi) });
                        }
                    }
                    if (matches_identity(x, c.rhs_) && rlo != rhi && llo == lhi)
                    {
                        if (rlo == llo)
                        {
                            return CInterval::make_with_identity(x, UInterval{ succ(rlo), rhi });
                        }
                        else if (rhi == llo)
                        {
                            return CInterval::make_with_identity(x, UInterval{ rlo, pred(rhi) });
                        }
                    }
                }
            }
        }
        return x;
    }
    else return x;
}
template <typename IntervalT, std::derived_from<condition> L, std::derived_from<condition> R>
constexpr as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, constraint_conjunction<L, R> const& c, bool& constraintApplied)
{
    using CInterval = as_constrained_interval_t<IntervalT>;
    using UInterval = interval_t<IntervalT>;

    if (c.contains(true))
    {
        auto xl = detail::constrain(x, c.lhs_, constraintApplied);
        auto xr = detail::constrain(x, c.rhs_, constraintApplied);

            // Make sure that the two constraints overlap.
        gsl_AssertDebug(upper(xl) >= lower(xr) && lower(xl) <= upper(xr));

        return CInterval::make_with_identity(x, UInterval{ std::max(lower(xl), lower(xr)), std::min(upper(xl), upper(xr)) });
    }
    return x;
}
template <typename IntervalT, std::derived_from<condition> L, std::derived_from<condition> R>
constexpr as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, constraint_disjunction<L, R> const& c, bool& constraintApplied)
{
    using CInterval = as_constrained_interval_t<IntervalT>;
    using UInterval = interval_t<IntervalT>;

    if (c.contains(true))
    {
        bool constraintApplied1 = false;
        bool constraintApplied2 = false;
        auto xl = detail::constrain(x, c.lhs_, constraintApplied1);
        auto xr = detail::constrain(x, c.rhs_, constraintApplied2);
        constraintApplied = constraintApplied1 || constraintApplied2;
        if (constraintApplied1 && constraintApplied2)
        {
            return CInterval::make_with_identity(x, UInterval{ std::min(lower(xl), lower(xr)), std::max(upper(xl), upper(xr)) });
        }
        else if (constraintApplied1)
        {
            return xl;
        }
        else if (constraintApplied2)
        {
            return xr;
        }
    }
    return x;
}


}  // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_INTERVAL_HPP_
