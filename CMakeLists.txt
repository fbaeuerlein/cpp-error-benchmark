cmake_minimum_required(VERSION 3.0.0)

project(cpp-error-handling)

enable_testing()

add_definitions("-std=c++17" "-O3" )

add_subdirectory("external/googletest") # include before benchmark
add_subdirectory("external/benchmark")

include_directories("external/benchmark/include/" "external/expected/include/")
add_executable(bench benchmark.cpp)
target_link_libraries(bench benchmark pthread)