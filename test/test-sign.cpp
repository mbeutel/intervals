
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
        CHECK(+sign::negativeSign == sign::negativeSign);
        CHECK(+sign::zeroSign == sign::zeroSign);
        CHECK(+sign::positiveSign == sign::positiveSign);
        CHECK(-sign::negativeSign == sign::positiveSign);
        CHECK(-sign::zeroSign == sign::zeroSign);
        CHECK(-sign::positiveSign == sign::negativeSign);
        CHECK(sign::negativeSign*sign::negativeSign == sign::positiveSign);
        CHECK(sign::positiveSign*sign::negativeSign == sign::negativeSign);
        CHECK(sign::negativeSign*sign::zeroSign == sign::zeroSign);
        CHECK(sign::zeroSign*sign::zeroSign == sign::zeroSign);
        CHECK(sign::positiveSign*sign::zeroSign == sign::zeroSign);
    }
    SECTION("double")
    {
        CHECK(sgn(0.) == sign::zeroSign);
        CHECK(sgn(-0.) == sign::zeroSign);
        CHECK(sgn(eps) == sign::positiveSign);
        CHECK(sgn(min) == sign::positiveSign);
        CHECK(sgn(1.) == sign::positiveSign);
        CHECK(sgn(-eps) == sign::negativeSign);
        CHECK(sgn(-min) == sign::negativeSign);
        CHECK(sgn(-1.) == sign::negativeSign);
        CHECK(sign::negativeSign*2. == -2.);
        CHECK(sign::zeroSign*2. == 0.);
        CHECK(sign::positiveSign*2. == 2.);
        CHECK_THROWS_AS(sgn(nan), gsl::fail_fast);
    }
    SECTION("int")
    {
        CHECK(sgn(-2) == sign::negativeSign);
        CHECK(sgn(-2ll) == sign::negativeSign);
        CHECK(sgn(0) == sign::zeroSign);
        CHECK(sgn(0u) == sign::zeroSign);
        CHECK(sgn(3) == sign::positiveSign);
        CHECK(sgn(3ll) == sign::positiveSign);
        CHECK(sign::negativeSign*2 == -2);
        CHECK(sign::zeroSign*2 == 0);
        CHECK(sign::positiveSign*2 == 2);
    }
}


} // anonymous namespace
