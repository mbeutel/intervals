
# intervals C++ library
# Author: Moritz Beutel
# example projects


cmake_minimum_required(VERSION 3.20)

find_package(gsl-lite 0.40 REQUIRED)
find_package(makeshift 4.0 REQUIRED)
find_package(fmt 9 REQUIRED)

add_library(simple-libs INTERFACE)
target_precompile_headers(simple-libs
    INTERFACE
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
        <fmt/core.h>
        <fmt/ostream.h>
        <makeshift/tuple.hpp>
        <makeshift/concepts.hpp>
        <makeshift/metadata.hpp>
        <makeshift/type_traits.hpp>
)
target_link_libraries(simple-libs
    INTERFACE
        gsl::gsl-lite-v1
        makeshift::makeshift
        fmt::fmt
        intervals
)

add_executable(simple1
    "simple1.cpp"
)
target_link_libraries(simple1
    PRIVATE
        simple-libs
)

add_executable(simple2
    "simple2.cpp"
)
target_link_libraries(simple2
    PRIVATE
        simple-libs
)

add_executable(simple3
    "simple3.cpp"
)
target_link_libraries(simple3
    PRIVATE
        simple-libs
)

add_executable(interpolate-nn
    "interpolate-nn.cpp"
)
target_link_libraries(interpolate-nn
    PRIVATE
        simple-libs
)

add_executable(interpolate-linear
    "interpolate-linear.cpp"
)
target_link_libraries(interpolate-linear
    PRIVATE
        simple-libs
)

add_executable(preprocess
    "preprocess.cpp"
)
target_link_libraries(preprocess
    PRIVATE
        simple-libs
)
