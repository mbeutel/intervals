
#include <limits>
#include <vector>
#include <ranges>
#include <numbers>
#include <iterator>

#include <gsl-lite/gsl-lite.hpp>  // for fail_fast, type_identity<>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <intervals/set.hpp>
#include <intervals/interval.hpp>
#include <intervals/algorithm.hpp>


namespace {

namespace gsl = ::gsl_lite;


enum Color { red = 2, green = 1, blue = 4 };
consteval auto
reflect(gsl::type_identity<Color>)
{
    return std::array{ red, green, blue };
}

static_assert(std::input_iterator<intervals::detail::single_value_iterator<int>>);
static_assert(std::input_iterator<intervals::detail::set_value_iterator<bool>>);
static_assert(std::input_iterator<intervals::detail::set_value_iterator<Color>>);
static_assert(std::input_iterator<intervals::detail::interval_value_iterator<int>>);
static_assert(std::input_iterator<intervals::detail::interval_value_iterator<std::vector<int>::iterator>>);
static_assert(std::ranges::input_range<intervals::detail::single_value_range<int>>);
static_assert(std::ranges::input_range<intervals::detail::set_value_range<bool>>);
static_assert(std::ranges::input_range<intervals::detail::set_value_range<Color>>);
static_assert(std::ranges::input_range<intervals::detail::interval_value_range<int>>);
static_assert(std::ranges::input_range<intervals::detail::interval_value_range<std::vector<int>::iterator>>);


template <typename RangeT>
auto
to_vector(RangeT&& range)
{
    using T = std::iter_value_t<decltype(range.begin())>;
    auto result = std::vector<T>{ };
    for (auto&& element : range)
    {
        result.push_back(element);
    }
    return result;
}

TEST_CASE("enumerate()")
{
    using intervals::set;
    using intervals::interval;
    using intervals::enumerate;

    SECTION("scalars")
    {
        CHECK_THAT(to_vector(enumerate(false)), Catch::Matchers::Equals(std::vector{ false }));
        CHECK_THAT(to_vector(enumerate(true)), Catch::Matchers::Equals(std::vector{ true }));
        CHECK_THAT(to_vector(enumerate(42)), Catch::Matchers::Equals(std::vector{ 42 }));
    }
    SECTION("set<bool>")
    {
        CHECK_THROWS_AS(enumerate(set<bool>{ }), gsl::fail_fast);
        CHECK_THAT(to_vector(enumerate(set{ false })), Catch::Matchers::Equals(std::vector{ false }));
        CHECK_THAT(to_vector(enumerate(set{ true })), Catch::Matchers::Equals(std::vector{ true }));
        CHECK_THAT(to_vector(enumerate(set{ false, true })), Catch::Matchers::Equals(std::vector{ false, true }));
    }
    SECTION("set<Color>")
    {
        CHECK_THROWS_AS(enumerate(set<Color>{ }), gsl::fail_fast);
        CHECK_THAT(to_vector(enumerate(set{ red })), Catch::Matchers::Equals(std::vector{ red }));
        CHECK_THAT(to_vector(enumerate(set{ blue })), Catch::Matchers::Equals(std::vector{ blue }));
        CHECK_THAT(to_vector(enumerate(set{ green, blue })), Catch::Matchers::Equals(std::vector{ green, blue }));
    }
    SECTION("interval<int>")
    {
        CHECK_THROWS_AS(enumerate(interval<int>{ }), gsl::fail_fast);
        CHECK_THAT(to_vector(enumerate(interval{ 3 })), Catch::Matchers::Equals(std::vector{ 3 }));
        CHECK_THAT(to_vector(enumerate(interval{ 3, 5 })), Catch::Matchers::Equals(std::vector{ 3, 4, 5 }));
    }
}

TEST_CASE("at()")
{
    using intervals::at;
    using intervals::set;
    using intervals::interval;

    auto colors = std::array{ blue, green, red };
    auto numbers = std::array{ 2, 4, 6, 8 };

    SECTION("scalars")
    {
        CHECK_THROWS_AS(at(colors, -1), gsl::fail_fast);
        CHECK(at(colors, 0) == blue);
        CHECK(at(colors, 2) == red);
        CHECK_THROWS_AS(at(colors, 3), gsl::fail_fast);
    }
    SECTION("sets")
    {
        CHECK_THROWS_AS(at(colors, interval{ -1 }), gsl::fail_fast);
        CHECK_THROWS_AS(at(colors, interval{ -1, 1 }), gsl::fail_fast);
        CHECK_THROWS_AS(at(colors, interval{ 1, 3 }), gsl::fail_fast);
        CHECK_THROWS_AS(at(colors, interval{ 3 }), gsl::fail_fast);
        CHECK(at(colors, interval{ 0, 1 }).matches(set{ blue, green }));
        CHECK(at(colors, interval{ 1, 2 }).matches(set{ green, red }));
        CHECK(at(colors, interval{ 0, 2 }).matches(set{ blue, green, red }));
        CHECK(at(colors, interval{ 1 }).matches(set{ green }));
    }
    SECTION("intervals")
    {
        CHECK_THROWS_AS(at(numbers, interval{ -1 }), gsl::fail_fast);
        CHECK_THROWS_AS(at(numbers, interval{ -1, 1 }), gsl::fail_fast);
        CHECK_THROWS_AS(at(numbers, interval{ 1, 4 }), gsl::fail_fast);
        CHECK_THROWS_AS(at(numbers, interval{ 4 }), gsl::fail_fast);
        CHECK(at(numbers, interval{ 0, 1 }).matches(interval{ 2, 4 }));
        CHECK(at(numbers, interval{ 1, 2 }).matches(interval{ 4, 6 }));
        CHECK(at(numbers, interval{ 0, 2 }).matches(interval{ 2, 6 }));
        CHECK(at(numbers, interval{ 1 }).matches(interval{ 4 }));
    }
}


} // anonymous namespace
