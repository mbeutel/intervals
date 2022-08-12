
#include <array>
#include <tuple>
#include <vector>
#include <cstddef>
#include <algorithm>  // for sort()

#include <gsl-lite/gsl-lite.hpp>  // for fail_fast, type_identity<>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rpmc/tools/math.hpp>           // for log(), sqrt()
#include <rpmc/tools/bound-algebra.hpp>


namespace {

namespace gsl = ::gsl_lite;


enum Color { red = 2, green = 1, blue = 4 };
constexpr auto
reflect(gsl::type_identity<Color>)
{
    return std::array{ red, green, blue };
}


TEST_CASE("Set<>", "algebra with bounds")
{
    using rpmc::Set;

    SECTION("fail on uninitialized read")
    {
        auto value = GENERATE(false, true);
        CAPTURE(value);
        auto s0 = rpmc::Set<bool>{ };
        auto svalue = rpmc::Set<bool>{ value };

            // `contains()` and `matches()` are supported for uninitialized values.
        CHECK_NOTHROW(s0.contains(value));
        CHECK_NOTHROW(s0.contains(svalue));
        CHECK_NOTHROW(s0.matches(value));
        CHECK_NOTHROW(s0.matches(svalue));

        CHECK_THROWS_AS(s0 == value, gsl::fail_fast);
        CHECK_THROWS_AS(s0 != value, gsl::fail_fast);
        CHECK_THROWS_AS(!s0, gsl::fail_fast);
    }
    SECTION("single-valued sets (bool)")
    {
        auto value = GENERATE(false, true);
        CAPTURE(value);
        auto svalue = Set{ value };
        auto nsvalue = Set{ !value };
        CHECK(svalue.contains(value));
        CHECK(svalue.contains(svalue));
        CHECK_FALSE(svalue.contains(!value));
        CHECK_FALSE(svalue.contains(nsvalue));
        CHECK(svalue.matches(value));
        CHECK(svalue.matches(svalue));
        CHECK_FALSE(svalue.matches(!value));
        CHECK_FALSE(svalue.matches(nsvalue));
        CHECK((svalue == svalue).matches(Set{ true }));
        CHECK((svalue == nsvalue).matches(Set{ false }));
        CHECK((svalue != svalue).matches(Set{ false }));
        CHECK((svalue != nsvalue).matches(Set{ true }));
        CHECK((!svalue).matches(nsvalue));
        CHECK((!nsvalue).matches(svalue));
        CHECK(maybe(svalue) == value);
        CHECK(definitely(svalue) == value);
    }
    SECTION("multi-valued sets (bool)")
    {
        auto value = GENERATE(false, true);
        CAPTURE(value);
        auto svalue = Set{ value };
        auto s1 = Set{ false, true };
        CHECK(s1.contains(s1));
        CHECK(s1.contains(value));
        CHECK(s1.contains(svalue));
        CHECK(s1.matches(s1));
        CHECK_FALSE(s1.matches(value));
        CHECK_FALSE(s1.matches(svalue));
        CHECK((s1 == s1).matches(Set{ true }));
        CHECK((s1 == svalue).matches(s1));
        CHECK((s1 != svalue).matches(s1));
        CHECK((!s1).matches(s1));
        CHECK(maybe(s1));
        CHECK_FALSE(definitely(s1));
    }
    SECTION("assignment (bool)")
    {
        auto value = GENERATE(false, true);
        auto svalue = Set{ value };
        auto nsvalue = Set{ !value };
        auto s = rpmc::Set<bool>{ };
        s.assign(value);
        CHECK(maybe(s == Set{ value }));
        CHECK(definitely(s == Set{ value }));
        CHECK((s == svalue).matches(Set{ true }));
        s.assign(!value);
        CHECK(maybe(s == Set{ true, false }));
        CHECK(definitely(s == Set{ true, false }));
    }
    SECTION("multi-valued sets (enum)")
    {
        auto s0 = Set<Color>{ };
        auto sp1 = Set{ red, green };
        auto sp2 = Set{ red, blue };
        auto sp3 = Set{ green, blue };
        auto s1 = Set{ red, green, blue };
        SECTION("multiple values")
        {
            CHECK_THROWS_AS(s0.value(), gsl::fail_fast);
            CHECK_THROWS_AS(sp1.value(), gsl::fail_fast);
            CHECK_THROWS_AS(sp2.value(), gsl::fail_fast);
            CHECK_THROWS_AS(sp3.value(), gsl::fail_fast);
            CHECK_THROWS_AS(s1.value(), gsl::fail_fast);
            CHECK(s1.contains(s1));
            CHECK(s1.contains(sp1));
            CHECK(s1.contains(sp2));
            CHECK(s1.contains(sp3));
            CHECK(s1.matches(s1));
            CHECK_FALSE(s1.matches(sp1));
            CHECK_FALSE(s1.matches(sp2));
            CHECK_FALSE(s1.matches(sp3));
            CHECK_FALSE(sp1.matches(sp2));
            CHECK_FALSE(sp1.matches(sp3));
            CHECK_FALSE(sp2.matches(sp3));
            CHECK((s1 == s1).matches(Set{ true }));
            CHECK((sp1 == Set{ red }).matches(Set{ false, true }));
            CHECK((sp1 == Set{ blue }).matches(Set{ false }));
            CHECK((sp1 == sp2).matches(Set{ false, true }));
            CHECK((sp1 == sp3).matches(Set{ false, true }));
            CHECK((sp2 == sp3).matches(Set{ false, true }));
            CHECK((s1 == sp1).matches(Set{ false, true }));
            CHECK((s1 == sp2).matches(Set{ false, true }));
            CHECK((s1 == sp3).matches(Set{ false, true }));
        }
        SECTION("single value")
        {
            auto value = GENERATE(red, green, blue);
            CAPTURE(value);
            auto svalue = Set{ value };
            CHECK(s1.contains(value));
            CHECK(s1.contains(svalue));
            CHECK_FALSE(s1.matches(value));
            CHECK_FALSE(s1.matches(svalue));
            CHECK(svalue.value() == value);
        }
    }
    SECTION("Boolean operators")
    {
            // !
        CHECK((!Set{ true        }).matches(Set{ false       }));
        CHECK((!Set{ false       }).matches(Set{ true        }));
        CHECK((!Set{ false, true }).matches(Set{ false, true }));

            // &
        CHECK((Set{ true        } & Set{ true        }).matches(Set{ true        }));
        CHECK((Set{ true        } & Set{ false, true }).matches(Set{ false, true }));
        CHECK((Set{ true        } & Set{ false       }).matches(Set{ false       }));
        CHECK((Set{ false, true } & Set{ true        }).matches(Set{ false, true }));
        CHECK((Set{ false, true } & Set{ false, true }).matches(Set{ false, true }));
        CHECK((Set{ false, true } & Set{ false       }).matches(Set{ false       }));
        CHECK((Set{ false       } & Set{ true        }).matches(Set{ false       }));
        CHECK((Set{ false       } & Set{ false, true }).matches(Set{ false       }));
        CHECK((Set{ false       } & Set{ false       }).matches(Set{ false       }));

            // |
        CHECK((Set{ true        } | Set{ true        }).matches(Set{ true        }));
        CHECK((Set{ true        } | Set{ false, true }).matches(Set{ true        }));
        CHECK((Set{ true        } | Set{ false       }).matches(Set{ true        }));
        CHECK((Set{ false, true } | Set{ true        }).matches(Set{ true        }));
        CHECK((Set{ false, true } | Set{ false, true }).matches(Set{ false, true }));
        CHECK((Set{ false, true } | Set{ false       }).matches(Set{ false, true }));
        CHECK((Set{ false       } | Set{ true        }).matches(Set{ true        }));
        CHECK((Set{ false       } | Set{ false, true }).matches(Set{ false, true }));
        CHECK((Set{ false       } | Set{ false       }).matches(Set{ false       }));

            // ^
        CHECK((Set{ true        } ^ Set{ true        }).matches(Set{ false       }));
        CHECK((Set{ true        } ^ Set{ false, true }).matches(Set{ false, true }));
        CHECK((Set{ true        } ^ Set{ false       }).matches(Set{ true        }));
        CHECK((Set{ false, true } ^ Set{ true        }).matches(Set{ false, true }));
        CHECK((Set{ false, true } ^ Set{ false, true }).matches(Set{ false, true }));
        CHECK((Set{ false, true } ^ Set{ false       }).matches(Set{ false, true }));
        CHECK((Set{ false       } ^ Set{ true        }).matches(Set{ true        }));
        CHECK((Set{ false       } ^ Set{ false, true }).matches(Set{ false, true }));
        CHECK((Set{ false       } ^ Set{ false       }).matches(Set{ false       }));
    }
    SECTION("Boolean predicates")
    {
            // maybe()
        CHECK(rpmc::maybe(true));
        CHECK_FALSE(rpmc::maybe(false));
        CHECK(maybe(Set{ true }));
        CHECK(maybe(Set{ false, true }));
        CHECK_FALSE(maybe(Set{ false }));

            // maybe_not()
        CHECK(rpmc::maybe_not(false));
        CHECK_FALSE(rpmc::maybe_not(true));
        CHECK(maybe_not(Set{ false }));
        CHECK(maybe_not(Set{ false, true }));
        CHECK_FALSE(maybe_not(Set{ true }));

            // definitely()
        CHECK(rpmc::definitely(true));
        CHECK_FALSE(rpmc::definitely(false));
        CHECK(definitely(Set{ true }));
        CHECK_FALSE(definitely(Set{ false, true }));
        CHECK_FALSE(definitely(Set{ false }));

            // definitely_not()
        CHECK(rpmc::definitely_not(false));
        CHECK_FALSE(rpmc::definitely_not(true));
        CHECK(definitely_not(Set{ false }));
        CHECK_FALSE(definitely_not(Set{ false, true }));
        CHECK_FALSE(definitely_not(Set{ true }));
    }
}

TEST_CASE("Bound<>", "algebra with bounds")
{
    using rpmc::Set;
    using rpmc::Bound;
    using rpmc::Interval;

    constexpr double inf = std::numeric_limits<double>::infinity();
    constexpr double nan = std::numeric_limits<double>::quiet_NaN();

    SECTION("value()")
    {
        CHECK(Bound{ 1., 1. }.value() == 1.);
        CHECK_THROWS_AS(Bound<double>{ }.value(), gsl::fail_fast);
        CHECK_THROWS_AS((Bound{ 1., 2. }.value()), gsl::fail_fast);
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
        auto x = rpmc::Bound{ a, b };
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

    // The `Bound<>` type implements the transposition of the  min/max  operator and an unary/binary algebraic function, i.e.
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
    // We use this definition to write property-based tests for the operations of the `Bound<>` type.

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
        auto x = rpmc::Bound{ a, b };
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
                CHECK(square(x).lower() == std::min(rpmc::square(a), rpmc::square(b)));
            }
            CHECK(square(x).upper() == std::max(rpmc::square(a), rpmc::square(b)));
        }
        SECTION("sqrt()")
        {
            if (a >= 0)
            {
                CHECK(sqrt(x).lower() == std::min(rpmc::sqrt(a), rpmc::sqrt(b)));
                CHECK(sqrt(x).upper() == std::max(rpmc::sqrt(a), rpmc::sqrt(b)));
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
                CHECK(log(x).lower() == std::min(rpmc::log(a), rpmc::log(b)));
                CHECK(log(x).upper() == std::max(rpmc::log(a), rpmc::log(b)));
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
            CHECK(maybe(sgn(x) == rpmc::positiveSign) == maybe(x > 0));
            CHECK(maybe(sgn(x) == rpmc::zeroSign) == maybe(x == 0));
            CHECK(maybe(sgn(x) == rpmc::negativeSign) == maybe(x < 0));
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
        auto x = rpmc::Bound{ a, b };
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
        auto y = rpmc::Bound{ c, d };
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
                    auto x = Bound{ a, b };
                    CAPTURE(x);
                    for (gsl::index m = 0; m < gsl::ssize(ys); ++m)
                    {
                        auto c = ys[m];
                        for (gsl::index n = m; n < gsl::ssize(ys); ++n)
                        {
                            auto d = ys[n];
                            auto y = Bound{ c, d };
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
                        auto y = Bound{ c, d };
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
            CHECK(maybe(isnan(pow(Bound{ -1., 0. }, -1.5))));
            CHECK(maybe(isnan(pow(Bound{ -1., 0. }, 1.5))));
            CHECK(maybe(isnan(pow(Bound{ -1., 0. }, Bound{ -1., 0. }))));
            CHECK(maybe(isnan(pow(Bound{ -1., 0. }, Bound{ 0., 1. }))));
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
                    auto x = Bound{ a, b };
                    CAPTURE(x);
                    for (auto c : ys)
                    {
                        auto y = Bound{ c, c };
                        CAPTURE(y);

                        auto z = pow(x, y);
                        CAPTURE(z);

                        if (maybe(x == 0) && definitely(y < 0))
                        {
                            auto signs = Set<rpmc::Sign>{ };
                            if (gsl::narrow_cast<int>(c) % 2 == 0)
                            {
                                signs.assign(rpmc::positiveSign);
                            }
                            else
                            {
                                signs.assign(sgn(x));
                            }
                            if (maybe(signs == rpmc::positiveSign))
                            {
                                CHECK(z.upper() == inf);
                            }
                            if (maybe(signs == rpmc::negativeSign))
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
                    auto y = Bound{ c, c };
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
        using rpmc::pi;

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
                    auto x = Bound{ a, b };
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
                    auto x = Bound{ a, b };
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
                std::tuple{ Bound{ -3., -3. }, Bound{ -4., -4. }, false },
                std::tuple{ Bound{  2.,  2. }, Bound{ -4., -4. }, false },
                std::tuple{ Bound{  2.,  2. }, Bound{  1.,  1. }, false },
                std::tuple{ Bound{ -3., -3. }, Bound{  1.,  1. }, false },
                // intervals not covering  x ≤ 0 ∧ y = 0
                std::tuple{ Bound{ -3., -1. }, Bound{ -4., -2. }, false },
                std::tuple{ Bound{ -3.,  0. }, Bound{ -4., -2. }, false },
                std::tuple{ Bound{ -3.,  2. }, Bound{ -4., -2. }, false },
                std::tuple{ Bound{  0.,  2. }, Bound{ -4., -2. }, false },
                std::tuple{ Bound{  1.,  4. }, Bound{ -4.,  0. }, false },
                std::tuple{ Bound{  1.,  4. }, Bound{ -4.,  2. }, false },
                std::tuple{ Bound{  1.,  4. }, Bound{  0.,  2. }, false },
                std::tuple{ Bound{  1.,  4. }, Bound{  1.,  2. }, false },
                std::tuple{ Bound{  0.,  4. }, Bound{  1.,  2. }, false },
                std::tuple{ Bound{ -3.,  2. }, Bound{  1.,  2. }, false },
                std::tuple{ Bound{ -3.,  0. }, Bound{  1.,  2. }, false },
                std::tuple{ Bound{ -3., -1. }, Bound{  1.,  2. }, false },
                // intervals covering  x ≤ 0 ∧ y = 0
                std::tuple{ Bound{ -3.,  0. }, Bound{  0.,  0. }, true  },
                std::tuple{ Bound{  0.,  0. }, Bound{  0.,  2. }, true  },
                std::tuple{ Bound{  0.,  0. }, Bound{  0.,  0. }, true  },
                std::tuple{ Bound{ -3.,  0. }, Bound{  0.,  2. }, true  },
                std::tuple{ Bound{ -3.,  0. }, Bound{ -1.,  2. }, true  },
                std::tuple{ Bound{ -3.,  0. }, Bound{ -1.,  0. }, true  },
                std::tuple{ Bound{  0.,  1. }, Bound{  0.,  2. }, true  },
                std::tuple{ Bound{  0.,  1. }, Bound{ -1.,  2. }, true  },
                std::tuple{ Bound{  0.,  1. }, Bound{ -1.,  0. }, true  },
                std::tuple{ Bound{ -3.,  1. }, Bound{  0.,  2. }, true  },
                std::tuple{ Bound{ -3.,  1. }, Bound{ -1.,  2. }, true  },
                std::tuple{ Bound{ -3.,  1. }, Bound{ -1.,  0. }, true  }
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
    SECTION("if_else()")
    {
        using rpmc::if_else;

        CHECK(if_else(true, 1, 2) == 1);
        CHECK(if_else(false, 1, 2) == 2);
        CHECK(if_else(Set<bool>{ false       }, 1.,            2.           ).matches(2.));
        CHECK(if_else(Set<bool>{ false, true }, 1.,            2.           ).matches(Bound(1., 2.)));
        CHECK(if_else(Set<bool>{        true }, 1.,            2.           ).matches(1.));
        CHECK(if_else(Set<bool>{ false       }, Bound(1., 2.), 3.           ).matches(3.));
        CHECK(if_else(Set<bool>{ false, true }, Bound(1., 2.), 3.           ).matches(Bound(1., 3.)));
        CHECK(if_else(Set<bool>{        true }, Bound(1., 2.), 3.           ).matches(Bound(1., 2.)));
        CHECK(if_else(Set<bool>{ false       }, 1.,            Bound(3., 4.)).matches(Bound(3., 4.)));
        CHECK(if_else(Set<bool>{ false, true }, 1.,            Bound(3., 4.)).matches(Bound(1., 4.)));
        CHECK(if_else(Set<bool>{        true }, 1.,            Bound(3., 4.)).matches(1.));
        CHECK(if_else(Set<bool>{ false       }, Bound(1., 2.), Bound(3., 4.)).matches(Bound(3., 4.)));
        CHECK(if_else(Set<bool>{ false, true }, Bound(1., 2.), Bound(3., 4.)).matches(Bound(1., 4.)));
        CHECK(if_else(Set<bool>{        true }, Bound(1., 2.), Bound(3., 4.)).matches(Bound(1., 2.)));
    }
    SECTION("constrain()")
    {
        using rpmc::constrain;

        SECTION("scalar")
        {
            auto a = GENERATE(-std::numeric_limits<double>::infinity(), -1., 0., 1.);
            auto b = 2.;
            auto x = GENERATE(-2., -1., 0., 1., 2., 3.);
            CAPTURE(x);
            CAPTURE(a);
            CAPTURE(b);

            if (x >= a && x <= b)
            {
                CHECK(constrain(x, a, b) == x);
            }
            else
            {
                CHECK_THROWS_AS(constrain(x, a, b), gsl::fail_fast);
            }
        }
        SECTION("bound")
        {
            auto a = GENERATE(-std::numeric_limits<double>::infinity(), -1., 0., 1.);
            auto b = 2.;
            auto x = GENERATE(-2., -1., 0., 1., 2., 3.);
            auto dx = GENERATE(0., 2.);
            auto xs = Bound(x, x + dx);
            CAPTURE(xs);
            CAPTURE(a);
            CAPTURE(b);

            if (xs.asInterval().overlapsWith(Interval(a, b)))
            {
                auto xs_ab = constrain(xs, a, b);
                auto xs_ab_fiducial = Bound(std::max(a, x), std::min(x + dx, b));
                CAPTURE(xs_ab);
                CAPTURE(xs_ab_fiducial);

                CHECK(xs_ab.matches(xs_ab_fiducial));
            }
            else
            {
                CHECK_THROWS_AS(constrain(xs, a, b), gsl::fail_fast);
            }
        }
        SECTION("NaN")
        {
            CHECK_THROWS_AS(std::isnan(constrain(nan, 0., 1.)), gsl::fail_fast);
            CHECK_THROWS_AS(isnan(constrain(Bound(nan, 2.), 0., 1.)), gsl::fail_fast);
            CHECK_THROWS_AS(isnan(constrain(Bound(0., nan), 0., 1.)), gsl::fail_fast);
            CHECK_THROWS_AS(isnan(constrain(Bound(nan, nan), 0., 1.)), gsl::fail_fast);
        }
    }
}


} // anonymous namespace
