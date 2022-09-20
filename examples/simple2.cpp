
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <intervals/set.hpp>
#include <intervals/interval.hpp>
using namespace intervals;


interval<double>
imax0(interval<double> a, interval<double> b) {
    auto result = interval<double>{ };
    auto cond = a >= b;
    if (maybe(cond)) {
        assign_partial(result, a);
    }
    if (maybe_not(cond)) {
        assign_partial(result, b);
    }
    return result;
}
interval<double>
imax(interval<double> a, interval<double> b) {
    auto result = interval<double>{ };
    auto cond = a >= b;
    if (maybe(cond)) {
        auto ac = constrain(a, cond);
        assign_partial(result, ac);
    }
    if (maybe_not(cond)) {
        auto bc = constrain(b, !cond);
        assign_partial(result, bc);
    }
    return result;
}
int main() {
    auto a = interval{ 0., 4. };
    auto b = interval{ 1., 2. };
    fmt::print("imax0({}, {}) = {}\n",
        a, b, imax0(a, b));
    fmt::print("imax({}, {}) = {}\n",
        a, b, imax(a, b));
}
// output:
//     imax0([0, 4], [1, 2]) = [0, 4]
//     imax([0, 4], [1, 2]) = [1, 4]
