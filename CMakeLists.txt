
# intervals C++ library
# Author: Moritz Beutel


cmake_minimum_required(VERSION 3.20)

project(intervals
    VERSION 1.0.0
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Define build options.
option(INTERVALS_BUILD_TESTING "Build tests" OFF)
option(INTERVALS_BUILD_EXAMPLES "Build examples" OFF)

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
include(InstallBasicPackageFiles)
cmakeshift_install_basic_package_files(intervals
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
    DEPENDENCIES
        "gsl-lite 0.40"
        "makeshift 4.0"
)
