cmake_minimum_required(VERSION 3.0.0)

project(cpp-error-handling)

enable_testing()

add_definitions("-std=c++17" "-O3" )

#include "expected/include/tl/expected.hpp"
#include "benchmark/include/benchmark/benchmark.h"
add_subdirectory("benchmark")
include_directories("benchmark/include/" "expected/include/")
add_executable(err main.cpp)
target_link_libraries(err benchmark pthread)