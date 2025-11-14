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

#ifndef STEVENS_TERMINAL_CORE_HPP
#define STEVENS_TERMINAL_CORE_HPP

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

// Stevens library dependencies
#include "libraries/stevensStringLib.h"
namespace strlib = stevensStringLib;

#include "libraries/stevensMathLib.h"

#include "libraries/stevensMapLib.hpp"
namespace maplib = stevensMapLib;

#include "libraries/stevensVectorLib.hpp"
#include "libraries/stevensFileLib.hpp"

// Stevens Terminal component classes
#include "classes/s_TerminalDisplayMode.h"
#include "classes/s_TerminalPrintToken.h"
#include "classes/s_TerminalPrintTokenStyling.hpp"
#include "classes/s_TerminalPrintTokenHelper.hpp"
#include "classes/s_TerminalPrintHelper.h"
namespace term_printhelper = s_TerminalPrintHelper;

#include "classes/WindowManager.hpp"

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

    // Core library state variables
    std::map<std::string, s_TerminalDisplayMode> displayModes = {
        {"very small", s_TerminalDisplayMode({-1,-1},{79,23},18)},
        {"small", s_TerminalDisplayMode({80,24},{119,39},18)},
        {"regular", s_TerminalDisplayMode({120,40},{-1,-1},24)}
    };
    
    std::string currentDisplayMode = "regular";
    bool textStyling = true;
    bool curses_colorCompatible = false;
    
    /**
     * @brief Get the global window manager instance
     * @return Reference to the singleton WindowManager instance
     */
    inline WindowManager& windowManager() {
        static WindowManager instance;
        return instance;
    }

    /**
     * @brief Get current screen dimensions
     * @return Pair containing width and height of terminal in characters
     */
    #if defined(_WIN32)
    std::pair<int, int> get_screen_size() {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        return std::pair<int, int>(csbi.srWindow.Right - csbi.srWindow.Left + 1,
                                   csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
    }
    #elif defined(__linux__)
    std::pair<int, int> get_screen_size() {
        struct winsize size;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
        return std::pair<int, int>(size.ws_col, size.ws_row);
    }
    #endif

    #ifdef curses
    /**
     * @brief Initialize ncurses color support if available
     * 
     * Checks if the terminal supports colors and initializes ncurses
     * color functionality. Must be called after initscr().
     */
    void curses_prepare_color() {
        if(has_colors()) {
            start_color();
            // Set up Curses color codes
            s_TerminalPrintHelper::curses_setup_colorCodes();
            // Set up Curses colorpairs
            s_TerminalPrintHelper::curses_setup_colorPairs();
            curses_colorCompatible = true;
            textStyling = true;
        } else {
            curses_colorCompatible = false;
            textStyling = false;
        }
    }
    #endif

    /**
     * @brief Check if screen size meets minimum display mode requirements
     * @param minSize Minimum required size (width, height)
     * @param screenSize Current screen size to check
     * @return true if screen size is greater than or equal to minimum
     */
    bool displayMode_GTEminSize(std::pair<int,int> minSize, std::pair<int,int> screenSize) {
        // Implementation will be moved from main header
        if(minSize.first == -1 && minSize.second == -1) {
            return true;
        }
        if(minSize.first == -1) {
            return screenSize.second >= minSize.second;
        }
        if(minSize.second == -1) {
            return screenSize.first >= minSize.first;
        }
        return (screenSize.first >= minSize.first && screenSize.second >= minSize.second);
    }

    /**
     * @brief Check if screen size is within maximum display mode requirements
     * @param maxSize Maximum allowed size (width, height)
     * @param screenSize Current screen size to check
     * @return true if screen size is less than or equal to maximum
     */
    bool displayMode_LTEmaxSize(std::pair<int,int> maxSize, std::pair<int,int> screenSize) {
        // Implementation will be moved from main header
        if(maxSize.first == -1 && maxSize.second == -1) {
            return true;
        }
        if(maxSize.first == -1) {
            return screenSize.second <= maxSize.second;
        }
        if(maxSize.second == -1) {
            return screenSize.first <= maxSize.first;
        }
        return (screenSize.first <= maxSize.first && screenSize.second <= maxSize.second);
    }

    /**
     * @brief Set display mode based on current screen size
     * @param screenSize Current terminal dimensions
     * 
     * Automatically selects the appropriate display mode based on the
     * terminal size and updates currentDisplayMode accordingly.
     */
    void setDisplayMode(std::pair<int,int> screenSize) {
        for(auto const& mode : displayModes) {
            if(displayMode_GTEminSize(mode.second.minSize, screenSize) && 
               displayMode_LTEmaxSize(mode.second.maxSize, screenSize)) {
                currentDisplayMode = mode.first;
                return;
            }
        }
        // If no mode matches, default to "regular"
        currentDisplayMode = "regular";
    }

    // Forward declaration of main print function
    void print(std::string input,
               std::unordered_map<std::string, std::string> style = {
                   {"textColor", "default"},
                   {"bgColor", "default"},
                   {"flash", "false"},
                   {"bold", "false"}
               },
               std::unordered_map<std::string, std::string> format = {
                   {"wrap", "true"},
                   {"preserve newlines on wrap", "false"}
               });

} // namespace stevensTerminal

#endif // STEVENS_TERMINAL_CORE_HPP