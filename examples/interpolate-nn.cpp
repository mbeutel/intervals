
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


template <typename T>
T
interpolate_nearest_neighbour_scalar(
        ranges::random_access_range auto&& xs,  // points  xᵢ  with  x₁ ≤ ... ≤ xₙ
        ranges::random_access_range auto&& ys,  // corresponding values  yᵢ
        T x) {
    gsl_Expects(ranges::size(xs) >= 2);
    gsl_ExpectsDebug(ranges::size(xs) == ranges::size(ys));
    gsl_ExpectsAudit(ranges::is_sorted(xs));

    auto it = ranges::partition_point(
        index_range(ranges::ssize(xs) - 1),
        [&xs, &x](index i) {
            auto xhalf = std::midpoint(xs[i], xs[i + 1]);
            return xhalf < x;
        });
    index i = *it;
    return at(ys, i);
}

template <typename T>
T
interpolate_nearest_neighbour(
        ranges::random_access_range auto&& xs,  // points  xᵢ  with  x₁ ≤ ... ≤ xₙ
        ranges::random_access_range auto&& ys,  // corresponding values  yᵢ
        T x) {
    gsl_Expects(ranges::size(xs) >= 2);
    gsl_ExpectsDebug(ranges::size(xs) == ranges::size(ys));
    gsl_ExpectsAudit(ranges::is_sorted(xs));

    auto [_, it] = intervals::partition_point(
        index_range(ranges::ssize(xs) - 1),
        [&xs, &x](index i) {
            auto xhalf = std::midpoint(xs[i], xs[i + 1]);
            return xhalf < x;
        });
    auto i = *it;
    return at(ys, i);
}

int main() {
    auto xs = std::array{1.,2.,4.,8.};
    auto ys = std::array{1.,3.,9.,-3.};
    auto y0s = [&](double x) {
        return interpolate_nearest_neighbour_scalar(
            xs, ys, x);
    };
    auto y0 = [&](auto const& x) {
        return interpolate_nearest_neighbour(
            xs, ys, x);
    };
    fmt::print("y({}) = {}\n", 1.2, y0s(1.2));
    fmt::print("y({}) = {}\n", 1.2, y0(1.2));
    fmt::print("y({}) = {}\n",
        interval{1.2}, y0(interval{1.2}));
    fmt::print("y({}) = {}\n",
        interval{0,2.4}, y0(interval{0,2.4}));
    fmt::print("y({}) = {}\n",
        interval{1.2,1.7}, y0(interval{1.2,1.7}));
    fmt::print("y({}) = {}\n",
        interval{1.5,5}, y0(interval{1.5,5}));
}
// output:
//     y(1.2) = 1
//     y(1.2) = 1
//     y([0, 2.4]) = [1, 3]
//     y([1.2, 1.7]) = [1, 3]
//     y([1.5, 5]) = [1, 9]
