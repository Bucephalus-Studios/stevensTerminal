#pragma once
/**
 * @file Core.hpp
 * @brief Core foundations for stevensTerminal library
 *
 * This header provides the essential includes, platform detection,
 * namespace setup, global state, and fundamental functions for the
 * stevensTerminal library.
 *
 * Part of the Stevens Terminal Library
 * Originally created July 2019
 */

#if defined(_WIN32) || defined(__MSDOS__)
    // Must be set before <windows.h> is included below (it's the first thing
    // in this translation unit to pull it in): windows.h otherwise #defines
    // min/max macros that break every std::min/std::max call in this library
    // and its dependencies (stevensStringLib, PrintHelper, etc.).
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
#endif

// Standard library includes
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <unordered_map>
#include <string>
#include <utility>
#include <iostream>
#include <limits>

// Platform-specific includes for ncurses and system functions
#if defined(__linux__)
    #include <ncurses.h>
    #include <sys/ioctl.h>
    #include <unistd.h>
#elif defined(_WIN32) || defined(__MSDOS__)
    // PDCursesMod's public header is named curses.h (matching ncurses), not pdcurses.h
    #include <curses.h>
    #include <windows.h>
    // wingdi.h (pulled in by windows.h) #defines ERROR as a plain integer macro,
    // which collides with any enum/enum-class value literally named ERROR
    // (e.g. stevensSound's ErrorLevel::ERROR) anywhere later in this translation
    // unit. WIN32_LEAN_AND_MEAN does not exclude wingdi.h, so undef it here.
    #undef ERROR
#endif

// Library dependencies - bundled copies kept in sync with main libraries
#include <stevensStringLib.h>
#include <stevensMathLib.h>
#include <stevensMapLib.hpp>
#include <stevensVectorLib.hpp>
#include <stevensFileLib.hpp>

// Stevens Terminal component classes
#include "../classes/DisplayMode.hpp"
#include "../classes/PrintToken.hpp"
#include "../classes/PrintTokenStyling.hpp"
#include "../classes/PrintTokenHelper.hpp"
#include "Colors.hpp"   // Must be included before PrintHelper
#include "../classes/PrintHelper.hpp"
#include "../classes/WindowManager.hpp"

// Decorative glyphs, shared across every platform (real UTF-8, decoded
// correctly by ncursesw on Linux and by PDCursesMod's PDC_FORCE_UTF8 build
// on Windows). Card-suit CP437 codepage bytes were tried on Windows as a
// no-font-dependency alternative, but that only works for CP437's "real"
// high-range characters (0x80-0xFF) \u2014 these live at CP437's low/control-range
// byte positions (0x00-0x1F), which TrueType console fonts (Consolas
// included) never render as graphics (only the legacy bitmap "Terminal"/
// Raster Fonts font does that) \u2014 so it couldn't give real extensibility
// either. Individual glyph choices below just need to stick to Unicode
// blocks both Consolas and Ubuntu Mono actually cover (Mathematical
// Operators, Greek) \u2014 see BorderStyle.hpp's CORNER for a case (Geometric
// Shapes) that Consolas turned out not to cover.
#define ST_SPADE       "\xE2\x99\xA0"
#define ST_CLUB        "\xE2\x99\xA3"
#define ST_HEART       "\xE2\x99\xA5"
#define ST_DIAMOND     "\xE2\x99\xA6"
#define ST_APPROXEQUAL "\xE2\x89\x88"
#define ST_ASYMPEQUAL  "\xE2\x89\x83"
#define ST_DELTA       "\xCE\x94"


/**
 * @namespace stevensTerminal
 * @brief Main namespace for all stevensTerminal functionality
 *
 * This namespace contains all terminal manipulation functions, utilities,
 * and state management for the stevensTerminal library.
 */
namespace stevensTerminal
{
    /***** LIBRARY STATE VARIABLES *****/
    // TODO: Check if this can be removed. I believe it can. I don't think CultGame is using these display modes at all.
    extern std::map<std::string, DisplayMode> displayModes;

    //TODO check if we need currentDisplay mode
    extern std::string currentDisplayMode;
    extern bool textStyling;
    extern bool curses_colorCompatible;
    

    /***** FUNCTIONS *****/
    /**
     * @brief Get the global window manager instance
     * @return Reference to the singleton WindowManager instance
     */
    WindowManager& windowManager();


    /**
     * @brief Get current screen dimensions
     * @return Pair containing width and height of terminal in characters
     */
    std::pair<int, int> get_screen_size();


    /**
     * @brief Hide the terminal cursor.
     *
     * Call at the end of a view's render to keep the cursor invisible.
     * Some input flows re-enable the cursor via curs_set(1); calling hideCursor()
     * at the start or end of the next render restores the expected state.
     */
    void hideCursor();

    /**
     * @brief Show the terminal cursor (normal blinking line or block).
     *
     * Use in views where the cursor position is meaningful to the user,
     * such as world-map navigation where it marks the selected tile.
     */
    void showCursor();


    /**
     * @brief Initialize ncurses color support if available
     *
     * Checks if the terminal supports colors and initializes ncurses
     * color functionality. Must be called after initscr().
     */
    void curses_prepare_color();


    /**
     * @brief Set the title of the terminal window the program is running in.
     *
     * On Windows (PDCurses/PDCursesMod wincon port), sets the console host
     * window's title via PDC_set_title(). On Linux, emits the ANSI OSC escape
     * sequence that terminal emulators (xterm, gnome-terminal, konsole, etc.)
     * use to set their window title. Must be called after initscr().
     *
     * @param title The title to display.
     */
    void setTerminalTitle(const std::string & title);


    /**
     * @brief Initialize the stevensTerminal library
     *
     * This is the recommended way to initialize stevensTerminal. It handles:
     * - ncurses initialization (initscr)
     * - Color support setup
     * - Window manager initialization (optional)
     * - Display mode configuration
     *
     * @param initWindowManager If true, initializes the WindowManager with default windows
     * @param windowNames Custom window names for WindowManager (only used if initWindowManager is true)
     *
     * @example
     * // Simple initialization without window manager
     * stevensTerminal::initialize();
     *
     * // With default window manager
     * stevensTerminal::initialize(true);
     *
     * // With custom windows
     * stevensTerminal::initialize(true, {"header", "content", "footer"});
     */
    void initialize(bool initWindowManager = false,
                    const std::vector<std::string>& windowNames = {});

    /**
     * @brief Shutdown the stevensTerminal library
     *
     * This is the recommended way to clean up stevensTerminal resources.
     * It handles:
     * - Window manager cleanup
     * - ncurses termination (endwin)
     *
     * Call this before your program exits to ensure proper cleanup.
     *
     * @example
     * stevensTerminal::initialize();
     * // ... use the library ...
     * stevensTerminal::shutdown();
     */
    void shutdown();


    /**
     * @brief Check if screen size meets minimum display mode requirements
     * @param minSize Minimum required size (width, height)
     * @param screenSize Current screen size to check
     * @return true if screen size is greater than or equal to minimum
     */
    bool displayMode_GTEminSize(std::pair<int,int> minSize, std::pair<int,int> screenSize);


    /**
     * @brief Check if screen size is within maximum display mode requirements
     * @param maxSize Maximum allowed size (width, height)
     * @param screenSize Current screen size to check
     * @return true if screen size is less than or equal to maximum
     */
    bool displayMode_LTEmaxSize(std::pair<int,int> maxSize, std::pair<int,int> screenSize);


    /**
     * @brief Set display mode based on current screen size
     * @param screenSize Current terminal dimensions
     *
     * Automatically selects the appropriate display mode based on the
     * terminal size and updates currentDisplayMode accordingly.
     */
    void setDisplayMode(std::pair<int,int> screenSize);


    // NOTE: print() (plain std::cout ANSI-styling-era print function) was
    // removed — unreachable dead code. Use curses_wprint() instead.

} // namespace stevensTerminal

