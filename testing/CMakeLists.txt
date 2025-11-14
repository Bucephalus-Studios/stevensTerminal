cmake_minimum_required(VERSION 3.22.1)

project(testStevensTerminal C CXX)

add_executable(${PROJECT_NAME} test.cpp)

# GoogleTest requires at least C++14, and stevensTerminal takes C++ 20
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)
target_compile_features(testStevensTerminal PUBLIC cxx_std_20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(googletest-1.15.2)

#add_library(${PROJECT_NAME} STATIC ${Sources}) #${Headers})

target_link_libraries(${PROJECT_NAME} PUBLIC
	gtest_main
)

add_test(
	NAME ${PROJECT_NAME}
	COMMAND ${PROJECT_NAME}
)
