# PDCursesMod's Windows console (wincon) backend can render color two ways:
#
#   1. Legacy: WriteConsoleOutput() with a packed attribute byte, via the
#      original Win32 console color API. Present since Windows NT 3.1 (early
#      1990s) and unchanged since - works identically on every NT-based
#      Windows version (7, 8, 10, 11, Server editions, all of it).
#   2. ANSI/VT: emits raw ANSI escape sequences into the output stream and
#      depends on the console host actually interpreting them, via
#      ENABLE_VIRTUAL_TERMINAL_PROCESSING - only available starting Windows 10
#      version 1511 (Nov. 2015), and reliability varies across console hosts.
#
# wincon/pdcscrn.c decides which to use via the pdc_ansi flag, computed at
# startup from environment variables (WT_SESSION for Windows Terminal,
# ConEmuANSI for ConEmu) - i.e. opportunistically upgrading to the ANSI path
# whenever it looks supported, regardless of whether it actually works
# correctly in that specific environment.
#
# Only applied when STEVENSTERMINAL_FORCE_LEGACY_PDCURSES_COLOR is ON (a
# consumer opt-in, not stevensTerminal's default - other projects using this
# library may prefer the richer ANSI color modes on environments where they
# do work correctly, and may not share cultgame's "broadest possible Windows
# version support" priority).
#
# Run as a FetchContent/ExternalProject PATCH_COMMAND, with the working
# directory set to the populated PDCursesMod source tree.
#
# BRITTLENESS NOTE: this does an exact text match against the PDCursesMod
# source as it exists at the pinned GIT_TAG. If that tag is ever bumped to a
# newer PDCursesMod release where this code has changed, the match below will
# fail. string(REPLACE) does not error on a non-match - it silently returns
# the input unchanged - which would silently reintroduce the auto-detecting
# ANSI/VT behavior with no warning. To avoid that silent failure mode, this
# script verifies the replacement actually happened and fails loudly
# (FATAL_ERROR) if not, so bumping the pinned tag surfaces this immediately
# as a build break rather than a quiet behavioral regression.

set(_pdcscrn_file "wincon/pdcscrn.c")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/${_pdcscrn_file}" _contents)

set(_old_block "    pdc_ansi =
#ifdef PDC_WIDE
        pdc_wt ? TRUE :
#endif
        pdc_conemu ? !strcmp(str, \"ON\") : FALSE;")

set(_new_block "    /* Forced FALSE unconditionally - see cmake/patch_pdcurses_force_legacy_color.cmake
       in stevensTerminal for why. */
    pdc_ansi = FALSE;")

string(REPLACE "${_old_block}" "${_new_block}" _patched_contents "${_contents}")

if(_patched_contents STREQUAL _contents)
    message(FATAL_ERROR
        "patch_pdcurses_force_legacy_color.cmake: expected text block not found "
        "in ${_pdcscrn_file}. PDCursesMod's source has likely changed since this "
        "patch was written (e.g. the pinned GIT_TAG was bumped) - update the "
        "_old_block match in this script to reflect the new source.")
endif()

file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/${_pdcscrn_file}" "${_patched_contents}")
