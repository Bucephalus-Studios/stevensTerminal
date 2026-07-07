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
    # PDC_SDL2_BUILD/PDC_SDL2_DEPS_BUILD/PDC_GL_BUILD are PDCursesMod's real
    # option names (see cmake/build_options.cmake) — we only need the wincon
    # (Windows console) backend, so skip the SDL2/OpenGL backends and their
    # vendored dependency builds. wincon/wingui/vt are always added on WIN32
    # (no option to skip them individually) but are cheap to configure.
    set(PDC_BUILD_SHARED    OFF CACHE BOOL "" FORCE)
    set(PDC_SDL2_BUILD      OFF CACHE BOOL "" FORCE)
    set(PDC_SDL2_DEPS_BUILD OFF CACHE BOOL "" FORCE)
    set(PDC_GL_BUILD        OFF CACHE BOOL "" FORCE)
    # PDC_UTF8 -> -DPDC_FORCE_UTF8 (see cmake/gen_config_header.cmake), which
    # curses.h treats as implying PDC_WIDE too. Needed so narrow char* strings
    # containing raw UTF-8 bytes (e.g. BorderStyle.hpp's "◆") get decoded as
    # UTF-8 codepoints instead of one garbled codepage byte per glyph byte —
    # matching how ncursesw already behaves on Linux. Set PDC_WIDE explicitly
    # too rather than relying solely on curses.h's implication.
    set(PDC_WIDE            ON  CACHE BOOL "" FORCE)
    set(PDC_UTF8            ON  CACHE BOOL "" FORCE)

    FetchContent_Declare(
        PDCurses
        GIT_REPOSITORY https://github.com/Bill-Gray/PDCursesMod.git
        GIT_TAG        v4.3.7
        GIT_SHALLOW    TRUE
        # See cmake/patch_pdcurses_cmake_source_dir.cmake for why this is needed.
        PATCH_COMMAND  ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch_pdcurses_cmake_source_dir.cmake"
    )
    FetchContent_MakeAvailable(PDCurses)

    # Real target names, per cmake/project_common.cmake: "${PROJECT_NAME}_pdcurses"
    # (shared) or "${PROJECT_NAME}_pdcursesstatic" (static) — not "PDCurses_wincon".
    if(TARGET wincon_pdcursesstatic)
        target_link_libraries(stevensTerminal PUBLIC wincon_pdcursesstatic)
    elseif(TARGET wincon_pdcurses)
        target_link_libraries(stevensTerminal PUBLIC wincon_pdcurses)
    else()
        message(FATAL_ERROR "Could not find a PDCurses wincon target after FetchContent.")
    endif()

    target_include_directories(stevensTerminal PUBLIC ${pdcurses_SOURCE_DIR})

    # PDCursesMod's build only applies -DPDC_WIDE/-DPDC_FORCE_UTF8 to its own
    # sources via add_definitions() (directory-scoped, set in gen_config_header.cmake)
    # — it never propagates them to consumers via target_compile_definitions.
    # curses.h uses these defines to rename several functions (e.g. endwin ->
    # endwin_u64_4302) specifically so a define mismatch between the library
    # and its consumers fails to LINK rather than silently misbehaving at
    # runtime. PUBLIC here ensures stevensTerminal's own sources AND anything
    # linking against it (cultgame) see the exact same defines the library
    # was actually built with.
    target_compile_definitions(stevensTerminal PUBLIC PDC_WIDE PDC_FORCE_UTF8)
endif()

find_package(Threads REQUIRED)
target_link_libraries(stevensTerminal PUBLIC Threads::Threads)

option(STEVENSTERMINALBUILD_TESTS "Build tests" OFF)
option(STEVENSTERMINALBUILD_BENCHMARKS "Build benchmarks" OFF)

if(STEVENSTERMINALBUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(testing)
endif()

if(STEVENSTERMINALBUILD_BENCHMARKS)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
    )
    FetchContent_MakeAvailable(benchmark)
    add_subdirectory(benchmarking)
endif()
