cmake_minimum_required(VERSION 3.22.1)

project(tests C CXX)

# GoogleTest requires at least C++14, and stevensTerminal takes C++ 20
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Google Test using FetchContent (modern CMake approach)
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
  GIT_SHALLOW    TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Create test executable
add_executable(${PROJECT_NAME} test.cpp)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Find and link curses library (platform-specific)
if(WIN32)
	# On Windows, use PDCurses
	find_package(Curses REQUIRED)
	include_directories(${CURSES_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} PUBLIC
		gtest_main
		${CURSES_LIBRARIES}
	)
else()
	# On Linux/Unix, use ncurses
	find_package(Curses REQUIRED)
	include_directories(${CURSES_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} PUBLIC
		gtest_main
		ncurses
	)
endif()

# Enable testing
enable_testing()
add_test(
	NAME ${PROJECT_NAME}
	COMMAND ${PROJECT_NAME}
)
