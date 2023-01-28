
#include <fmt/core.h>
#include <fmt/ostream.h>

#include <intervals/set.hpp>
#include <intervals/interval.hpp>
using namespace intervals;


interval<double>
f1(interval<double> x) {
    return square(x) - 2*x;
}
interval<double>
f2(interval<double> x) {
    return square(x - 1) - 1;
}
int main() {
    auto x = interval{ 0., 1. };
    fmt::print("f1({}) = {}\n", x, f1(x));
    fmt::print("f1({}) > 0: {}\n", x, f1(x) > 0);
    fmt::print("possibly(f1({}) > 0): {}\n",
        x, possibly(f1(x) > 0));
    fmt::print("f2({}) = {}\n", x, f2(x));
    fmt::print("f2({}) > 0: {}\n", x, f2(x) > 0);
    fmt::print("possibly(f2({}) > 0): {}\n",
        x, possibly(f2(x) > 0));
}
// output:
//     f1([0, 1]) = [-2, 1]
//     f1([0, 1]) > 0: { 0, 1 }
//     possibly(f1([0, 1]) > 0): true
//     f2([0, 1]) = [-1, 0]
//     f2([0, 1]) > 0: { 0 }
//     possibly(f2([0, 1]) > 0): false
