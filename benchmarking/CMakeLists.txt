cmake_minimum_required(VERSION 3.22.1)

project(stevensTerminalBenchmarks CXX)

# Set C++23 standard for stevensTerminal compatibility
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force Release build for accurate benchmarking
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Ensure Release optimizations for benchmarking
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Configure Google Benchmark (disable testing to avoid GoogleTest dependency)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark testing")
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable gtest tests")

# Add Google Benchmark as subdirectory
add_subdirectory(benchmark)

# Create benchmark executable
add_executable(benchmarkStevensTerminal benchmark_terminal.cpp)

# Set C++23 for the benchmark executable
target_compile_features(benchmarkStevensTerminal PUBLIC cxx_std_23)

# Link Google Benchmark
target_link_libraries(benchmarkStevensTerminal 
    benchmark::benchmark
    benchmark::benchmark_main
    pthread
)

# Include benchmark headers (automatically handled by target_link_libraries above)

# Add compiler optimizations for Release builds
target_compile_options(benchmarkStevensTerminal PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native -DNDEBUG>
    $<$<CONFIG:Debug>:-O0 -g>
)

# Print build type
message(STATUS "stevensTerminal Benchmarks Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX Flags Release: ${CMAKE_CXX_FLAGS_RELEASE}")