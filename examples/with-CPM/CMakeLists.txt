﻿
cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

project(my-program LANGUAGES CXX)

# Obtain source dependencies.
set(CPM_SOURCE_CACHE "${CMAKE_BINARY_DIR}/CPM-cache")
include(cmake/CPM.cmake)
CPMFindPackage(NAME gsl-lite  VERSION  1.0.1 GITHUB_REPOSITORY gsl-lite/gsl-lite)
CPMFindPackage(NAME makeshift GIT_TAG master GITHUB_REPOSITORY mbeutel/makeshift)
CPMFindPackage(NAME intervals GIT_TAG master GITHUB_REPOSITORY mbeutel/intervals)

find_package(intervals 1.0 REQUIRED)

add_executable(my-program main.cpp)
target_compile_features(my-program PRIVATE cxx_std_20)
target_link_libraries(my-program PRIVATE intervals::intervals)
