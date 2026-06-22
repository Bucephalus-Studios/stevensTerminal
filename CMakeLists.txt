cmake_minimum_required(VERSION 3.16)
project(stevensTerminal VERSION 1.0 LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(stevensMathLib
    GIT_REPOSITORY https://github.com/Bucephalus-Studios/stevensMathLib.git
    GIT_TAG main
)
FetchContent_Declare(stevensMapLib
    GIT_REPOSITORY https://github.com/Bucephalus-Studios/stevensMapLib.git
    GIT_TAG main
)
FetchContent_Declare(stevensVectorLib
    GIT_REPOSITORY https://github.com/Bucephalus-Studios/stevensVectorLib.git
    GIT_TAG main
)
FetchContent_Declare(stevensStringLib
    GIT_REPOSITORY https://github.com/Bucephalus-Studios/stevensStringLib.git
    GIT_TAG main
)
FetchContent_Declare(stevensFileLib
    GIT_REPOSITORY https://github.com/Bucephalus-Studios/stevensFileLib.git
    GIT_TAG main
)
FetchContent_MakeAvailable(stevensMathLib stevensMapLib stevensVectorLib stevensStringLib stevensFileLib)

add_library(stevensTerminal STATIC stevensTerminal.cpp)

target_compile_features(stevensTerminal PUBLIC cxx_std_23)

target_include_directories(stevensTerminal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(stevensTerminal PUBLIC
    stevensMathLib
    stevensMapLib
    stevensVectorLib
    stevensStringLib
    stevensFileLib
)

if(UNIX AND NOT APPLE)
    find_package(Curses REQUIRED)
    target_link_libraries(stevensTerminal PUBLIC ${CURSES_LIBRARIES})
    target_include_directories(stevensTerminal PUBLIC ${CURSES_INCLUDE_DIR})
elseif(WIN32)
    set(PDC_BUILD_WINCON ON  CACHE BOOL "" FORCE)
    set(PDC_BUILD_SDL2   OFF CACHE BOOL "" FORCE)
    set(PDC_BUILD_VT     OFF CACHE BOOL "" FORCE)
    set(PDC_BUILD_WINGUI OFF CACHE BOOL "" FORCE)
    set(PDC_BUILD_X11    OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        PDCurses
        GIT_REPOSITORY https://github.com/Bill-Gray/PDCursesMod.git
        GIT_TAG        ver4.3.7
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(PDCurses)

    if(TARGET PDCurses_wincon)
        target_link_libraries(stevensTerminal PUBLIC PDCurses_wincon)
    elseif(TARGET PDCurses)
        target_link_libraries(stevensTerminal PUBLIC PDCurses)
    else()
        message(FATAL_ERROR "Could not find a PDCurses target after FetchContent.")
    endif()

    target_include_directories(stevensTerminal PUBLIC ${pdcurses_SOURCE_DIR})
endif()

find_package(Threads REQUIRED)
target_link_libraries(stevensTerminal PUBLIC Threads::Threads)
