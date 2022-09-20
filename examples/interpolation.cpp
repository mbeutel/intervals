

#include <initializer_list>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <intervals/set.hpp>
#include <intervals/interval.hpp>
using namespace intervals;

#include <array>
#include <ranges>
#include <numeric>
#include <iterator>
#include <algorithm>

#include <gsl-lite/gsl-lite.hpp>  // for gsl_Expects(), gsl_ExpectsDebug(), gsl_ExpectsAudit()
using gsl_lite::index;

#include <makeshift/ranges.hpp>  // for index_range
using makeshift::index_range;

#include <intervals/algorithm.hpp>


template <std::ranges::random_access_range R1, std::ranges::random_access_range R2, typename XT>
auto
interpolate_nearest_neighbour(
        R1&& xs,  // points  xᵢ  with  x₁ ≤ ... ≤ xₙ
        R2&& ys,  // corresponding values  yᵢ
        XT const& x) {
    gsl_ExpectsDebug(std::ranges::size(xs) == std::ranges::size(ys));
    gsl_Expects(!std::ranges::empty(xs));  // at least one point of support
    gsl_ExpectsAudit(std::ranges::is_sorted(xs));

    using YT = propagate_set_t<XT, std::ranges::range_value_t<R2>>;
    auto result = YT{ };
    if (std::ranges::ssize(xs) == 1)
    {
            // If we have only a single point of support, just return it.
        assign(result, xs[0]);
    }
    else  // std::ranges::ssize(xs) > 1
    {
        auto i = *lower_bound(
            index_range(std::ranges::ssize(xs) - 1),
            x,
            [&xs](index i, auto const& x)
            {
                auto xhalf = std::midpoint(xs[i], xs[i + 1]);
                return xhalf < x;
            });
        assign(result, at(xs, i));
    }
    return result;
}

template <std::random_access_iterator It1, std::random_access_iterator It2, typename XT>
auto
interpolate_linear_0(
    It1 xfirst, It1 xlast,  // range of values  xᵢ  in ascending order
    It2 yfirst, It2 ylast,  // range of corresponding values  yᵢ
    XT x)
{
    gsl_ExpectsDebug(ylast - yfirst == xlast - xfirst);  // both ranges have the same length
    gsl_Expects(xfirst != xlast);  // at least one point of support
    gsl_ExpectsAudit(std::is_sorted(xfirst, xlast));  // range of elements  xᵢ  is sorted in ascending order

    auto xpos = std::lower_bound(xfirst, xlast, x);

        // For values  x < x₁ , just extend the first point of support  y₁  as a constant.
    bool below = xpos == xfirst;
    if (below)
    {
        return *yfirst;
    }

        // For values  x > xₙ , just extend the last point of support  yₙ  as a constant.
    bool above = xpos == xlast;
    if (above)
    {
        return *std::prev(ylast);
    }

        // Otherwise, return linear interpolation  yᵢ₋₁ + (x - xᵢ₋₁)/(xᵢ - xᵢ₋₁)⋅(yᵢ - yᵢ₋₁) .
    auto i = xpos - xfirst;
    auto x0 = xfirst[i - 1];
    auto x1 = xfirst[i];
    auto y0 = yfirst[i - 1];
    auto y1 = yfirst[i];
    return y0 + (x - x0)/(x1 - x0)*(y1 - y0);
}
template <std::ranges::random_access_range R1, std::ranges::random_access_range R2, typename XT>
auto
interpolate_linear(
        R1&& xs,  // points  xᵢ  with  x₁ ≤ ... ≤ xₙ
        R2&& ys,  // corresponding values  yᵢ
        XT x) {
    gsl_ExpectsDebug(std::ranges::size(xs) == std::ranges::size(ys));
    gsl_Expects(!std::ranges::empty(xs));  // at least one point of support
    gsl_ExpectsAudit(std::ranges::is_sorted(xs));

    auto n = std::ranges::ssize(xs);

    auto xpos = lower_bound(xs, x);
    using YT = propagate_set_t<XT, std::ranges::range_value_t<R2>>;
    auto result = YT{ };

        // For values  x < x₁ , just extend the first point of support  y₁  as a constant.
    auto below = xpos == std::ranges::begin(xs);
    if (maybe(below)) {
        assign_partial(result, ys[0]);
    }

        // For values  x > xₙ , just extend the last point of support  yₙ  as a constant.
    auto above = xpos == std::ranges::end(xs);
    if (maybe(above)) {
        assign_partial(result, ys[n - 1]);
    }

        // Otherwise, return linear interpolation  yᵢ₋₁ + (x - xᵢ₋₁)/(xᵢ - xᵢ₋₁)⋅(yᵢ - yᵢ₋₁) .
    if (auto cond = !below & !above; maybe(cond)) {
        auto xposc = constrain(xpos, cond);
        auto ii = xposc - std::ranges::begin(xs);
        for (gsl_lite::index i : intervals::enumerate(ii)) {
            auto x0 = xs[i - 1];
            auto x1 = xs[i];
            auto y0 = ys[i - 1];
            auto y1 = ys[i];
            auto xc = constrain(x, (x >= x0) & (x < x1));
            assign_partial(result, y0 + (xc - x0)/(x1 - x0)*(y1 - y0));
        }
    }

    return result;
}

int main() {
    auto xs = std::array{ 1., 2., 4., 8. };
    auto ys = std::array{ 1., 3., 9., 3. };
    auto y0 = [&](auto const& x) {
        return interpolate_nearest_neighbour(xs, ys, x);
    };
    auto y1 = [&](auto const& x) {
        return interpolate_linear(xs, ys, x);
    };

        // Constant interpolation:
    fmt::print("y|c({}) = {}\n", 1.5, y0(1.5));
    fmt::print("y|c({}) = {}\n", interval{ 1.5 }, y0(interval{ 1.5 }));
    fmt::print("y|c({}) = {}\n", interval{ 0, 1.2 }, y0(interval{ 0, 1.2 }));
    fmt::print("y|c({}) = {}\n", interval{ 1.2, 1.7 }, y0(interval{ 1.2, 1.7 }));
    fmt::print("y|c({}) = {}\n", interval{ 1.5, 5 }, y0(interval{ 1.5, 5 }));

        // Linear interpolation:
    fmt::print("y|l({}) = {}\n", 1.5, y1(1.5));
    fmt::print("y|l({}) = {}\n", interval{ 1.5 }, y1(interval{ 1.5 }));
    fmt::print("y|l({}) = {}\n", interval{ 0, 1.2 }, y1(interval{ 0, 1.2 }));
    fmt::print("y|l({}) = {}\n", interval{ 1.2, 1.7 }, y1(interval{ 1.2, 1.7 }));
    fmt::print("y|l({}) = {}\n", interval{ 1.5, 5 }, y1(interval{ 1.5, 5 }));
}
// output:
//     y|c(1.5) = 1
//     y|c(1.5) = 1
//     y|c([0, 1.2]) = 1
//     y|c([1.2, 1.7]) = 1
//     y|c([1.5, 5]) = [1, 4]
//     y|l(1.5) = 2
//     y|l(1.5) = 2
//     y|l([0, 1.2]) = [1, 1.4]
//     y|l([1.2, 1.7]) = [1.4, 2.4]
//     y|l([1.5, 5]) = [2, 9]
