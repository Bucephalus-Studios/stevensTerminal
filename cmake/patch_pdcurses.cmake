# Dispatcher for all PDCursesMod patches applied via stevensTerminal's
# FetchContent PATCH_COMMAND.
#
# Always applies the CMAKE_SOURCE_DIR fix (required unconditionally for
# PDCursesMod to configure at all when pulled in via FetchContent - see
# patch_pdcurses_cmake_source_dir.cmake).
#
# Conditionally applies the legacy-color patches when
# STEVENSTERMINAL_FORCE_LEGACY_PDCURSES_COLOR is ON. That variable is passed
# in via -D on the command line (not read from the parent project's cache)
# because FetchContent's PATCH_COMMAND runs as a standalone `cmake -P`
# invocation with no access to the parent build's cache variables otherwise.

get_filename_component(_patch_dir "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)

include("${_patch_dir}/patch_pdcurses_cmake_source_dir.cmake")

# patch_pdcurses_force_legacy_color.cmake / patch_pdcurses_pin_colors_16.cmake
# disabled: these were added while chasing a "no color at all" bug that
# turned out to be caused by something else entirely (hardcoded bright-color
# numbers in stevensTerminal.cpp assuming the wrong RGB/BGR bit ordering -
# now fixed there). Once that real bug was fixed, these two were never
# re-verified as still necessary - disabled to test whether PDCurses' default
# ANSI/VT auto-detection is fine on its own now. Re-enable if colors break
# again in a way these two specifically explain.
# if(STEVENSTERMINAL_FORCE_LEGACY_PDCURSES_COLOR)
#     include("${_patch_dir}/patch_pdcurses_force_legacy_color.cmake")
#     include("${_patch_dir}/patch_pdcurses_pin_colors_16.cmake")
# endif()
