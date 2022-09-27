
#include <array>
#include <ranges>
#include <numeric>
#include <iterator>
#include <algorithm>
#include <initializer_list>
namespace ranges = std::ranges;

#include <gsl-lite/gsl-lite.hpp>  // for gsl_Expects(), gsl_ExpectsDebug(), gsl_ExpectsAudit()
using gsl_lite::index;

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <makeshift/ranges.hpp>  // for index_range
using makeshift::index_range;

#include <intervals/set.hpp>
#include <intervals/interval.hpp>
#include <intervals/algorithm.hpp>
using namespace intervals;


template <ranges::random_access_range XR, ranges::random_access_range YR, typename X>
auto
interpolate_linear(
        XR&& xs,  // points  xᵢ  with  x₁ ≤ ... ≤ xₙ
        YR&& ys,  // corresponding values  yᵢ
        X x) {
    gsl_Expects(ranges::size(xs) >= 2);
    gsl_ExpectsDebug(ranges::size(xs) == ranges::size(ys));
    gsl_ExpectsAudit(ranges::is_sorted(xs));

    auto n = ranges::ssize(xs);

    auto [partitioning, pos] = lower_bound(xs, x);

    using Y = propagate_set_t<X, ranges::range_value_t<YR>>;
    auto result = Y{ };

        // For values  x < x₁ , just extend the first point of support  y₁  as a constant.
    auto below = pos == ranges::begin(xs);
    if (maybe(below)) {
        assign_partial(result, ys[0]);
    }

        // For values  x > xₙ , just extend the last point of support  yₙ  as a constant.
    auto above = pos == ranges::end(xs);
    if (maybe(above)) {
        assign_partial(result, ys[n - 1]);
    }

        // Otherwise, return linear interpolation  yᵢ + (x - xᵢ)/(xᵢ₊₁ - xᵢ)⋅(yᵢ₊₁ - yᵢ) .
    if (auto cond = !below & !above; maybe(cond)) {
        auto posc = constrain(pos, cond);
        auto ii = posc - ranges::begin(xs);
        for (gsl_lite::index i : intervals::enumerate(ii)) {
            auto x0 = xs[i - 1];
            auto x1 = xs[i];
            auto y0 = ys[i - 1];
            auto y1 = ys[i];
            auto xc = constrain(x, partitioning[i]);  // imposes  xᵢ₋₁ ≤ x < xᵢ
            assign_partial(result, y0 + (xc - x0)/(x1 - x0)*(y1 - y0));
        }
    }

    return result;
}

int main() {
    auto xs = std::array{ 1., 2., 4., 8. };
    auto ys = std::array{ 1., 3., 9., 3. };
    auto y1 = [&](auto const& x) {
        return interpolate_linear(xs, ys, x);
    };

        // Linear interpolation:
    fmt::print("y({}) = {}\n", 1.5, y1(1.5));
    fmt::print("y({}) = {}\n", interval{ 1.5 }, y1(interval{ 1.5 }));
    fmt::print("y({}) = {}\n", interval{ 0, 1.2 }, y1(interval{ 0, 1.2 }));
    fmt::print("y({}) = {}\n", interval{ 1.2, 1.7 }, y1(interval{ 1.2, 1.7 }));
    fmt::print("y({}) = {}\n", interval{ 1.5, 5 }, y1(interval{ 1.5, 5 }));
}
// output:
//     y(1.5) = 2
//     y(1.5) = 2
//     y([0, 1.2]) = [1, 1.4]
//     y([1.2, 1.7]) = [1.4, 2.4]
//     y([1.5, 5]) = [2, 9]
