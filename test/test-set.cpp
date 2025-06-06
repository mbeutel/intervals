
#include <array>
#include <functional>
#include <initializer_list>

#include <gsl-lite/gsl-lite.hpp>  // for fail_fast, type_identity<>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <intervals/set.hpp>


namespace {

namespace gsl = ::gsl_lite;


enum Color { red = 2, green = 1, blue = 4 };
consteval auto
reflect(gsl::type_identity<Color>)
{
    return std::array{ red, green, blue };
}


template <typename F>
intervals::set<bool>
collect(intervals::set<bool> x, intervals::set<bool> y, F&& func)
{
    auto result = intervals::set<bool>{ };
    for (bool cx : { false, true })
    {
        if (x.contains(cx))
        {
            for (bool cy : { false, true })
            {
                if (y.contains(cy))
                {
                    result.assign(func(x, y));
                }
            }
        }
    }
    return result;
}


TEST_CASE("set<>", "algebra with bounds")
{
    using intervals::set;

    SECTION("fail on uninitialized read")
    {
        auto value = GENERATE(false, true);
        CAPTURE(value);
        auto s0 = set<bool>{ };
        auto svalue = set<bool>{ value };

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
        auto svalue = set{ value };
        auto nsvalue = set{ !value };
        CHECK(svalue.contains(value));
        CHECK(svalue.contains(svalue));
        CHECK_FALSE(svalue.contains(!value));
        CHECK_FALSE(svalue.contains(nsvalue));
        CHECK(svalue.matches(value));
        CHECK(svalue.matches(svalue));
        CHECK_FALSE(svalue.matches(!value));
        CHECK_FALSE(svalue.matches(nsvalue));
        CHECK((svalue == svalue).matches(set{ true }));
        CHECK((svalue == nsvalue).matches(set{ false }));
        CHECK((svalue != svalue).matches(set{ false }));
        CHECK((svalue != nsvalue).matches(set{ true }));
        CHECK((!svalue).matches(nsvalue));
        CHECK((!nsvalue).matches(svalue));
        CHECK(possibly(svalue) == value);
        CHECK(always(svalue) == value);
    }
    SECTION("multi-valued sets (bool)")
    {
        auto value = GENERATE(false, true);
        CAPTURE(value);
        auto svalue = set{ value };
        auto s1 = set{ false, true };
        CHECK(s1.contains(s1));
        CHECK(s1.contains(value));
        CHECK(s1.contains(svalue));
        CHECK(s1.matches(s1));
        CHECK_FALSE(s1.matches(value));
        CHECK_FALSE(s1.matches(svalue));
        CHECK((s1 == s1).matches(s1));
        CHECK((s1 == svalue).matches(s1));
        CHECK((s1 != svalue).matches(s1));
        CHECK((!s1).matches(s1));
        CHECK(possibly(s1));
        CHECK_FALSE(always(s1));
    }
    SECTION("assignment (bool)")
    {
        using intervals::assign_partial;
        using intervals::reset;

        auto value = GENERATE(false, true);
        auto svalue = set{ value };
        auto s = intervals::set<bool>{ };
        s.assign(value);
        CHECK(possibly(s == set{ value }));
        CHECK(always(s == set{ value }));
        CHECK((s == svalue).matches(set{ true }));
        s.assign(!value);
        CHECK(possibly(s == set{ true, false }));
        CHECK_FALSE(always(s == set{ true, false }));
        s.reset();
        assign_partial(s, value);
        CHECK(s.matches(value));
        assign_partial(s, !value);
        CHECK(s.matches(set{ false, true }));
        reset(s, value);
        CHECK(s.matches(value));
    }
    SECTION("multi-valued sets (enum)")
    {
        auto s0 = set<Color>{ };
        auto sp1 = set{ red, green };
        auto sp2 = set{ red, blue };
        auto sp3 = set{ green, blue };
        auto s1 = set{ red, green, blue };
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
            CHECK((s1 == s1).matches(set{ false, true }));
            CHECK((sp1 == set{ red }).matches(set{ false, true }));
            CHECK((sp1 == set{ blue }).matches(set{ false }));
            CHECK((sp1 == sp2).matches(set{ false, true }));
            CHECK((sp1 == sp3).matches(set{ false, true }));
            CHECK((sp2 == sp3).matches(set{ false, true }));
            CHECK((s1 == sp1).matches(set{ false, true }));
            CHECK((s1 == sp2).matches(set{ false, true }));
            CHECK((s1 == sp3).matches(set{ false, true }));
        }
        SECTION("single value")
        {
            auto value = GENERATE(red, green, blue);
            CAPTURE(value);
            auto svalue = set{ value };
            CHECK(s1.contains(value));
            CHECK(s1.contains(svalue));
            CHECK_FALSE(s1.matches(value));
            CHECK_FALSE(s1.matches(svalue));
            CHECK(svalue.value() == value);
        }
    }
    SECTION("negation")
    {
            // !
        CHECK((!set{ true        }).matches(set{ false       }));
        CHECK((!set{ false       }).matches(set{ true        }));
        CHECK((!set{ false, true }).matches(set{ false, true }));
    }
    SECTION("mixed binary logical operators")
    {
        auto x = GENERATE(false, true);
        auto y = GENERATE(set{ false }, set{ true }, set{ false, true });
        CAPTURE(x);
        CAPTURE(y);
        {
            INFO("operation: x & y");
            auto z = x & y;
            auto zc = collect(x, y, std::bit_and<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: y & x");
            auto z = y & x;
            auto zc = collect(y, x, std::bit_and<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x | y");
            auto z = x | y;
            auto zc = collect(x, y, std::bit_or<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: y | x");
            auto z = y | x;
            auto zc = collect(y, x, std::bit_or<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x ^ y");
            auto z = x ^ y;
            auto zc = collect(x, y, std::bit_xor<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: y ^ x");
            auto z = y ^ x;
            auto zc = collect(y, x, std::bit_xor<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x == y");
            auto z = x == y;
            auto zc = collect(x, y, std::equal_to<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: y == x");
            auto z = y == x;
            auto zc = collect(y, x, std::equal_to<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x != y");
            auto z = x != y;
            auto zc = collect(x, y, std::not_equal_to<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: y != x");
            auto z = y != x;
            auto zc = collect(y, x, std::not_equal_to<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x <= y");
            auto z = x <= y;
            auto zc = collect(x, y, std::less_equal<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: y <= x");
            auto z = y <= x;
            auto zc = collect(y, x, std::less_equal<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x < y");
            auto z = x < y;
            auto zc = collect(x, y, std::less<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: y < x");
            auto z = y < x;
            auto zc = collect(y, x, std::less<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x >= y");
            auto z = x >= y;
            auto zc = collect(x, y, std::greater_equal<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: y >= x");
            auto z = y >= x;
            auto zc = collect(y, x, std::greater_equal<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x > y");
            auto z = x > y;
            auto zc = collect(x, y, std::greater<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: y > x");
            auto z = y > x;
            auto zc = collect(y, x, std::greater<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
    }
    SECTION("binary logical operators")
    {
        auto x = GENERATE(set{ false }, set{ true }, set{ false, true });
        auto y = GENERATE(set{ false }, set{ true }, set{ false, true });
        CAPTURE(x);
        CAPTURE(y);
        {
            INFO("operation: x & y");
            auto z = x & y;
            auto zc = collect(x, y, std::bit_and<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x | y");
            auto z = x | y;
            auto zc = collect(x, y, std::bit_or<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x ^ y");
            auto z = x ^ y;
            auto zc = collect(x, y, std::bit_xor<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x == y");
            auto z = x == y;
            auto zc = collect(x, y, std::equal_to<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x != y");
            auto z = x != y;
            auto zc = collect(x, y, std::not_equal_to<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x <= y");
            auto z = x <= y;
            auto zc = collect(x, y, std::less_equal<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x < y");
            auto z = x < y;
            auto zc = collect(x, y, std::less<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x >= y");
            auto z = x >= y;
            auto zc = collect(x, y, std::greater_equal<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
        {
            INFO("operation: x > y");
            auto z = x > y;
            auto zc = collect(x, y, std::greater<>{ });
            CAPTURE(z);
            CAPTURE(zc);
            CHECK(z.matches(zc));
        }
    }
    SECTION("Boolean predicates")
    {
            // possibly()
        CHECK(intervals::possibly(true));
        CHECK_FALSE(intervals::possibly(false));
        CHECK(possibly(set{ true }));
        CHECK(possibly(set{ false, true }));
        CHECK_FALSE(possibly(set{ false }));

            // possibly_not()
        CHECK(intervals::possibly_not(false));
        CHECK_FALSE(intervals::possibly_not(true));
        CHECK(possibly_not(set{ false }));
        CHECK(possibly_not(set{ false, true }));
        CHECK_FALSE(possibly_not(set{ true }));

            // always()
        CHECK(intervals::always(true));
        CHECK_FALSE(intervals::always(false));
        CHECK(always(set{ true }));
        CHECK_FALSE(always(set{ false, true }));
        CHECK_FALSE(always(set{ false }));

            // never()
        CHECK(intervals::never(false));
        CHECK_FALSE(intervals::never(true));
        CHECK(never(set{ false }));
        CHECK_FALSE(never(set{ false, true }));
        CHECK_FALSE(never(set{ true }));

            // contingent()
        CHECK_FALSE(intervals::contingent(true));
        CHECK_FALSE(intervals::contingent(false));
        CHECK_FALSE(contingent(set{ true }));
        CHECK(contingent(set{ false, true }));
        CHECK_FALSE(contingent(set{ false }));
    }
}


}  // anonymous namespace
