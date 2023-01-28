
#include <array>
#include <ranges>
#include <numeric>
#include <iterator>
#include <algorithm>
#include <initializer_list>
namespace ranges = std::ranges;

#include <gsl-lite/gsl-lite.hpp>  // for gsl_Expects(), gsl_ExpectsDebug(), gsl_ExpectsAudit()
using gsl_lite::dim;
using gsl_lite::index;

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <makeshift/ranges.hpp>  // for index_range
using makeshift::index_range;

#include <intervals/set.hpp>
#include <intervals/interval.hpp>
#include <intervals/algorithm.hpp>
using namespace intervals;


template <typename T>
T
interpolate_linear_scalar(
        ranges::random_access_range auto&& xs,  // points  xᵢ  with  x₁ ≤ ... ≤ xₙ
        ranges::random_access_range auto&& ys,  // corresponding values  yᵢ
        T x) {
    dim n = ranges::ssize(xs);

    gsl_Expects(n >= 2);
    gsl_ExpectsDebug(ranges::ssize(ys) == n);
    gsl_ExpectsAudit(ranges::is_sorted(xs));

    auto pos = ranges::lower_bound(xs, x);
    index i = pos - ranges::begin(xs);

    T result;

        // For values  x < x₁ , just extend the first point of support  y₁  as a constant.
    bool below = i == 0;
    if (below) {
        result = ys[0];
    }

        // For values  x > xₙ , just extend the last point of support  yₙ  as a constant.
    bool above = i == n;
    if (above) {
        result = ys[n - 1];
    }

        // Otherwise, return linear interpolation  yᵢ + (x - xᵢ)/(xᵢ₊₁ - xᵢ)⋅(yᵢ₊₁ - yᵢ) .
    if (!below && !above) {


        auto x0 = xs[i - 1];
        auto x1 = xs[i];
        auto y0 = ys[i - 1];
        auto y1 = ys[i];
    
        result =  y0 + (x - x0)/(x1 - x0)*(y1 - y0);
    }

    return result;
}

template <typename T>
T
interpolate_linear(
        ranges::random_access_range auto&& xs,  // points  xᵢ  with  x₁ ≤ ... ≤ xₙ
        ranges::random_access_range auto&& ys,  // corresponding values  yᵢ
        T x) {
    dim n = ranges::ssize(xs);

    gsl_Expects(n >= 2);
    gsl_ExpectsDebug(ranges::ssize(ys) == n);
    gsl_ExpectsAudit(ranges::is_sorted(xs));

    auto [preds, pos] = lower_bound(xs, x);
    auto i = pos - ranges::begin(xs);

    auto result = T{ };

        // For values  x < x₁ , extend the first point of support  y₁  as a constant.
    auto below = i == 0;
    if (possibly(below)) {
        assign_partial(result, ys[0]);
    }

        // For values  x > xₙ , extend the last point of support  yₙ  as a constant.
    auto above = i == n;
    if (possibly(above)) {
        assign_partial(result, ys[n - 1]);
    }

        // Otherwise, return linear interpolation  yᵢ + (x - xᵢ)/(xᵢ₊₁ - xᵢ)⋅(yᵢ₊₁ - yᵢ) .
    if (auto c = !below & !above; possibly(c)) {
        auto ic = constrain(i, c);
        for (index j : enumerate(ic)) {
            auto x0 = xs[j - 1];
            auto x1 = xs[j];
            auto y0 = ys[j - 1];
            auto y1 = ys[j];
            auto xc = constrain(x, preds[j]);  // imposes  xᵢ₋₁ ≤ x < xᵢ
            assign_partial(result, y0 + (xc - x0)/(x1 - x0)*(y1 - y0));
        }
    }

    return result;
}

int main() {
    auto xs = std::array{ 1., 2., 4., 8. };
    auto ys = std::array{ 1., 3., 9., -3. };
    auto y1s = [&](auto const& x) {
        return interpolate_linear_scalar(xs, ys, x);
    };
    auto y1 = [&](auto const& x) {
        return interpolate_linear(xs, ys, x);
    };

        // Linear interpolation:
    fmt::print("y({}) = {}\n", 1.5, y1s(1.5));
    fmt::print("y({}) = {}\n", 1.5, y1(1.5));
    fmt::print("y({}) = {}\n",
        interval{1.5}, y1(interval{1.5}));
    fmt::print("y({}) = {}\n",
        interval{0,1.2}, y1(interval{0,1.2}));
    fmt::print("y({}) = {}\n",
        interval{1.2,1.7}, y1(interval{1.2,1.7}));
    fmt::print("y({}) = {}\n",
        interval{1.5,5}, y1(interval{1.5,5}));
}
// output:
//     y(1.5) = 2
//     y(1.5) = 2
//     y(1.5) = 2
//     y([0, 1.2]) = [1, 1.4]
//     y([1.2, 1.7]) = [1.4, 2.4]
//     y([1.5, 5]) = [2, 9]
