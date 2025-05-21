
# intervals C++ library
# Author: Moritz Beutel


cmake_minimum_required(VERSION 3.30)

project(intervals
    VERSION 1.0.0
    LANGUAGES CXX
)

# Define build options.
option(INTERVALS_BUILD_TESTING "Build tests" OFF)
option(INTERVALS_BUILD_EXAMPLES "Build examples" OFF)
set(INTERVALS_COMPILE_OPTIONS "" CACHE STRING "Extra compile options which should not be passed on when building dependencies (e.g. warning flags)")

# Obtain source dependencies.
# We use CPM to fetch dependencies in a stand-alone build but configure it to prefer packages provided through other means.
option(CPM_USE_LOCAL_PACKAGES "Always try to use `find_package` to get dependencies" ON)
option(CPM_LOCAL_PACKAGES_ONLY "Only use `find_package` to get dependencies" OFF)
set(CPM_SOURCE_CACHE "${CMAKE_BINARY_DIR}/CPM-cache")
include(cmake/CPM.cmake)
CPMAddPackage(NAME gsl-lite      VERSION  1.0.1 GITHUB_REPOSITORY gsl-lite/gsl-lite   SYSTEM OFF)
CPMAddPackage(NAME makeshift     GIT_TAG master GITHUB_REPOSITORY mbeutel/makeshift   SYSTEM OFF)
if(INTERVALS_BUILD_TESTING OR INTERVALS_BUILD_EXAMPLES)
    CPMAddPackage(NAME fmt       GIT_TAG 11.0.2 GITHUB_REPOSITORY fmtlib/fmt          SYSTEM OFF)
endif()
if(INTERVALS_BUILD_TESTING)
    set(CMAKE_CXX_STANDARD 17)  # workaround, cf. https://github.com/catchorg/Catch2/issues/2046
    CPMAddPackage(NAME Catch2    VERSION  3.8.1 GITHUB_REPOSITORY catchorg/Catch2     SYSTEM OFF)
endif()

# Include target definitions.
add_subdirectory(src)
if(INTERVALS_BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
if(INTERVALS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Install.
include(cmake/InstallBasicPackageFiles.cmake)
install_basic_package_files(intervals
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
    DEPENDENCIES
        "gsl-lite 1.0"
        "makeshift 4.0"
)
