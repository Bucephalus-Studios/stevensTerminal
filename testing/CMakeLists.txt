cmake_minimum_required(VERSION 3.22.1)

project(testStevensTerminal C CXX)

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

# Fetch Stevens utility libraries from GitHub
FetchContent_Declare(
  stevensStringLib
  GIT_REPOSITORY https://github.com/Bucephalus-Studios/stevensStringLib.git
  GIT_TAG        main
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(
  stevensVectorLib
  GIT_REPOSITORY https://github.com/Bucephalus-Studios/stevensVectorLib.git
  GIT_TAG        main
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(
  stevensMapLib
  GIT_REPOSITORY https://github.com/Bucephalus-Studios/stevensMapLib.git
  GIT_TAG        main
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(
  stevensMathLib
  GIT_REPOSITORY https://github.com/Bucephalus-Studios/stevensMathLib.git
  GIT_TAG        main
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(
  stevensFileLib
  GIT_REPOSITORY https://github.com/Bucephalus-Studios/stevensFileLib.git
  GIT_TAG        main
  GIT_SHALLOW    TRUE
)

# Populate the libraries (download but don't configure/build)
# Since they're header-only, we just need the source files
FetchContent_Populate(stevensStringLib)
FetchContent_Populate(stevensVectorLib)
FetchContent_Populate(stevensMapLib)
FetchContent_Populate(stevensMathLib)
FetchContent_Populate(stevensFileLib)

# Create test executable
add_executable(${PROJECT_NAME} test.cpp)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Find ncurses library (required by stevensTerminal)
find_package(Curses REQUIRED)

# Add include directories for stevensTerminal and fetched libraries
include_directories(
	${CMAKE_SOURCE_DIR}/..
	${stevensstringlib_SOURCE_DIR}
	${stevensvectorlib_SOURCE_DIR}
	${stevensmaplib_SOURCE_DIR}
	${stevensmathlib_SOURCE_DIR}
	${stevensfilelib_SOURCE_DIR}
	${CURSES_INCLUDE_DIR}
)

# Link Google Test and ncurses
target_link_libraries(${PROJECT_NAME} PUBLIC
	gtest_main
	${CURSES_LIBRARIES}
)

# Enable testing
enable_testing()
add_test(
	NAME ${PROJECT_NAME}
	COMMAND ${PROJECT_NAME}
)
