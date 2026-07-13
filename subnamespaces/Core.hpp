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
#include <string_view>
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

/**
 * @namespace stevensTerminal::glyph
 * @brief Decorative glyphs, shared across every platform (real UTF-8, decoded
 * correctly by ncursesw on Linux and by PDCursesMod's PDC_FORCE_UTF8 build
 * on Windows).
 *
 * Card-suit CP437 codepage bytes were tried on Windows as a no-font-
 * dependency alternative, but that only works for CP437's "real" high-range
 * characters (0x80-0xFF) - these live at CP437's low/control-range byte
 * positions (0x00-0x1F), which TrueType console fonts (Consolas included)
 * never render as graphics (only the legacy bitmap "Terminal"/Raster Fonts
 * font does that) - so it couldn't give real extensibility either.
 *
 * Each glyph below has been empirically confirmed present in Consolas via
 * GetFontUnicodeRanges() - block-level assumptions are not reliable (e.g.
 * U+2243 fails despite its neighbor U+2248 in the same Mathematical
 * Operators block working; see BorderStyle.hpp's CORNER for a Geometric
 * Shapes-block example of the same trap). Ubuntu Mono coverage has not yet
 * been machine-verified the same way.
 *
 * Plain std::string_view (not a macro) so these live in stevensTerminal's
 * namespace instead of the global preprocessor namespace, and so a typo'd
 * name is a compile error at the point of use instead of silently expanding
 * to nothing.
 */
/**
 * @namespace stevensTerminal::key
 * @brief Named constants for special input key strings used as cg_response selectStr values.
 *
 * Use these instead of raw character literals so call sites are readable and
 * consistent. These are plain std::string constants (not ncurses KEY_* integers)
 * and are safe to include from any header.
 *
 * Arrow keys are intentionally excluded: they require ncurses KEY_UP/DOWN/LEFT/RIGHT
 * integer constants (platform-specific), which belong at call sites via
 * std::to_string(KEY_UP) etc., typically in "hidden" response sections.
 */
namespace stevensTerminal::key
{
    inline const std::string Esc = std::string(1, '\x1b'); // ASCII 27 — displayed as "Esc" by ResponseHelper
}


namespace stevensTerminal::glyph
{
    inline constexpr std::string_view spade       = "\xE2\x99\xA0"; // U+2660 BLACK SPADE SUIT
    inline constexpr std::string_view club        = "\xE2\x99\xA3"; // U+2663 BLACK CLUB SUIT
    inline constexpr std::string_view heart       = "\xE2\x99\xA5"; // U+2665 BLACK HEART SUIT
    inline constexpr std::string_view diamond     = "\xE2\x99\xA6"; // U+2666 BLACK DIAMOND SUIT
    inline constexpr std::string_view approxEqual = "\xE2\x89\x88"; // U+2248 ALMOST EQUAL TO
    inline constexpr std::string_view delta       = "\xCE\x94";     // U+0394 GREEK CAPITAL LETTER DELTA

    // Added while auditing item/UI symbols against Consolas coverage.
    inline constexpr std::string_view upTriangle       = "\xE2\x96\xB2"; // U+25B2 BLACK UP-POINTING TRIANGLE
    inline constexpr std::string_view smallSquare      = "\xE2\x96\xAA"; // U+25AA BLACK SMALL SQUARE
    inline constexpr std::string_view rectangle        = "\xE2\x96\xAC"; // U+25AC BLACK RECTANGLE
    inline constexpr std::string_view filledCircle     = "\xE2\x97\x8F"; // U+25CF BLACK CIRCLE
    inline constexpr std::string_view rightHalfBlock    = "\xE2\x96\x90"; // U+2590 RIGHT HALF BLOCK
    inline constexpr std::string_view sixPointedStar   = "\xE2\x9C\xB6"; // U+2736 SIX POINTED BLACK STAR
    inline constexpr std::string_view spinnerUp        = "\xE2\x96\xB4"; // U+25B4 BLACK UP-POINTING SMALL TRIANGLE
    inline constexpr std::string_view spinnerRight     = "\xE2\x96\xB8"; // U+25B8 BLACK RIGHT-POINTING SMALL TRIANGLE
    inline constexpr std::string_view spinnerDown      = "\xE2\x96\xBE"; // U+25BE BLACK DOWN-POINTING SMALL TRIANGLE
    inline constexpr std::string_view spinnerLeft      = "\xE2\x97\x82"; // U+25C2 BLACK LEFT-POINTING SMALL TRIANGLE
}


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
     * @brief Resize the terminal window itself (not just curses' idea of the
     * screen) to the given dimensions, so the game doesn't open into a
     * console smaller than its UI expects.
     *
     * On Windows (PDCurses/PDCursesMod wincon port), calls resize_term(),
     * which directly resizes the actual Win32 console window via
     * SetConsoleWindowInfo()/SetConsoleScreenBufferSize() - this only works
     * after initscr() has been called (PDCursesMod's Windows backend can't
     * do it beforehand). The requested size is silently clamped to whatever
     * GetLargestConsoleWindowSize() allows for the user's screen/font, so
     * asking for more than fits is safe.
     *
     * On Linux, terminal emulator windows aren't something a program can
     * resize this way (the user's terminal emulator owns that) - this is a
     * no-op there; players are expected to size their terminal manually
     * (see CLAUDE.md's terminal size requirements).
     *
     * @param rows Desired terminal height in rows.
     * @param cols Desired terminal width in columns.
     */
    void resizeTerminalWindow(int rows, int cols);


    /**
     * @brief Make the A_BLINK text attribute actually blink on Windows.
     *
     * On Windows (PDCurses/PDCursesMod wincon port), A_BLINK defaults to
     * setting the background color to high intensity instead of blinking
     * (PDC_set_blink(FALSE) is PDCursesMod's default on most platforms) -
     * so blink=true text just renders with a bright background and never
     * flashes until this is called. Must be called after initscr() (and
     * after curses_prepare_color(), since PDC_set_blink() reinitializes
     * COLORS when color support is already started).
     *
     * On Linux/macOS, real terminals already treat A_BLINK as a true blink
     * via terminfo - this is a no-op there.
     */
    void enableTrueBlink();


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

