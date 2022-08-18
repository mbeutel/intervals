
# intervals C++ library
# Author: Moritz Beutel
# example projects


cmake_minimum_required(VERSION 3.20)

find_package(gsl-lite 0.40 REQUIRED)
find_package(makeshift 4.0 REQUIRED)
find_package(fmt 8 REQUIRED)

add_executable(simple1
    "simple1.cpp"
)
target_link_libraries(simple1
    PRIVATE
        gsl::gsl-lite-v1
        makeshift::makeshift
        fmt::fmt
        intervals
)
