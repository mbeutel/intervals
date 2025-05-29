
# intervals C++ library
# Author: Moritz Beutel
# example projects


cmake_minimum_required(VERSION 3.20)

find_package(gsl-lite 1.0 REQUIRED)
find_package(makeshift 4.0 REQUIRED)
find_package(fmt REQUIRED)

add_library(simple-libs INTERFACE)
target_link_libraries(simple-libs
    INTERFACE
        gsl-lite::gsl-lite
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

add_executable(doc_sample
    "doc/sample.cpp"
)
target_link_libraries(doc_sample
    PRIVATE
        simple-libs
)
