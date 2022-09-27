
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


template <ranges::random_access_range XR,
          ranges::random_access_range YR,
          typename X>
auto
interpolate_nearest_neighbour(
        XR&& xs,  // points  xᵢ  with  x₁ ≤ ... ≤ xₙ
        YR&& ys,  // corresponding values  yᵢ
        X x) {
    gsl_Expects(ranges::size(xs) >= 2);
    gsl_ExpectsDebug(ranges::size(xs) == ranges::size(ys));
    gsl_ExpectsAudit(ranges::is_sorted(xs));

    auto [_, it] = partition_point(
        index_range(ranges::ssize(xs) - 1),
        [&xs, &x](index i) {
            auto xhalf = std::midpoint(xs[i], xs[i + 1]);
            return xhalf < x;
        });
    auto index = *it;
    return at(ys, index);
}

int main() {
    auto xs = std::array{1.,2.,4.,8.};
    auto ys = std::array{1.,3.,9.,3.};
    auto y0 = [&](auto const& x) {
        return interpolate_nearest_neighbour(
            xs, ys, x);
    };
    fmt::print("y({}) = {}\n", 1.5, y0(1.5));
    fmt::print("y({}) = {}\n",
        interval{1.5}, y0(interval{1.5}));
    fmt::print("y({}) = {}\n",
        interval{0,1.2}, y0(interval{0,1.2}));
    fmt::print("y({}) = {}\n",
        interval{1.2,1.7}, y0(interval{1.2,1.7}));
    fmt::print("y({}) = {}\n",
        interval{1.5,5}, y0(interval{1.5,5}));
}
// output:
//     y(1.5) = 1
//     y(1.5) = 1
//     y([0, 1.2]) = 1
//     y([1.2, 1.7]) = 1
//     y([1.5, 5]) = [1, 4]
