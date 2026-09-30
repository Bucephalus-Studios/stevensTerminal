# PDCursesMod's own build scripts (CMakeLists.txt, cmake/get_version.cmake,
# cmake/project_common.cmake, cmake/dll_version.cmake, etc.) reference
# CMAKE_SOURCE_DIR directly, assuming PDCursesMod is always the top-level
# CMake project. That breaks when it's pulled in via FetchContent/
# add_subdirectory from another project (like stevensTerminal), since
# CMAKE_SOURCE_DIR then points at the *including* project's source tree
# instead of PDCursesMod's own.
#
# CMAKE_CURRENT_SOURCE_DIR isn't a safe substitute either: dll_version.cmake
# is `include()`d indirectly from each platform subdirectory's own
# CMakeLists.txt (sdl2/, gl/, wincon/, ...), each its own add_subdirectory
# scope, so CMAKE_CURRENT_SOURCE_DIR there is the *subdirectory*, not the
# PDCursesMod root where cmake/version.in.cmake etc. actually live.
#
# Fix: define one fixed variable (PDCURSES_ROOT_DIR) at the top-level
# CMakeLists.txt, where CMAKE_CURRENT_SOURCE_DIR is guaranteed correct
# regardless of whether PDCursesMod is top-level or add_subdirectory'd in.
# It's inherited into every nested subdirectory scope, so it stays correct
# everywhere. Replace all CMAKE_SOURCE_DIR references with it.
#
# Run as a FetchContent/ExternalProject PATCH_COMMAND, with the working
# directory set to the populated PDCursesMod source tree.

set(_pdcurses_files_to_patch
    CMakeLists.txt
    cmake/project_common.cmake
    cmake/get_version.cmake
    cmake/dll_version.cmake
    cmake/build_dependencies.cmake
    cmake/sdl2_ttf/CMakeLists.txt
)

foreach(_f ${_pdcurses_files_to_patch})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_f}")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/${_f}" _contents)
        string(REPLACE "CMAKE_SOURCE_DIR" "PDCURSES_ROOT_DIR" _contents "${_contents}")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/${_f}" "${_contents}")
    endif()
endforeach()

# Define PDCURSES_ROOT_DIR at the very top of the top-level CMakeLists.txt,
# before it's first used (the CMAKE_MODULE_PATH line that used to read
# CMAKE_SOURCE_DIR, now PDCURSES_ROOT_DIR).
#
# BRITTLENESS NOTE: this matches the exact "cmake_minimum_required(VERSION
# 3.11)" line from the pinned GIT_TAG. If that tag is ever bumped and the
# version string changes, this silently fails to insert the definition
# (string(REPLACE) doesn't error on a non-match), leaving PDCURSES_ROOT_DIR
# undefined - so verify the replacement actually happened and fail loudly.
#
# Idempotency: FetchContent's populate/patch step can be re-triggered by an
# unrelated CMakeLists.txt change (e.g. adding a new target) against a source
# tree this script already patched successfully. Skip re-inserting (and skip
# the brittleness check below, which would otherwise fail against text that's
# no longer there) if the insertion has already happened.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt" _top_contents)
string(FIND "${_top_contents}" "set(PDCURSES_ROOT_DIR" _already_patched_pos)
if(_already_patched_pos EQUAL -1)
    string(REPLACE
        "cmake_minimum_required(VERSION 3.11)"
        "cmake_minimum_required(VERSION 3.11)\nset(PDCURSES_ROOT_DIR \"\${CMAKE_CURRENT_SOURCE_DIR}\")"
        _patched_top_contents "${_top_contents}")

    if(_patched_top_contents STREQUAL _top_contents)
        message(FATAL_ERROR
            "patch_pdcurses_cmake_source_dir.cmake: expected "
            "'cmake_minimum_required(VERSION 3.11)' line not found in "
            "CMakeLists.txt. PDCursesMod's source has likely changed since this "
            "patch was written (e.g. the pinned GIT_TAG was bumped) - update this "
            "script to reflect the new source.")
    endif()

    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt" "${_patched_top_contents}")
endif()
