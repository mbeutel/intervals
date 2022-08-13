
#include <limits>

#include <gsl-lite/gsl-lite.hpp>  // for fail_fast, type_identity<>

#include <catch2/catch_test_macros.hpp>

#include <intervals/sign.hpp>


namespace {

namespace gsl = ::gsl_lite;


TEST_CASE("sign", "sign enum")
{
    using intervals::sgn;
    using intervals::sign;
    constexpr double eps = std::numeric_limits<double>::epsilon();
    constexpr double min = std::numeric_limits<double>::min();
    constexpr double nan = std::numeric_limits<double>::quiet_NaN();

    SECTION("general")
    {
        CHECK(+sign::negative_sign == sign::negative_sign);
        CHECK(+sign::zero_sign == sign::zero_sign);
        CHECK(+sign::positive_sign == sign::positive_sign);
        CHECK(-sign::negative_sign == sign::positive_sign);
        CHECK(-sign::zero_sign == sign::zero_sign);
        CHECK(-sign::positive_sign == sign::negative_sign);
        CHECK(sign::negative_sign*sign::negative_sign == sign::positive_sign);
        CHECK(sign::positive_sign*sign::negative_sign == sign::negative_sign);
        CHECK(sign::negative_sign*sign::zero_sign == sign::zero_sign);
        CHECK(sign::zero_sign*sign::zero_sign == sign::zero_sign);
        CHECK(sign::positive_sign*sign::zero_sign == sign::zero_sign);
    }
    SECTION("double")
    {
        CHECK(sgn(0.) == sign::zero_sign);
        CHECK(sgn(-0.) == sign::zero_sign);
        CHECK(sgn(eps) == sign::positive_sign);
        CHECK(sgn(min) == sign::positive_sign);
        CHECK(sgn(1.) == sign::positive_sign);
        CHECK(sgn(-eps) == sign::negative_sign);
        CHECK(sgn(-min) == sign::negative_sign);
        CHECK(sgn(-1.) == sign::negative_sign);
        CHECK(sign::negative_sign*2. == -2.);
        CHECK(sign::zero_sign*2. == 0.);
        CHECK(sign::positive_sign*2. == 2.);
        CHECK_THROWS_AS(sgn(nan), gsl::fail_fast);
    }
    SECTION("int")
    {
        CHECK(sgn(-2) == sign::negative_sign);
        CHECK(sgn(-2ll) == sign::negative_sign);
        CHECK(sgn(0) == sign::zero_sign);
        CHECK(sgn(0u) == sign::zero_sign);
        CHECK(sgn(3) == sign::positive_sign);
        CHECK(sgn(3ll) == sign::positive_sign);
        CHECK(sign::negative_sign*2 == -2);
        CHECK(sign::zero_sign*2 == 0);
        CHECK(sign::positive_sign*2 == 2);
    }
}


} // anonymous namespace
