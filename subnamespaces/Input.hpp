#pragma once
/**
 * @file Input.hpp
 * @brief Input handling and validation functions for stevensTerminal
 *
 * This header provides functions for user input, input validation,
 * and input stream management.
 *
 * Part of the Stevens Terminal Library
 * Originally created July 2019
 */

#include <string>
#include <unordered_map>
#include <cstddef>

#if defined(__linux__)
    #include <ncurses.h>
#elif defined(_WIN32) || defined(__MSDOS__)
    #include <curses.h>
#endif

namespace stevensTerminal
{
//TODO clearInput is a function that does not work with curses, as it deals with cin and cout. What should we do with all of our non-curses
    //cin/cout functions? They are useful, but they don't feel like they lock into place with the other curses functions here which are the star
    //of the show. My first idea is to create a new library or namespace used for the non curses functions, but I'm up for suggestions, Claude.
    /**
     * @brief Clear the input stream buffer
     *
     * Clears the cin buffer to prevent input injections.
     * Not used with getline operations.
     */
    void clearInput();


    /**
     * @brief Get std::string input from user with validation
     *
     * User input function used throughout the application for accepting strings as input.
     * Returns the strings that the player inputs via getline, refuses strings of complete
     * whitespace or empty strings.
     *
     * @return std::string The validated user input string
     */
    std::string input();


    /**
     * @brief Get integer input from user within a specified range
     *
     * User input function used throughout the application for accepting integer input.
     * Keeps a loop going until the desired type of input is accepted.
     *
     * @param responseRange The total number of responses inclusive that will exist for inputting the integer
     *                      (passing 2 in means that there will be 2 choices: 0, 1, 2)
     * @return int The validated integer input from the user
     */
    int input(int responseRange);

    /**
     * @brief Check if input std::string is a valid number within response range
     *
     * Given a std::string input, determine if it is a number that exists within
     * the size range of possible responses. If the std::string input is determined
     * to not be a number, then we return false.
     *
     * @param input A std::string value that potentially exists as a number in the response range or not
     * @param responseRange The number of total responses. The range is effectively from 1 to the responseRange passed in.
     * @return bool True if the input std::string is a number and exists within a range of 1 to the response range inclusive.
     *              False in all other situations.
     */
    bool inputWithinResponseRange(std::string input, int responseRange);


    /**
     * @brief Read a line of free-form text from a bordered ncurses window.
     *
     * Manages cursor visibility, clears the inner area on each keystroke to
     * eliminate color bleed from previously-rendered window content, and
     * wraps the cursor at the right border. When text would push the cursor
     * into the bottom border the window grows via wresize; the caller's next
     * full re-render will restore any border decoration.
     *
     * @param win       Window to read from and render into.
     * @param maxLen    Maximum characters accepted (SIZE_MAX = unlimited).
     * @param textStyle Attribute map applied to the typed text.
     * @return          The submitted string, or "\x1b" if the user pressed ESC.
     */
    std::string input(
        WINDOW * win,
        size_t maxLen = SIZE_MAX,
        const std::unordered_map<std::string,std::string> & textStyle = {}
    );

} // namespace stevensTerminal
