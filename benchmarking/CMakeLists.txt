cmake_minimum_required(VERSION 3.22.1)

project(benchmark CXX)

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

# Fetch Google Benchmark using FetchContent (modern CMake approach)
include(FetchContent)
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.1
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(googlebenchmark)

# Find and link curses library (platform-specific)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# Create benchmark executable
add_executable(benchmark benchmark_terminal.cpp)

# Set C++23 for the benchmark executable
target_compile_features(benchmark PUBLIC cxx_std_23)

# Link Google Benchmark and curses (platform-specific)
if(WIN32)
	# On Windows, use PDCurses
	target_link_libraries(benchmark
		benchmark::benchmark
		benchmark::benchmark_main
		${CURSES_LIBRARIES}
	)
else()
	# On Linux/Unix, use ncurses and pthread
	target_link_libraries(benchmark
		benchmark::benchmark
		benchmark::benchmark_main
		ncurses
		pthread
	)
endif()

# Include benchmark headers (automatically handled by target_link_libraries above)

# Add compiler optimizations for Release builds
target_compile_options(benchmark PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native -DNDEBUG>
    $<$<CONFIG:Debug>:-O0 -g>
)

# Print build type
message(STATUS "Benchmark Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX Flags Release: ${CMAKE_CXX_FLAGS_RELEASE}")