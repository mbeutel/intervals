
#ifndef INCLUDED_INTERVALS_DETAIL_INTERVAL_HPP_
#define INCLUDED_INTERVALS_DETAIL_INTERVAL_HPP_


#include <cmath>
#include <memory>     // for addressof()
#include <concepts>   // for derived_from<>
#include <iterator>   // for random_access_iterator<>
#include <algorithm>  // for min(), max()

#include <gsl-lite/gsl-lite.hpp>  // for gsl_AssertDebug()

#include <intervals/set.hpp>
#include <intervals/concepts.hpp>

#include <intervals/detail/concepts-internal.hpp>


namespace intervals {


template <typename T> class interval;


namespace detail {


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


template <typename L, typename R> struct is_same_interval : std::false_type { };
template <any_interval L, any_interval R> struct is_same_interval<L, R> : std::is_same<interval_t<L>, interval_t<R>> { };
template <interval_arg T> constexpr bool have_id_v = false;
template <any_interval IntervalT> constexpr bool have_id_v<constrained_interval<IntervalT>> = true;
template <interval_arg T> struct have_id : std::bool_constant<have_id_v<T>> { };
template <any_interval L, interval_arg T>
struct can_compare_id : std::conjunction<is_same_interval<L, T>, have_id<T>> { };
template <any_interval L, interval_arg T>
constexpr bool can_compare_id_v = can_compare_id<L, T>::value;

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
    constexpr inline T
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
    if constexpr (detail::floating_point_operands<L, R>)
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
    if constexpr (detail::floating_point_operands<L, R>)
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
less_constraint(condition, L, R) -> less_constraint<L, R>;
template <typename L, typename R>
struct equal_constraint : condition  // lhs = rhs
{
    L lhs_;
    R rhs_;
};
template <typename L, typename R>
equal_constraint(condition, L, R) -> equal_constraint<L, R>;
template <typename L, typename R>
struct not_equal_constraint : condition  // lhs ≠ rhs
{
    L lhs_;
    R rhs_;
};
template <typename L, typename R>
not_equal_constraint(condition, L, R) -> not_equal_constraint<L, R>;
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
constexpr not_equal_constraint<L, R>
operator !(equal_constraint<L, R> const& c)
{
    return { { !c.as_set() }, c.lhs_, c.rhs_ };
}
template <typename L, typename R>
constexpr equal_constraint<L, R>
operator !(not_equal_constraint<L, R> const& c)
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
    return x;
}
template <typename IntervalT, typename L, typename R>
constexpr as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, less_equal_constraint<L, R> const& c, bool& constraintConsidered, bool optional = false)
{
    using CInterval = as_constrained_interval_t<IntervalT>;
    using UInterval = interval_t<IntervalT>;
    using T = common_interval_value_t<L, R>;

    if constexpr (can_compare_id_v<IntervalT, L>)
    {
        if (matches_identity(x, c.lhs_))
        {
            constraintConsidered = true;
            if (c.contains(true))
            {
                    // Apply constraint  x ≤ rhs⁺ .
                auto xlo = lower(x);
                auto xhi = upper(x);
                auto rhi = upper(c.rhs_);
                gsl_AssertDebug(xlo <= rhi);
                return CInterval::make_with_identity(x, UInterval{ xlo, std::min<T>(xhi, rhi) });
            }
            else
            {
                gsl_Assert(optional);
            }
        }
    }
    if constexpr (can_compare_id_v<IntervalT, R>)
    {
        if (matches_identity(x, c.rhs_))
        {
            constraintConsidered = true;
            if (c.contains(true))
            {
                    // Apply constraint  lhs⁻ ≤ x .
                auto xlo = lower(x);
                auto xhi = upper(x);
                auto llo = lower(c.lhs_);
                gsl_AssertDebug(llo <= xhi);
                return CInterval::make_with_identity(x, UInterval{ std::max<T>(llo, xlo), xhi });
            }
            else
            {
                gsl_Assert(optional);
            }
        }
    }
    return x;
}
template <typename IntervalT, typename L, typename R>
constexpr as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, less_constraint<L, R> const& c, bool& constraintConsidered, bool optional = false)
{
    using CInterval = as_constrained_interval_t<IntervalT>;
    using UInterval = interval_t<IntervalT>;
    using T = common_interval_value_t<L, R>;

    if constexpr (can_compare_id_v<IntervalT, L>)
    {
        if (matches_identity(x, c.lhs_))
        {
            constraintConsidered = true;
            if (c.contains(true))
            {
                    // Apply constraint  x < rhs⁺ .
                auto xlo = lower(x);
                auto xhi = upper(x);
                auto rhi = upper(c.rhs_);
                gsl_AssertDebug(xlo < rhi);
                return CInterval::make_with_identity(x, UInterval{ xlo, std::min<T>(xhi, pred(rhi)) });
            }
            else
            {
                gsl_Assert(optional);
            }
        }
    }
    if constexpr (can_compare_id_v<IntervalT, R>)
    {
        if (matches_identity(x, c.rhs_))
        {
            constraintConsidered = true;
            if (c.contains(true))
            {
                    // Apply constraint  lhs⁻ < x .
                auto xlo = lower(x);
                auto xhi = upper(x);
                auto llo = lower(c.lhs_);
                gsl_AssertDebug(llo < xhi);
                return CInterval::make_with_identity(x, UInterval{ std::max<T>(succ(llo), xlo), xhi });
            }
            else
            {
                gsl_Assert(optional);
            }
        }
    }
    return x;
}
template <typename IntervalT, typename L, typename R>
constexpr as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, equal_constraint<L, R> const& c, bool& constraintConsidered, bool optional = false)
{
    using CInterval = as_constrained_interval_t<IntervalT>;
    using UInterval = interval_t<IntervalT>;
    using T = common_interval_value_t<L, R>;

    if constexpr (can_compare_id_v<IntervalT, L> || can_compare_id_v<IntervalT, R>)
    {
        if (matches_identity(x, c.lhs_) || matches_identity(x, c.rhs_))
        {
            constraintConsidered = true;
            if (c.contains(true))
            {
                    // Apply constraint  lhs = rhs .
                auto llo = lower(c.lhs_);
                auto lhi = upper(c.lhs_);
                auto rlo = lower(c.rhs_);
                auto rhi = upper(c.rhs_);
                gsl_AssertDebug(rhi >= llo && rlo <= lhi);  // intervals overlap
                return CInterval::make_with_identity(x, UInterval{ std::max<T>(llo, rlo), std::min<T>(lhi, rhi) });  // overlapping interval
            }
            else
            {
                gsl_Assert(optional);
            }
        }
        return x;
    }
    else return x;
}
template <typename IntervalT, typename L, typename R>
constexpr as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, not_equal_constraint<L, R> const& c, bool& constraintConsidered, bool optional = false)
{
    using CInterval = as_constrained_interval_t<IntervalT>;
    using UInterval = interval_t<IntervalT>;

    if constexpr (can_compare_id_v<IntervalT, L> || can_compare_id_v<IntervalT, R>)
    {
        if (matches_identity(x, c.lhs_) || matches_identity(x, c.rhs_))
        {
            constraintConsidered = true;
            if (c.contains(true))
            {
                    // Apply constraint  lhs ≠ rhs .
                auto llo = lower(c.lhs_);
                auto lhi = upper(c.lhs_);
                auto rlo = lower(c.rhs_);
                auto rhi = upper(c.rhs_);
                gsl_AssertDebug(llo != rhi || lhi != rlo);  // intervals not identical
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
            else
            {
                gsl_Assert(optional);
            }
        }
        return x;
    }
    else return x;
}
template <typename IntervalT, std::derived_from<condition> L, std::derived_from<condition> R>
constexpr as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, constraint_conjunction<L, R> const& c, bool& constraintConsidered, bool optional = false)
{
    using CInterval = as_constrained_interval_t<IntervalT>;
    using UInterval = interval_t<IntervalT>;

    auto xl = detail::constrain(x, c.lhs_, constraintConsidered, optional);
    auto xr = detail::constrain(x, c.rhs_, constraintConsidered, optional);
    if (c.contains(true))
    {
            // Make sure that the two constraints overlap.
        gsl_AssertDebug(upper(xl) >= lower(xr) && lower(xl) <= upper(xr));

        return CInterval::make_with_identity(x, UInterval{ std::max(lower(xl), lower(xr)), std::min(upper(xl), upper(xr)) });
    }
    // If `!c.contains(true) && !optional`, one of the nested calls to `constrain()` will have triggered an assertion.
    return x;
}
template <typename IntervalT, std::derived_from<condition> L, std::derived_from<condition> R>
constexpr as_constrained_interval_t<IntervalT>
constrain(IntervalT const& x, constraint_disjunction<L, R> const& c, bool& constraintConsidered, bool optional = false)
{
    using CInterval = as_constrained_interval_t<IntervalT>;
    using UInterval = interval_t<IntervalT>;

    bool constraintConsidered1 = false;
    bool constraintConsidered2 = false;
    bool nestedOptional = true;
    auto xl = detail::constrain(x, c.lhs_, constraintConsidered1, nestedOptional);
    auto xr = detail::constrain(x, c.rhs_, constraintConsidered2, nestedOptional);
    constraintConsidered = constraintConsidered1 || constraintConsidered2;
    bool blhs = constraintConsidered1 && c.lhs_.contains(true);
    bool brhs = constraintConsidered2 && c.rhs_.contains(true);
    gsl_Assert(c.contains(true) || optional);
    if (blhs && brhs)
    {
        return CInterval::make_with_identity(x, UInterval{ std::min(lower(xl), lower(xr)), std::max(upper(xl), upper(xr)) });
    }
    else if (blhs)
    {
        return xl;
    }
    else if (brhs)
    {
        return xr;
    }
    return x;
}


}  // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_INTERVAL_HPP_
