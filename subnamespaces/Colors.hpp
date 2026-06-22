#pragma once
/**
 * @file Colors.hpp
 * @brief Utilities for working with ncurses color pairs
 * @version 2.1
 * @date 2026-03-07
 *
 * Provides centralized color management for stevensTerminal, including:
 * - Color pair setup and registration
 * - Color code mapping
 * - Color pair lookup functions
 */


#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

namespace stevensTerminal
{

namespace Colors {

// Curses color maps
extern std::unordered_map<std::string, int> curses_colorPairs;  // Map of color pair names to pair numbers (e.g., "red_on_black" -> 1)
extern std::unordered_map<std::string, int> curses_colors;      // Map of color names to ncurses color constants (e.g., "red" -> COLOR_RED)
const std::string curses_default_textColor = "bright-white";
const std::string curses_default_backgroundColor = "black";

/**
 * @brief Detects how many curses colors the terminal can display and populates curses_colorCodes map
 */
void curses_setup_colorCodes();

/**
 * @brief Sets up all color pairs in a data-driven way
 *
 * Creates color pairs for all combinations of foreground and background colors.
 * Color pair names follow the pattern: "foreground_on_background"
 * (e.g., "red_on_black", "bright-green_on_blue")
 */
void curses_setup_colorPairs();


/**
 * @brief Converts an ncurses color number to its std::string name
 * @param colorNum The color number (0-7 for standard colors, 8-15 for bright colors)
 * @return String name of the color (e.g., "yellow", "red", "bright-green")
 *
 * Performs a reverse lookup in the curses_colors map.
 */
std::string colorToString(int colorNum);


/**
 * @brief Looks up a color pair number by combining fg and bg colors
 * @param fgColor Foreground COLOR_ constant
 * @param bgColor Background COLOR_ constant
 * @return Color pair number for this combination, or 0 if not found
 */
int lookupColorPair(int fgColor, int bgColor);

/**
 * @brief Looks up a color pair number by color name strings (e.g. "black", "green")
 * @param fgName Foreground color name (must match a key in curses_colors)
 * @param bgName Background color name (must match a key in curses_colors)
 * @return Color pair number, or 0 (default) if not found
 */
int lookupColorPairByName(const std::string& fgName, const std::string& bgName);

/**
 * @brief Extracts the foreground color from a color pair number
 * @param colorPairNum The ncurses color pair number
 * @return The foreground color (0-15)
 */
short extractForegroundColor(int colorPairNum);

/**
 * @brief Extracts the background color from a color pair number
 * @param colorPairNum The ncurses color pair number
 * @return The background color (0-15)
 */
short extractBackgroundColor(int colorPairNum);

} // namespace Colors

} // namespace stevensTerminal

