
# intervals C++ library
# Author: Moritz Beutel
# intervals library target


cmake_minimum_required(VERSION 3.30)

find_package(gsl-lite 0.43 REQUIRED)
find_package(makeshift 4.0 REQUIRED)

add_library(intervals INTERFACE
    "intervals.natvis"
)
add_library(intervals::intervals ALIAS intervals)

target_compile_features(intervals INTERFACE cxx_std_20)

target_include_directories(intervals
    INTERFACE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
)

target_link_libraries(intervals
    INTERFACE
        gsl::gsl-lite-v1
        makeshift::makeshift
)

install(
    DIRECTORY
        "${PROJECT_SOURCE_DIR}/include/" # the trailing '/' is significant
    TYPE INCLUDE
)
install(
    TARGETS intervals
    EXPORT intervals
    INCLUDES
)
