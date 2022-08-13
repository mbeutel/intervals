
#include <limits>
#include <numbers>

#include <gsl-lite/gsl-lite.hpp>  // for fail_fast, type_identity<>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <intervals/interval.hpp>


namespace {

namespace gsl = ::gsl_lite;


TEST_CASE("interval<>", "interval arithmetic")
{
    using intervals::set;
    using intervals::interval;
    constexpr double inf = std::numeric_limits<double>::infinity();
    constexpr double nan = std::numeric_limits<double>::quiet_NaN();

    SECTION("value()")
    {
        CHECK(interval{ 1., 1. }.value() == 1.);
        CHECK_THROWS_AS(interval<double>{ }.value(), gsl::fail_fast);
        CHECK_THROWS_AS((interval{ 1., 2. }.value()), gsl::fail_fast);
    }
    SECTION("mixed relational operators")
    {
        auto [xbelow, a, xin, b, xabove] = GENERATE_COPY(
            //          below   a       in      b       above
            std::array{ nan,    -inf,   -inf,   -inf,   -1.  },
            std::array{ nan,    -inf,   -2.,    -1.,    -0.5 },
            std::array{ nan,    -inf,   -1.,    -0.,     0.5 },
            std::array{ nan,    -inf,    0.,     1.,     2.  },
            std::array{ nan,    -inf,    0.,     1.,     inf },
            std::array{ nan,    -inf,    0.,     inf,    nan },
            std::array{ -2.,    -1.5,   -1.5,   -1.5,   -1.  },
            std::array{ -2.,    -1.5,   -1.,    -0.1,    0.  },
            std::array{ -2.,    -1.5,   -1.,     0.,     0.5 },
            std::array{ -2.,    -1.5,    0.,     1.,     2.  },
            std::array{ -2.,    -1.5,    0.,     inf,    nan },
            std::array{ -2.,     0.,     0.,     0.,     1.  },
            std::array{ -2.,     0.,     0.1,    1.,     1.5 },
            std::array{ -2.,     0.,     1.,     inf,    nan },
            std::array{ -2.,     0.1,    0.1,    0.1,    1.  },
            std::array{  0.,     0.1,    0.1,    0.1,    1.  },
            std::array{  0.01,   0.1,    0.1,    0.1,    1.  },
            std::array{ -2.,     0.1,    1.,     2.,     3.  },
            std::array{ -2.,     0.1,    1.,     inf,    nan }
        );
        auto x = interval{ a, b };
        CAPTURE(x);

        if (!std::isnan(xbelow))
        {
            CHECK(definitely(xbelow < x));
            CHECK(!maybe(xbelow >= x));
        }
        CHECK(definitely(a <= x));
        CHECK(!maybe(a > x));
        CHECK(maybe(xin <= x));
        CHECK(!definitely(xin > x));
        CHECK(maybe(xin >= x));
        CHECK(!definitely(xin < x));
        CHECK(definitely(b >= x));
        CHECK(!maybe(b < x));
        if (!std::isnan(xabove))
        {
            CHECK(definitely(xabove > x));
            CHECK(!maybe(xabove <= x));
        }
    }

    // The `interval<>` type implements the transposition of the  min/max  operator and an unary/binary algebraic function, i.e.
    //
    //     x ∈ I := [a, b]
    //     y ∈ J := [c, d]
    //     fₘᵢₙ(I) := min{ f(x) | x ∈ I }
    //     fₘₐₓ(I) := max{ f(x) | x ∈ I }
    //     gₘᵢₙ(I, J) := min{ g(x, y) | x ∈ I, y ∈ J }
    //     gₘₐₓ(I, J) := max{ g(x, y) | x ∈ I, y ∈ J } .
    //
    // For monotonic functions  f, g  , this can be simplified to
    //
    //     fₘᵢₙ(I) = min{ f(a); f(b) }
    //     fₘₐₓ(I) = max{ f(a); f(b) }
    //     gₘᵢₙ(I, J) = min{ g(a, c); g(a, d); g(b, c); g(b, d) }
    //     gₘₐₓ(I, J) = max{ g(a, c); g(a, d); g(b, c); g(b, d) } .
    //
    // We use this definition to write property-based tests for the operations of the `interval<>` type.

    SECTION("unary operators")
    {
        auto [xbelow, a, xin, b, xabove] = GENERATE_COPY(
            //          below   a       in      b       above
            std::array{ nan,    -inf,   -inf,   -inf,   -1.  },
            std::array{ nan,    -inf,   -2.,    -1.,    -0.5 },
            std::array{ nan,    -inf,   -1.,    -0.,     0.5 },
            std::array{ nan,    -inf,    0.,     1.,     2.  },
            std::array{ nan,    -inf,    0.,     1.,     inf },
            std::array{ nan,    -inf,    0.,     inf,    nan },
            std::array{ -2.,    -1.5,   -1.5,   -1.5,   -1.  },
            std::array{ -2.,    -1.5,   -1.,    -0.1,    0.  },
            std::array{ -2.,    -1.5,   -1.,    -0.,     0.5 },
            std::array{ -2.,    -1.5,    0.,     1.,     2.  },
            std::array{ -2.,    -1.5,    0.,     inf,    nan },
            std::array{ -2.,     0.,     0.,     0.,     1.  },
            std::array{ -2.,     0.,     0.1,    1.,     1.5 },
            std::array{ -2.,     0.,     1.,     inf,    nan },
            std::array{ -2.,     0.1,    0.1,    0.1,    1.  },
            std::array{  0.,     0.1,    0.1,    0.1,    1.  },
            std::array{  0.01,   0.1,    0.1,    0.1,    1.  },
            std::array{ -2.,     0.1,    1.,     2.,     3.  },
            std::array{ -2.,     0.1,    1.,     inf,    nan }
        );
        auto x = interval{ a, b };
        CAPTURE(x);

        SECTION("operator -")
        {
            CHECK((-x).lower() == std::min(-a, -b));
            CHECK((-x).upper() == std::max(-a, -b));
        }
        SECTION("square()")
        {
            if (a <= 0 && b >= 0)  //  0 ∈ [a, b]
            {
                CHECK(square(x).lower() == 0);
            }
            else  //  0 ∉ [a, b]
            {
                CHECK(square(x).lower() == std::min(intervals::square(a), intervals::square(b)));
            }
            CHECK(square(x).upper() == std::max(intervals::square(a), intervals::square(b)));
        }
        SECTION("sqrt()")
        {
            if (a >= 0)
            {
                CHECK(sqrt(x).lower() == std::min(intervals::sqrt(a), intervals::sqrt(b)));
                CHECK(sqrt(x).upper() == std::max(intervals::sqrt(a), intervals::sqrt(b)));
            }
        }
        SECTION("cbrt()")
        {
            CHECK(cbrt(x).lower() == std::min(std::cbrt(a), std::cbrt(b)));
            CHECK(cbrt(x).upper() == std::max(std::cbrt(a), std::cbrt(b)));
        }
        SECTION("log()")
        {
            if (a >= 0)
            {
                CHECK(log(x).lower() == std::min(intervals::log(a), intervals::log(b)));
                CHECK(log(x).upper() == std::max(intervals::log(a), intervals::log(b)));
            }
        }
        SECTION("exp()")
        {
            if (b < 1.e2)
            {
                CHECK(exp(x).lower() == std::min(std::exp(a), std::exp(b)));
                CHECK(exp(x).upper() == std::max(std::exp(a), std::exp(b)));
            }
        }
        SECTION("sgn()")
        {
            CHECK(maybe(sgn(x) == intervals::positive_sign) == maybe(x > 0));
            CHECK(maybe(sgn(x) == intervals::zero_sign) == maybe(x == 0));
            CHECK(maybe(sgn(x) == intervals::negative_sign) == maybe(x < 0));
        }
        SECTION("isinf()")
        {
            CHECK(definitely(isinf(x)) == (std::isinf(x.lower()) && x.lower() == x.upper()));
            CHECK(maybe(isinf(x)) == (std::isinf(x.lower()) || std::isinf(x.upper())));
            CHECK(definitely(isfinite(x)) == (!std::isinf(x.lower()) && !std::isinf(x.upper())));
            CHECK(maybe(isfinite(x)) == (x.lower() != x.upper() || !std::isinf(x.lower())));
        }
    }
    SECTION("binary operators")
    {
        auto [xbelow, a, xin, b, xabove] = GENERATE_COPY(
            //          below   a       in      b       above
            std::array{ nan,    -inf,   -inf,   -inf,   -1.  },
            std::array{ nan,    -inf,   -2.,    -1.,    -0.5 },
            std::array{ nan,    -inf,   -1.,    -0.,     0.5 },
            std::array{ nan,    -inf,    0.,     1.,     2.  },
            std::array{ nan,    -inf,    0.,     1.,     inf },
            std::array{ nan,    -inf,    0.,     inf,    nan },
            std::array{ -2.,    -1.5,   -1.5,   -1.5,   -1.  },
            std::array{ -2.,    -1.5,   -1.,    -0.1,    0.  },
            std::array{ -2.,    -1.5,   -1.,    -0.,     0.5 },
            std::array{ -2.,    -1.5,    0.,     1.,     2.  },
            std::array{ -2.,    -1.5,    0.,     inf,    nan },
            std::array{ -2.,     0.,     0.,     0.,     1.  },
            std::array{ -2.,     0.,     0.1,    1.,     1.5 },
            std::array{ -2.,     0.,     1.,     inf,    nan },
            std::array{ -2.,     0.1,    0.1,    0.1,    1.  },
            std::array{  0.,     0.1,    0.1,    0.1,    1.  },
            std::array{  0.01,   0.1,    0.1,    0.1,    1.  },
            std::array{ -2.,     0.1,    1.,     2.,     3.  },
            std::array{ -2.,     0.1,    1.,     inf,    nan }
        );
        auto x = interval{ a, b };
        CAPTURE(x);
        auto [ybelow, c, yin, d, yabove] = GENERATE_COPY(
            //          below   a       in      b       above
            std::array{ nan,    -inf,   -inf,   -inf,   -1.  },
            std::array{ nan,    -inf,   -2.,    -1.,    -0.5 },
            std::array{ nan,    -inf,   -1.,    -0.,     0.5 },
            std::array{ nan,    -inf,    0.,     1.,     2.  },
            std::array{ nan,    -inf,    0.,     1.,     inf },
            std::array{ nan,    -inf,    0.,     inf,    nan },
            std::array{ -2.,    -1.5,   -1.5,   -1.5,   -1.  },
            std::array{ -2.,    -1.5,   -1.,    -0.1,    0.  },
            std::array{ -2.,    -1.5,   -1.,    -0.,     0.5 },
            std::array{ -2.,    -1.5,    0.,     1.,     2.  },
            std::array{ -2.,    -1.5,    0.,     inf,    nan },
            std::array{ -2.,     0.,     0.,     0.,     1.  },
            std::array{ -2.,     0.,     0.1,    1.,     1.5 },
            std::array{ -2.,     0.,     1.,     inf,    nan },
            std::array{ -2.,     0.1,    0.1,    0.1,    1.  },
            std::array{  0.,     0.1,    0.1,    0.1,    1.  },
            std::array{  0.01,   0.1,    0.1,    0.1,    1.  },
            std::array{ -2.,     0.1,    1.,     2.,     3.  },
            std::array{ -2.,     0.1,    1.,     inf,    nan }
        );
        auto y = interval{ c, d };
        CAPTURE(y);

        SECTION("operator +")
        {
            auto z = x + y;
            CAPTURE(z);
            bool indefinite = maybe((x ==  inf & y == -inf)
                                  | (x == -inf & y ==  inf));
            if (indefinite)
            {
                    // Subtracting infinities ⇒ NaN
                CHECK(maybe(isnan(z)));
            }
            else
            {
                CHECK(z.lower() == std::min({ a + c, a + d, b + c, b + d }));
                CHECK(z.upper() == std::max({ a + c, a + d, b + c, b + d }));
            }
        }
        SECTION("operator -")
        {
            auto z = x - y;
            CAPTURE(z);
            bool indefinite = maybe((x == -inf & y == -inf)
                                  | (x ==  inf & y ==  inf));
            if (indefinite)
            {
                    // Subtracting infinities ⇒ NaN
                CHECK(maybe(isnan(z)));
            }
            else
            {
                CHECK(z.lower() == std::min({ a - c, a - d, b - c, b - d }));
                CHECK(z.upper() == std::max({ a - c, a - d, b - c, b - d }));
            }
        }
        SECTION("operator *")
        {
            SECTION("bound*bound")
            {
                auto z = x*y;
                CAPTURE(z);
                bool indefinite = maybe((isinf(x) & y == 0)
                                      | (isinf(y) & x == 0));
                if (indefinite)
                {
                        // Subtracting infinities ⇒ NaN
                    CHECK(maybe(isnan(z)));
                }
                else
                {
                    CHECK(z.lower() == std::min({ a*c, a*d, b*c, b*d }));
                    CHECK(z.upper() == std::max({ a*c, a*d, b*c, b*d }));
                    CHECK(maybe(z == a*yin));
                    CHECK(maybe(z == xin*c));
                    CHECK(maybe(z == xin*yin));
                    CHECK(maybe(z == b*yin));
                    CHECK(maybe(z == xin*d));
                }
            }
            SECTION("bound*scalar")
            {
                auto z = x*c;
                CAPTURE(z);
                bool indefinite = maybe((isinf(x) & (c == 0))  // extra parentheses to silence Clang's `-Wparentheses`
                                      | (std::isinf(c) & x == 0));
                if (indefinite)
                {
                        // Subtracting infinities ⇒ NaN
                    CHECK(maybe(isnan(z)));
                }
                else
                {
                    CHECK(z.lower() == std::min({ a*c, b*c }));
                    CHECK(z.upper() == std::max({ a*c, b*c }));
                    CHECK(maybe(z == a*c));
                    CHECK(maybe(z == xin*c));
                    CHECK(maybe(z == b*c));
                }
            }
        }
        SECTION("operator /")
        {
            SECTION("bound/bound")
            {
                auto z = x/y;
                CAPTURE(z);
                bool indefinite = maybe((x == 0 & y == 0)
                                      | (isinf(x) & isinf(y)));
                bool infinite = definitely(x != 0) && y.encloses(0);
                if (indefinite)
                {
                        // Subtracting infinities ⇒ NaN
                    CHECK(maybe(isnan(z)));
                }
                else if (infinite)
                {
                        // [a,b]/0 = [-∞,∞]
                    CHECK(z.lower() == -inf);
                    CHECK(z.upper() == inf);
                }
                else
                {
                    CHECK(z.lower() == std::min({ a/c, a/d, b/c, b/d }));
                    CHECK(z.upper() == std::max({ a/c, a/d, b/c, b/d }));
                    CHECK(maybe(z == a/yin));
                    CHECK(maybe(z == xin/c));
                    CHECK(maybe(z == xin/yin));
                    CHECK(maybe(z == b/yin));
                    CHECK(maybe(z == xin/d));
                }
            }
            SECTION("bound/scalar")
            {
                auto z = x/c;
                CAPTURE(z);
                bool indefinite = maybe(((x == 0) & (c == 0))  // extra parentheses to silence Clang's `-Wparentheses`
                                      | (isinf(x) & std::isinf(c)));
                if (indefinite)
                {
                        // Subtracting infinities ⇒ NaN
                    CHECK(maybe(isnan(z)));
                }
                else
                {
                    CHECK(z.lower() == std::min({ a/c, b/c }));
                    CHECK(z.upper() == std::max({ a/c, b/c }));
                    CHECK(maybe(z == a/c));
                    CHECK(maybe(z == xin/c));
                    CHECK(maybe(z == b/c));
                }
            }
            SECTION("scalar/bound")
            {
                auto z = a/y;
                CAPTURE(z);
                bool indefinite = maybe(((a == 0) & (y == 0))  // extra parentheses to silence Clang's `-Wparentheses`
                                      | (std::isinf(a) & isinf(y)));
                bool infinite = a != 0 && y.encloses(0);
                if (indefinite)
                {
                        // Subtracting infinities ⇒ NaN
                    CHECK(maybe(isnan(z)));
                }
                else if (infinite)
                {
                        // a/0 = [-∞,∞]
                    CHECK(z.lower() == -inf);
                    CHECK(z.upper() == inf);
                }
                else
                {
                    CHECK(z.lower() == std::min({ a/c, a/d }));
                    CHECK(z.upper() == std::max({ a/c, a/d }));
                    CHECK(maybe(z == a/c));
                    CHECK(maybe(z == a/yin));
                    CHECK(maybe(z == a/d));
                }
            }
        }
    }
    SECTION("pow()")
    {
        SECTION("non-negative base")
        {
            auto xs = std::array{ 0., 0.4, 0.7, 1., 1.4, 2., 2.3 };
            auto ys = std::array{ -2., -1.6, -1.3, -1., -0.7, -0.5, -1./3, 0., 1./3, 0.5, 0.7, 1., 1.3, 1.6, 2. };
            for (gsl::index i = 0; i < gsl::ssize(xs); ++i)
            {
                auto a = xs[i];
                for (gsl::index j = i; j < gsl::ssize(xs); ++j)
                {
                    auto b = xs[j];
                    auto x = interval{ a, b };
                    CAPTURE(x);
                    for (gsl::index m = 0; m < gsl::ssize(ys); ++m)
                    {
                        auto c = ys[m];
                        for (gsl::index n = m; n < gsl::ssize(ys); ++n)
                        {
                            auto d = ys[n];
                            auto y = interval{ c, d };
                            CAPTURE(y);

                            auto z = pow(x, y);
                            CAPTURE(z);
                            auto v1 = std::pow(a, c);
                            auto v2 = std::pow(a, d);
                            auto v3 = std::pow(b, c);
                            auto v4 = std::pow(b, d);
                            CHECK(z.lower() == Catch::Approx(std::min({ v1, v2, v3, v4 })));
                            CHECK(z.upper() == Catch::Approx(std::max({ v1, v2, v3, v4 })));
                        }

                        auto v = pow(x, c);
                        CAPTURE(v);
                        auto v1 = std::pow(a, c);
                        auto v2 = std::pow(b, c);
                        CHECK(v.lower() == Catch::Approx(std::min({ v1, v2 })));
                        CHECK(v.upper() == Catch::Approx(std::max({ v1, v2 })));
                    }
                }
                for (gsl::index m = 0; m < gsl::ssize(ys); ++m)
                {
                    auto c = ys[m];
                    for (gsl::index n = m; n < gsl::ssize(ys); ++n)
                    {
                        auto d = ys[n];
                        auto y = interval{ c, d };
                        CAPTURE(y);

                        auto u = pow(a, y);
                        CAPTURE(u);
                        auto v1 = std::pow(a, c);
                        auto v2 = std::pow(a, d);
                        CHECK(u.lower() == Catch::Approx(std::min({ v1, v2 })));
                        CHECK(u.upper() == Catch::Approx(std::max({ v1, v2 })));
                    }
                }
            }
        }
        SECTION("indefinite")
        {
            CHECK(maybe(isnan(pow(interval{ -1., 0. }, -1.5))));
            CHECK(maybe(isnan(pow(interval{ -1., 0. }, 1.5))));
            CHECK(maybe(isnan(pow(interval{ -1., 0. }, interval{ -1., 0. }))));
            CHECK(maybe(isnan(pow(interval{ -1., 0. }, interval{ 0., 1. }))));
        }
        SECTION("negative base")
        {
            auto xs = std::array{ -2.3, -2., -1.4, -1., -0.7, -0.4, 0., 0.4, 0.7, 1., 1.4, 2., 2.3 };
            auto ys = std::array{ -3., -2., -1., 0., 1., 2., 3. };
            for (gsl::index i = 0; i < gsl::ssize(xs); ++i)
            {
                auto a = xs[i];
                for (gsl::index j = i; j < gsl::ssize(xs); ++j)
                {
                    auto b = xs[j];
                    auto x = interval{ a, b };
                    CAPTURE(x);
                    for (auto c : ys)
                    {
                        auto y = interval{ c, c };
                        CAPTURE(y);

                        auto z = pow(x, y);
                        CAPTURE(z);

                        if (maybe(x == 0) && definitely(y < 0))
                        {
                            auto signs = set<intervals::sign>{ };
                            if (gsl::narrow_cast<int>(c) % 2 == 0)
                            {
                                signs.assign(intervals::positive_sign);
                            }
                            else
                            {
                                signs.assign(sgn(x));
                            }
                            if (maybe(signs == intervals::positive_sign))
                            {
                                CHECK(z.upper() == inf);
                            }
                            if (maybe(signs == intervals::negative_sign))
                            {
                                CHECK(z.lower() == -inf);
                            }
                        }
                        else
                        {
                            auto v1 = std::pow(a, c);
                            auto v2 = std::pow(b, c);
                            auto vmin = std::min({ v1, v2 });
                            auto vmax = std::max({ v1, v2 });
                            if (c != 0 && gsl::narrow_cast<int>(c) % 2 == 0 && x.contains(0.))
                            {
                                vmin = std::min(vmin, 0.);
                            }
                            CHECK(z.lower() == Catch::Approx(vmin));
                            CHECK(z.upper() == Catch::Approx(vmax));

                            auto v = pow(x, c);
                            CAPTURE(v);
                            CHECK(v.lower() == Catch::Approx(vmin));
                            CHECK(v.upper() == Catch::Approx(vmax));
                        }
                    }
                }
                for (auto c : ys)
                {
                    auto y = interval{ c, c };
                    CAPTURE(y);

                    auto u = pow(a, y);
                    CAPTURE(u);
                    auto v1 = std::pow(a, c);
                    CHECK(u.lower() == Catch::Approx(v1));
                    CHECK(u.upper() == Catch::Approx(v1));
                }
            }
        }
    }
    SECTION("trigonometric functions")
    {
        using std::numbers::pi;

        SECTION("sine, cosine, tangent")
        {
            auto angles = std::array{ -7., -2*pi, -6., -5., -3./2*pi, -4., -pi, -3., -2., -pi/2, -1.55, -1., -pi/4, -0.5, 0., 0.5, pi/4, 1., 1.55, pi/2, 2., 3., pi, 4., 3./2*pi, 5., 6., 2*pi, 7. };
            auto sin_values = std::vector<double>{ };
            auto cos_values = std::vector<double>{ };
            auto tan_values = std::vector<double>{ };
            for (gsl::index i = 0; i < gsl::ssize(angles); ++i)
            {
                double a = angles[i];
                sin_values.clear();
                cos_values.clear();
                tan_values.clear();
                for (gsl::index j = i; j < gsl::ssize(angles); ++j)
                {
                    double b = angles[j];
                    sin_values.push_back(std::sin(b));
                    cos_values.push_back(std::cos(b));
                    tan_values.push_back(std::tan(b));
                    std::sort(sin_values.begin(), sin_values.end());
                    std::sort(cos_values.begin(), cos_values.end());
                    std::sort(tan_values.begin(), tan_values.end());
                    auto x = interval{ a, b };
                    CAPTURE(x);
                    auto sin_x = sin(x);
                    CAPTURE(sin_x);
                    CHECK(sin_x.lower() == Catch::Approx(sin_values.front()).margin(1.e-8));
                    CHECK(sin_x.upper() == Catch::Approx(sin_values.back()).margin(1.e-8));
                    auto cos_x = cos(x);
                    CAPTURE(cos_x);
                    CHECK(cos_x.lower() == Catch::Approx(cos_values.front()).margin(1.e-8));
                    CHECK(cos_x.upper() == Catch::Approx(cos_values.back()).margin(1.e-8));
                    if (a >= -1.55 && b <= 1.55)
                    {
                        auto tan_x = tan(x);
                        CAPTURE(tan_x);
                        CHECK(tan_x.lower() == Catch::Approx(tan_values.front()).margin(1.e-8));
                        CHECK(tan_x.upper() == Catch::Approx(tan_values.back()).margin(1.e-8));
                    }
                }
            }
        }
        SECTION("arc sine, arc cosine, arc tangent")
        {
            auto ratios = std::array{ -inf, -2., -1., -std::sqrt(1./2), -0.5, -std::sqrt(1./4), 0., std::sqrt(1./4), 0.5, std::sqrt(1./2), 1., 2., inf };
            for (gsl::index i = 0; i < gsl::ssize(ratios); ++i)
            {
                double a = ratios[i];
                for (gsl::index j = i; j < gsl::ssize(ratios); ++j)
                {
                    double b = ratios[j];
                    auto x = interval{ a, b };
                    CAPTURE(x);
                    if (a >= -1. && b <= 1.)
                    {
                        auto asin_x = asin(x);
                        CHECK(asin_x.lower() == std::asin(a));
                        CHECK(asin_x.upper() == std::asin(b));
                        auto acos_x = acos(x);
                        CHECK(acos_x.lower() == std::acos(b));
                        CHECK(acos_x.upper() == std::acos(a));
                    }
                    else
                    {
                        CHECK(maybe(isnan(asin(x))));
                        CHECK(maybe(isnan(acos(x))));
                    }
                    auto atan_x = atan(x);
                    CHECK(atan_x.lower() == std::atan(a));
                    CHECK(atan_x.upper() == std::atan(b));
                }
            }
        }
        SECTION("atan2()")
        {
            auto [x, y, indefinite] = GENERATE(
                // singular intervals
                std::tuple{ interval{ -3., -3. }, interval{ -4., -4. }, false },
                std::tuple{ interval{  2.,  2. }, interval{ -4., -4. }, false },
                std::tuple{ interval{  2.,  2. }, interval{  1.,  1. }, false },
                std::tuple{ interval{ -3., -3. }, interval{  1.,  1. }, false },
                // intervals not covering  x ≤ 0 ∧ y = 0
                std::tuple{ interval{ -3., -1. }, interval{ -4., -2. }, false },
                std::tuple{ interval{ -3.,  0. }, interval{ -4., -2. }, false },
                std::tuple{ interval{ -3.,  2. }, interval{ -4., -2. }, false },
                std::tuple{ interval{  0.,  2. }, interval{ -4., -2. }, false },
                std::tuple{ interval{  1.,  4. }, interval{ -4.,  0. }, false },
                std::tuple{ interval{  1.,  4. }, interval{ -4.,  2. }, false },
                std::tuple{ interval{  1.,  4. }, interval{  0.,  2. }, false },
                std::tuple{ interval{  1.,  4. }, interval{  1.,  2. }, false },
                std::tuple{ interval{  0.,  4. }, interval{  1.,  2. }, false },
                std::tuple{ interval{ -3.,  2. }, interval{  1.,  2. }, false },
                std::tuple{ interval{ -3.,  0. }, interval{  1.,  2. }, false },
                std::tuple{ interval{ -3., -1. }, interval{  1.,  2. }, false },
                // intervals covering  x ≤ 0 ∧ y = 0
                std::tuple{ interval{ -3.,  0. }, interval{  0.,  0. }, true  },
                std::tuple{ interval{  0.,  0. }, interval{  0.,  2. }, true  },
                std::tuple{ interval{  0.,  0. }, interval{  0.,  0. }, true  },
                std::tuple{ interval{ -3.,  0. }, interval{  0.,  2. }, true  },
                std::tuple{ interval{ -3.,  0. }, interval{ -1.,  2. }, true  },
                std::tuple{ interval{ -3.,  0. }, interval{ -1.,  0. }, true  },
                std::tuple{ interval{  0.,  1. }, interval{  0.,  2. }, true  },
                std::tuple{ interval{  0.,  1. }, interval{ -1.,  2. }, true  },
                std::tuple{ interval{  0.,  1. }, interval{ -1.,  0. }, true  },
                std::tuple{ interval{ -3.,  1. }, interval{  0.,  2. }, true  },
                std::tuple{ interval{ -3.,  1. }, interval{ -1.,  2. }, true  },
                std::tuple{ interval{ -3.,  1. }, interval{ -1.,  0. }, true  }
            );
            CAPTURE(x);
            CAPTURE(y);
            auto z = atan2(y, x);
            CAPTURE(z);
            if (indefinite)
            {
                CHECK(maybe(isnan(z)));
            }
            else
            {
                CHECK(definitely(!isnan(z)));
                auto v1 = std::atan2(y.lower(), x.lower());
                auto v2 = std::atan2(y.lower(), x.upper());
                auto v3 = std::atan2(y.upper(), x.lower());
                auto v4 = std::atan2(y.upper(), x.upper());
                CHECK(z.lower() == Catch::Approx(std::min({ v1, v2, v3, v4 })).margin(1.e-8));
                CHECK(z.upper() == Catch::Approx(std::max({ v1, v2, v3, v4 })).margin(1.e-8));

                auto u = atan2(y, x.lower());
                CAPTURE(u);
                CHECK(u.lower() == Catch::Approx(std::min({ v1, v3 })).margin(1.e-8));
                CHECK(u.upper() == Catch::Approx(std::max({ v1, v3 })).margin(1.e-8));

                auto v = atan2(y.lower(), x);
                CAPTURE(v);
                CHECK(v.lower() == Catch::Approx(std::min({ v1, v2 })).margin(1.e-8));
                CHECK(v.upper() == Catch::Approx(std::max({ v1, v2 })).margin(1.e-8));
            }
        }
    }
    SECTION("floor(), ceil(), frac()")
    {
        // TODO: add tests
    }
    SECTION("abs()")
    {
        // TODO: add tests
    }
    SECTION("if_else()")
    {
        using intervals::if_else;

        CHECK(if_else(true, 1, 2) == 1);
        CHECK(if_else(false, 1, 2) == 2);
        CHECK(if_else(set<bool>{ false       }, 1.,            2.           ).matches(2.));
        CHECK(if_else(set<bool>{ false, true }, 1.,            2.           ).matches(interval(1., 2.)));
        CHECK(if_else(set<bool>{        true }, 1.,            2.           ).matches(1.));
        CHECK(if_else(set<bool>{ false       }, interval(1., 2.), 3.           ).matches(3.));
        CHECK(if_else(set<bool>{ false, true }, interval(1., 2.), 3.           ).matches(interval(1., 3.)));
        CHECK(if_else(set<bool>{        true }, interval(1., 2.), 3.           ).matches(interval(1., 2.)));
        CHECK(if_else(set<bool>{ false       }, 1.,            interval(3., 4.)).matches(interval(3., 4.)));
        CHECK(if_else(set<bool>{ false, true }, 1.,            interval(3., 4.)).matches(interval(1., 4.)));
        CHECK(if_else(set<bool>{        true }, 1.,            interval(3., 4.)).matches(1.));
        CHECK(if_else(set<bool>{ false       }, interval(1., 2.), interval(3., 4.)).matches(interval(3., 4.)));
        CHECK(if_else(set<bool>{ false, true }, interval(1., 2.), interval(3., 4.)).matches(interval(1., 4.)));
        CHECK(if_else(set<bool>{        true }, interval(1., 2.), interval(3., 4.)).matches(interval(1., 2.)));
    }
    SECTION("constrain()")
    {
        using intervals::constrain;
        using intervals::maybe;

        SECTION("scalar")
        {
            auto a = GENERATE(-inf, -1., 0., 1.);
            auto b = 2.;
            auto x = GENERATE(-2., -1., 0., 1., 2., 3.);
            CAPTURE(x);
            CAPTURE(a);
            CAPTURE(b);
    
            if (auto cond = (x >= a) & (x <= b); maybe(cond))
            {
                CHECK(constrain(x, cond) == x);
            }
            else
            {
                CHECK(constrain(x, !cond) == x);
                CHECK_THROWS_AS(constrain(x, cond), gsl::fail_fast);
            }
        }
        SECTION("interval")
        {
            SECTION("x < y, x <= y")
            {
                auto [x, y, ltc, leqc, xc, yc] = GENERATE(
                    // plain scalars
                    std::tuple{ interval{ -2. }, interval{ 1. }, set{ true  }, set{ true  }, interval{ -2. }, interval{ 1. } },
                    std::tuple{ interval{  1. }, interval{ 1. }, set{ false }, set{ true  }, interval{  1. }, interval{ 1. } },
                    std::tuple{ interval{  3. }, interval{ 1. }, set{ false }, set{ false }, interval{  0. }, interval{ 0. } },

                    // mixing scalars and intervals
                    std::tuple{ interval{ -2., -1. }, interval{ 1. }, set{        true }, set{        true }, interval{ -2., -1. }, interval{ 1. }     },
                    std::tuple{ interval{ -2.,  1. }, interval{ 1. }, set{ false, true }, set{        true }, interval{ -2.,  1. }, interval{ 1. }     },
                    std::tuple{ interval{ -2.,  3. }, interval{ 1. }, set{ false, true }, set{ false, true }, interval{ -2.,  1. }, interval{ 1. }     },
                    std::tuple{ interval{  1.,  3. }, interval{ 1. }, set{ false       }, set{ false, true }, interval{ 1. },       interval{ 1. }     },
                    std::tuple{ interval{  2.,  3. }, interval{ 1. }, set{ false       }, set{ false       }, interval{ 0. },       interval{ 0. }     },
                    std::tuple{ interval{ 1. }, interval{  2.,  3. }, set{        true }, set{        true }, interval{ 1. },       interval{ 2., 3. } },
                    std::tuple{ interval{ 1. }, interval{  1.,  3. }, set{ false, true }, set{        true }, interval{ 1. },       interval{ 1., 3. } },
                    std::tuple{ interval{ 1. }, interval{ -2.,  3. }, set{ false, true }, set{ false, true }, interval{ 1. },       interval{ 1., 3. } },
                    std::tuple{ interval{ 1. }, interval{ -2.,  1. }, set{ false       }, set{ false, true }, interval{ 1. },       interval{ 1. }     },
                    std::tuple{ interval{ 1. }, interval{ -2., -1. }, set{ false       }, set{ false       }, interval{ 0. },       interval{ 0. }     },

                    // intervals
                    std::tuple{ interval{ -2., -1. }, interval{ 1., 3. }, set{        true }, set {        true }, interval{ -2., -1. }, interval{ 1., 3. } },
                    std::tuple{ interval{ -2.,  1. }, interval{ 1., 3. }, set{ false, true }, set {        true }, interval{ -2.,  1. }, interval{ 1., 3. } },
                    std::tuple{ interval{ -2.,  2. }, interval{ 1., 3. }, set{ false, true }, set { false, true }, interval{ -2.,  2. }, interval{ 1., 3. } },
                    std::tuple{ interval{ -2.,  3. }, interval{ 1., 3. }, set{ false, true }, set { false, true }, interval{ -2.,  3. }, interval{ 1., 3. } },
                    std::tuple{ interval{ -2.,  5. }, interval{ 1., 3. }, set{ false, true }, set { false, true }, interval{ -2.,  3. }, interval{ 1., 3. } },
                    std::tuple{ interval{  1.,  2. }, interval{ 1., 3. }, set{ false, true }, set { false, true }, interval{  1.,  2. }, interval{ 1., 3. } },
                    std::tuple{ interval{  1.,  3. }, interval{ 1., 3. }, set{ false, true }, set { false, true }, interval{  1.,  3. }, interval{ 1., 3. } },
                    std::tuple{ interval{  1.,  5. }, interval{ 1., 3. }, set{ false, true }, set { false, true }, interval{  1.,  3. }, interval{ 1., 3. } },
                    std::tuple{ interval{  2.,  3. }, interval{ 1., 3. }, set{ false, true }, set { false, true }, interval{  2.,  3. }, interval{ 2., 3. } },
                    std::tuple{ interval{  2.,  5. }, interval{ 1., 3. }, set{ false, true }, set { false, true }, interval{  2.,  3. }, interval{ 2., 3. } },
                    std::tuple{ interval{  3.,  5. }, interval{ 1., 3. }, set{ false       }, set { false, true }, interval{  3. },      interval{ 3. }     },
                    std::tuple{ interval{  4.,  5. }, interval{ 1., 3. }, set{ false       }, set { false       }, interval{  0. },      interval{ 0. }     }
                );
                CAPTURE(x);
                CAPTURE(y);
                CAPTURE(xc);
                CAPTURE(yc);

                {
                    auto cc = x <= y;
                    CAPTURE(leqc);
                    CAPTURE(cc);
                    CHECK(cc.matches(leqc));
                    if (cc.contains(true))
                    {
                        auto xcc = constrain(x, cc);
                        auto ycc = constrain(y, cc);
                        CAPTURE(xcc);
                        CAPTURE(ycc);
                        CHECK(xcc.matches(xc));
                        CHECK(ycc.matches(yc));
                    }
                    else
                    {
                        CHECK_THROWS_AS(constrain(x, cc), gsl::fail_fast);
                        CHECK_THROWS_AS(constrain(y, cc), gsl::fail_fast);
                    }
                }
                {
                    auto cc = y >= x;
                    CAPTURE(leqc);
                    CAPTURE(cc);
                    CHECK(cc.matches(leqc));
                    if (cc.contains(true))
                    {
                        auto xcc = constrain(x, cc);
                        auto ycc = constrain(y, cc);
                        CAPTURE(xcc);
                        CAPTURE(ycc);
                        CHECK(xcc.matches(xc));
                        CHECK(ycc.matches(yc));
                    }
                    else
                    {
                        CHECK_THROWS_AS(constrain(x, cc), gsl::fail_fast);
                        CHECK_THROWS_AS(constrain(y, cc), gsl::fail_fast);
                    }
                }
                {
                    auto cc = x < y;
                    CAPTURE(ltc);
                    CAPTURE(cc);
                    CHECK(cc.matches(ltc));
                    if (cc.contains(true))
                    {
                        auto xcc = constrain(x, cc);
                        auto ycc = constrain(y, cc);
                        CAPTURE(xcc);
                        CAPTURE(ycc);
                        CHECK(xcc.matches(xc));
                        CHECK(ycc.matches(yc));
                    }
                    else if (!leqc.contains(true))
                    {
                        CHECK_THROWS_AS(constrain(x, cc), gsl::fail_fast);
                        CHECK_THROWS_AS(constrain(y, cc), gsl::fail_fast);
                    }
                }
                {
                    auto cc = y > x;
                    CAPTURE(ltc);
                    CAPTURE(cc);
                    CHECK(cc.matches(ltc));
                    if (cc.contains(true))
                    {
                        auto xcc = constrain(x, cc);
                        auto ycc = constrain(y, cc);
                        CAPTURE(xcc);
                        CAPTURE(ycc);
                        CHECK(xcc.matches(xc));
                        CHECK(ycc.matches(yc));
                    }
                    else if (!leqc.contains(true))
                    {
                        CHECK_THROWS_AS(constrain(x, cc), gsl::fail_fast);
                        CHECK_THROWS_AS(constrain(y, cc), gsl::fail_fast);
                    }
                }
            }
            SECTION("x == y, x != y")
            {
                auto [x, y, c, xyc] = GENERATE(
                    // plain scalars
                    std::tuple{ interval{ -2. }, interval{ 1. }, set{ false }, interval{ 0. } },
                    std::tuple{ interval{  1. }, interval{ 1. }, set{ true  }, interval{ 1. } },
                    std::tuple{ interval{  3. }, interval{ 1. }, set{ false }, interval{ 0. } },
                    
                    // mixing scalars and intervals
                    std::tuple{ interval{ -2., -1. }, interval{ 1. }, set{ false       }, interval{ 0. } },
                    std::tuple{ interval{ -2.,  1. }, interval{ 1. }, set{ false, true }, interval{ 1. } },
                    std::tuple{ interval{ -2.,  3. }, interval{ 1. }, set{ false, true }, interval{ 1. } },
                    std::tuple{ interval{  1.,  3. }, interval{ 1. }, set{ false, true }, interval{ 1. } },
                    std::tuple{ interval{  2.,  3. }, interval{ 1. }, set{ false       }, interval{ 0. } },
                    
                    // intervals
                    std::tuple{ interval{ -2., -1. }, interval{ 1., 3. }, set { false       }, interval{ 0. }     },
                    std::tuple{ interval{ -2.,  1. }, interval{ 1., 3. }, set { false, true }, interval{ 1. }     },
                    std::tuple{ interval{ -2.,  2. }, interval{ 1., 3. }, set { false, true }, interval{ 1., 2. } },
                    std::tuple{ interval{ -2.,  3. }, interval{ 1., 3. }, set { false, true }, interval{ 1., 3. } },
                    std::tuple{ interval{ -2.,  5. }, interval{ 1., 3. }, set { false, true }, interval{ 1., 3. } },
                    std::tuple{ interval{  1.,  2. }, interval{ 1., 3. }, set { false, true }, interval{ 1., 2. } },
                    std::tuple{ interval{  1.,  3. }, interval{ 1., 3. }, set { false, true }, interval{ 1., 3. } },
                    std::tuple{ interval{  1.,  5. }, interval{ 1., 3. }, set { false, true }, interval{ 1., 3. } },
                    std::tuple{ interval{  2.,  3. }, interval{ 1., 3. }, set { false, true }, interval{ 2., 3. } },
                    std::tuple{ interval{  2.,  5. }, interval{ 1., 3. }, set { false, true }, interval{ 2., 3. } },
                    std::tuple{ interval{  3.,  5. }, interval{ 1., 3. }, set { false, true }, interval{ 3. }     },
                    std::tuple{ interval{  4.,  5. }, interval{ 1., 3. }, set { false       }, interval{ 0. }     }
                );
                CAPTURE(x);
                CAPTURE(y);
                CAPTURE(c);
                CAPTURE(xyc);

                {
                    auto cc = x == y;
                    CAPTURE(cc);
                    CHECK(cc.matches(c));
                    if (cc.contains(true))
                    {
                        auto xcc = constrain(x, cc);
                        auto ycc = constrain(y, cc);
                        CAPTURE(xcc);
                        CAPTURE(ycc);
                        CHECK(xcc.matches(xyc));
                        CHECK(ycc.matches(xyc));
                    }
                    else
                    {
                        CHECK_THROWS_AS(constrain(x, cc), gsl::fail_fast);
                        CHECK_THROWS_AS(constrain(y, cc), gsl::fail_fast);
                    }
                }
                {
                    auto cc = x != y;
                    CAPTURE(cc);
                    CHECK(cc.matches(!c));
                    if (cc.contains(false))
                    {
                        auto xcc = constrain(x, !cc);
                        auto ycc = constrain(y, !cc);
                        CAPTURE(xcc);
                        CAPTURE(ycc);
                        CHECK(xcc.matches(xyc));
                        CHECK(ycc.matches(xyc));
                    }
                    else
                    {
                        CHECK_THROWS_AS(constrain(x, !cc), gsl::fail_fast);
                        CHECK_THROWS_AS(constrain(y, !cc), gsl::fail_fast);
                    }
                }

            }
            SECTION("& and |")
            {
                auto xlo = GENERATE(-2., 1., 2., 3., 4., 5., 6., 7.);
                auto xhi = GENERATE(-1., 1., 2., 3., 4., 5., 6., 7., 8.);
                SECTION("(x >= a) & (x <= b)")
                {
                    if (xhi >= xlo)
                    {
                        auto [a, b] = GENERATE(
                            std::tuple{ interval{ 1. },     interval{ 1. }     },
                            std::tuple{ interval{ 1. },     interval{ 4. }     },
                            std::tuple{ interval{ 1. },     interval{ 4., 6. } },
                            std::tuple{ interval{ 4. },     interval{ 4., 6. } },
                            std::tuple{ interval{ 1., 3. }, interval{ 4. }     },
                            std::tuple{ interval{ 1., 3. }, interval{ 4., 6. } },
                            std::tuple{ interval{ 1., 4. }, interval{ 4. }     },
                            std::tuple{ interval{ 1., 4. }, interval{ 4., 6. } }
                        );
                        auto c = set<bool>{ };
                        if (xhi >= a.lower() && xlo <= b.upper())
                        {
                            c.assign(true);
                        }
                        if (xlo < a.upper() || xhi > b.lower())
                        {
                            c.assign(false);
                        }
                        auto x = interval{ xlo, xhi };
                        CAPTURE(x);
                        CAPTURE(a);
                        CAPTURE(b);
                        CAPTURE(c);
                        {
                            auto cc = (x >= a) & (x <= b);
                            CAPTURE(cc);
                            CHECK(cc.matches(c));
                            if (cc.contains(true))
                            {
                                auto xc = constrain(x, cc);
                                CAPTURE(xc);
                                CHECK(xc.matches(interval{ std::max(xlo, a.lower()), std::min(xhi, b.upper()) }));
                            }
                            else
                            {
                                CHECK_THROWS_AS(constrain(x, cc), gsl::fail_fast);
                            }
                        }
                        {
                            auto cc = (a <= x) & (b >= x);
                            CAPTURE(cc);
                            CHECK(cc.matches(c));
                            if (cc.contains(true))
                            {
                                auto xc = constrain(x, cc);
                                CAPTURE(xc);
                                CHECK(xc.matches(interval{ std::max(xlo, a.lower()), std::min(xhi, b.upper()) }));
                            }
                            else
                            {
                                CHECK_THROWS_AS(constrain(x, cc), gsl::fail_fast);
                            }
                        }
                        {
                            auto cc = (x >= interval(a)) & (x <= interval(b));
                            CAPTURE(cc);
                            CHECK(cc.matches(c));
                            if (cc.contains(true))
                            {
                                auto xc = constrain(x, cc);
                                CAPTURE(xc);
                                CHECK(xc.matches(interval{ std::max(xlo, a.lower()), std::min(xhi, b.upper()) }));
                            }
                            else
                            {
                                CHECK_THROWS_AS(constrain(x, cc), gsl::fail_fast);
                            }
                        }
                        {
                            auto cc = (interval(a) <= x) & (interval(b) >= x);
                            CAPTURE(cc);
                            CHECK(cc.matches(c));
                            if (cc.contains(true))
                            {
                                auto xc = constrain(x, cc);
                                CAPTURE(xc);
                                CHECK(xc.matches(interval{ std::max(xlo, a.lower()), std::min(xhi, b.upper()) }));
                            }
                            else
                            {
                                CHECK_THROWS_AS(constrain(x, cc), gsl::fail_fast);
                            }
                        }
                        {
                            auto cc = (x < a) | (x > b);
                            CAPTURE(cc);
                            CHECK(cc.matches(!c));
                            if (cc.contains(false))
                            {
                                auto xc = constrain(x, !cc);
                                CAPTURE(xc);
                                CHECK(xc.matches(interval{ std::max(xlo, a.lower()), std::min(xhi, b.upper()) }));
                            }
                            else
                            {
                                CHECK_THROWS_AS(constrain(x, !cc), gsl::fail_fast);
                            }
                        }
                        {
                            auto cc = (a > x) | (b < x);
                            CAPTURE(cc);
                            CHECK(cc.matches(!c));
                            if (cc.contains(false))
                            {
                                auto xc = constrain(x, !cc);
                                CAPTURE(xc);
                                CHECK(xc.matches(interval{ std::max(xlo, a.lower()), std::min(xhi, b.upper()) }));
                            }
                            else
                            {
                                CHECK_THROWS_AS(constrain(x, !cc), gsl::fail_fast);
                            }
                        }
                        {
                            auto cc = (x < interval(a)) | (x > interval(b));
                            CAPTURE(cc);
                            CHECK(cc.matches(!c));
                            if (cc.contains(false))
                            {
                                auto xc = constrain(x, !cc);
                                CAPTURE(xc);
                                CHECK(xc.matches(interval{ std::max(xlo, a.lower()), std::min(xhi, b.upper()) }));
                            }
                            else
                            {
                                CHECK_THROWS_AS(constrain(x, !cc), gsl::fail_fast);
                            }
                        }
                        {
                            auto cc = (interval(a) > x) | (interval(b) < x);
                            CAPTURE(cc);
                            CHECK(cc.matches(!c));
                            if (cc.contains(false))
                            {
                                auto xc = constrain(x, !cc);
                                CAPTURE(xc);
                                CHECK(xc.matches(interval{ std::max(xlo, a.lower()), std::min(xhi, b.upper()) }));
                            }
                            else
                            {
                                CHECK_THROWS_AS(constrain(x, !cc), gsl::fail_fast);
                            }
                        }
                    }
                }
                SECTION("(x >= a) | (x >= b)")
                {
                    if (xhi >= xlo)
                    {
                        auto a = GENERATE(interval{ 1. }, interval{ 1., 2. }, interval{ 1., 4. }, interval{ 2., 4. }, interval{ 4. });
                        auto b = GENERATE(interval{ 2. }, interval{ 2., 3. }, interval{ 4., 6. } );
                        auto c = set<bool>{ };
                        if (xhi >= std::min(a.lower(), b.lower()))
                        {
                            c.assign(true);
                        }
                        if (xlo < std::min(a.upper(), b.upper()))
                        {
                            c.assign(false);
                        }
                        auto x = interval{ xlo, xhi };
                        CAPTURE(x);
                        CAPTURE(a);
                        CAPTURE(b);
                        CAPTURE(c);
                        {
                            auto cc = (x >= a) | (x >= b);
                            CAPTURE(cc);
                            CHECK(cc.matches(c));
                            if (cc.contains(true))
                            {
                                auto xc = constrain(x, cc);
                                CAPTURE(xc);
                                CHECK(xc.matches(interval{ std::max(xlo, std::min(a.lower(), b.lower())), xhi }));
                            }
                            else
                            {
                                CHECK_THROWS_AS(constrain(x, cc), gsl::fail_fast);
                            }
                        }
                    }
                }
            }
        }
        SECTION("constraint not applied")
        {
            auto x = interval{ 1. };
            auto y = interval{ 2. };
            auto a = interval{ 0. };
            auto cx = x >= a;
            auto cxp1 = x + 1 >= a;
            auto cy = y >= a;
            CHECK_NOTHROW(constrain(x, cx));
            CHECK_THROWS_AS(constrain(x, cxp1), gsl::fail_fast);
            CHECK_THROWS_AS(constrain(x, cy), gsl::fail_fast);
        }
    }
    SECTION("example: max()")
    {
        auto maxG0 = []<typename T>(T x, T y)
        {
            using intervals::maybe;
            using intervals::maybe_not;
            using intervals::branch_value;
            
            auto result = T{ };
            if (maybe(x >= y))
            {
                branch_value(result) = x;
            }
            if (maybe_not(x >= y))
            {
                branch_value(result) = y;
            }
            return result;
        };
        auto maxG = []<typename T>(T x, T y)
        {
            using intervals::maybe;
            using intervals::maybe_not;
            using intervals::constrain;
            using intervals::branch_value;
            
            auto result = T{ };
            auto cond = x >= y;
            if (maybe(cond))
            {
                branch_value(result) = constrain(x, cond);
            }
            if (maybe_not(cond))
            {
                branch_value(result) = constrain(y, !cond);
            }
            return result;
        };

        using intervals::interval;

        {
            auto m = maxG0(interval{ 0., 2. }, interval{ 3., 4. });
            auto mE = interval{ 3., 4. };
            CAPTURE(m);
            CAPTURE(mE);
            CHECK(m.matches(mE));
        }
        {
            auto m = maxG0(interval{ 3., 4. }, interval{ 0., 2. });
            auto mE = interval{ 3., 4. };
            CAPTURE(m);
            CAPTURE(mE);
            CHECK(m.matches(mE));
        }
        {
            auto m = maxG0(interval{ 0., 2. }, interval{ 1., 4. });
            auto mE = interval{ 0., 4. };
            CAPTURE(m);
            CAPTURE(mE);
            CHECK(m.matches(mE));
        }
        {
            auto m = maxG0(interval{ 0., 4. }, interval{ 1., 3. });
            auto mE = interval{ 0., 4. };
            CAPTURE(m);
            CAPTURE(mE);
            CHECK(m.matches(mE));
        }

        {
            auto m = maxG(interval{ 0., 2. }, interval{ 3., 4. });
            auto mE = interval{ 3., 4. };
            CAPTURE(m);
            CAPTURE(mE);
            CHECK(m.matches(mE));
        }
        {
            auto m = maxG(interval{ 3., 4. }, interval{ 0., 2. });
            auto mE = interval{ 3., 4. };
            CAPTURE(m);
            CAPTURE(mE);
            CHECK(m.matches(mE));
        }
        {
            auto m = maxG(interval{ 0., 2. }, interval{ 1., 4. });
            auto mE = interval{ 1., 4. };
            CAPTURE(m);
            CAPTURE(mE);
            CHECK(m.matches(mE));
        }
        {
            auto m = maxG(interval{ 0., 4. }, interval{ 1., 3. });
            auto mE = interval{ 1., 4. };
            CAPTURE(m);
            CAPTURE(mE);
            CHECK(m.matches(mE));
        }
    }
}


} // anonymous namespace
