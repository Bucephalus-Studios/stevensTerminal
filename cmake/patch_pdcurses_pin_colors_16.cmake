# Companion to patch_pdcurses_force_legacy_color.cmake - see that file for the
# full rationale (legacy WriteConsoleOutput color path vs. ANSI/VT path) and
# the brittleness note about exact-text-match patching against a pinned
# GIT_TAG. Only applied when STEVENSTERMINAL_FORCE_LEGACY_PDCURSES_COLOR is ON
# (a consumer opt-in, not stevensTerminal's default).
#
# PDC_set_blink() (wincon/pdcsetsc.c) additionally auto-escalates COLORS from
# 16 up to PDC_MAXCOL (256+) whenever the console negotiates VT support,
# independently of the pdc_ansi flag. Pinning COLORS at 16 keeps color pair
# generation (see stevensTerminal's Colors::curses_setup_colorPairs) within
# the range the legacy path actually supports.
#
# Run as a FetchContent/ExternalProject PATCH_COMMAND, with the working
# directory set to the populated PDCursesMod source tree.

set(_pdcsetsc_file "wincon/pdcsetsc.c")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/${_pdcsetsc_file}" _contents)

set(_old_block "        COLORS = 16;
        if (PDC_can_change_color()) /* is_nt */
        {
            if (pdc_conemu || SetConsoleMode(pdc_con_out, 0x0004)) /* VT */
                COLORS = PDC_MAXCOL;

            if (!pdc_conemu)
                SetConsoleMode(pdc_con_out, 0x0010); /* LVB */
        }")

set(_new_block "        /* Pinned at 16 - see cmake/patch_pdcurses_pin_colors_16.cmake in
           stevensTerminal for why. */
        COLORS = 16;")

string(REPLACE "${_old_block}" "${_new_block}" _patched_contents "${_contents}")

if(_patched_contents STREQUAL _contents)
    message(FATAL_ERROR
        "patch_pdcurses_pin_colors_16.cmake: expected text block not found in "
        "${_pdcsetsc_file}. PDCursesMod's source has likely changed since this "
        "patch was written (e.g. the pinned GIT_TAG was bumped) - update the "
        "_old_block match in this script to reflect the new source.")
endif()

file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/${_pdcsetsc_file}" "${_patched_contents}")
