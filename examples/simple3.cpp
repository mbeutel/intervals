
#include <initializer_list>

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <intervals/set.hpp>
#include <intervals/interval.hpp>
using namespace intervals;


template <typename T>
T
s(T x) {
    auto result = T{ };
    auto cond = x >= 0;
    if (maybe(cond)) {
        auto xc = constrain(x, cond);
        assign_partial(result, sqrt(xc));
    }
    if (maybe_not(cond)) {
        assign_partial(result, 0);
    }
    return result;
}
int main() {
    for (auto x : { -1., 0., 2. }) {
        fmt::print("s({}) = {}\n", x, s(x));
    }
    auto ix = interval{ -1., 2. };
    fmt::print("s({}) = {}\n", ix, s(ix));
}
// output:
//     s(-1) = 0
//     s(0) = 0
//     s(2) = 1.41421
//     s([-1, 2]) = [0, 1.41421]
