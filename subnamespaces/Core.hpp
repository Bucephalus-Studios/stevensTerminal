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
    #include <pdcurses.h>
    #include <windows.h>
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

// Platform-specific character definitions
#if defined(_WIN32) || defined(__MSDOS__)
    #define ST_SPADE   "\x06"
    #define ST_CLUB    "\x05" 
    #define ST_HEART   "\x03"
    #define ST_DIAMOND "\x04"
#else
    #define ST_SPADE   "\xE2\x99\xA0"
    #define ST_CLUB    "\xE2\x99\xA3"
    #define ST_HEART   "\xE2\x99\xA5"
    #define ST_DIAMOND "\xE2\x99\xA6"
    #define ST_APPROXEQUAL "\xE2\x89\x88"
    #define ST_ASYMPEQUAL "\xE2\x89\x83"
    #define ST_DELTA "\u0394"
#endif


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


    #ifdef curses
    /**
     * @brief Initialize ncurses color support if available
     *
     * Checks if the terminal supports colors and initializes ncurses
     * color functionality. Must be called after initscr().
     */
    void curses_prepare_color();
    #endif


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


    // Forward declaration of main print function
    void print(std::string input,
               std::unordered_map<std::string, std::string> style = {
                   {"textColor", "default"},
                   {"bgColor", "default"},
                   {"blink", "false"},
                   {"bold", "false"}
               },
               std::unordered_map<std::string, std::string> format = {
                   {"wrap", "true"},
                   {"preserve newlines on wrap", "false"}
               });

} // namespace stevensTerminal

