
# intervals C++ library
# Author: Moritz Beutel
# intervals unit tests


cmake_minimum_required(VERSION 3.20)

find_package(Catch2 3.0 REQUIRED)
find_package(gsl-lite 0.40 REQUIRED)

include(TargetCompileSettings)

# common settings target
add_executable(test-intervals
    "test-set.cpp"
    "test-sign.cpp"
    "test-interval.cpp"
    "test-algorithm.cpp"
)
target_compile_definitions(test-intervals
    PRIVATE
        #CATCH_CONFIG_CONSOLE_WIDTH=120
        #CATCH_CLARA_CONFIG_CONSOLE_WIDTH=120
        CATCH_CONFIG_CPP17_UNCAUGHT_EXCEPTIONS
        CATCH_CONFIG_CPP17_STRING_VIEW
        CATCH_CONFIG_CPP17_VARIANT
        CATCH_CONFIG_CPP17_OPTIONAL
        CATCH_CONFIG_CPP17_BYTE
        gsl_CONFIG_CONTRACT_CHECKING_AUDIT  # run audit-level checks
        gsl_CONFIG_CONTRACT_VIOLATION_THROWS  # make gsl-lite's precondition assertions throw `gsl::fail_fast` on failure
)
target_compile_features(test-intervals
    PRIVATE
        cxx_std_20
)
target_link_libraries(test-intervals
    PRIVATE
        gsl::gsl-lite-v1
        Catch2::Catch2WithMain
        intervals
)
target_precompile_headers(test-intervals
    PRIVATE
        # C standard library
        <cmath>
        # C++ standard library
        <array>
        <bitset>
        <iosfwd>
        <limits>
        <compare>
        <numbers>
        <utility>
        <concepts>
        <algorithm>
        <type_traits>
        <initializer_list>
        # other libraries
        <gsl-lite/gsl-lite.hpp>
        <makeshift/concepts.hpp>
        <makeshift/type_traits.hpp>
        <catch2/catch_test_macros.hpp>
)
cmakeshift_target_compile_settings(test-intervals
    SOURCE_FILE_ENCODING "UTF-8"
)
add_test(NAME test-intervals COMMAND test-intervals)
set_property(TEST test-intervals PROPERTY FAIL_REGULAR_EXPRESSION "Sanitizer")
