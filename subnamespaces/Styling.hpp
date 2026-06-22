#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "../classes/PrintToken.hpp"
#include <sstream>


/**
 * @file Styling.hpp
 * @brief Text styling and token manipulation functions for stevensTerminal
 *
 * This header provides functions for manipulating style tokens, applying
 * text formatting, and managing styled std::string operations.
 *
 * Part of the Stevens Terminal Library
 * Originally created July 2019
 */


namespace stevensTerminal
{
    /**
     * @brief Create a style token around a std::string with given styles
     *
     * Given a std::string and some styles in an unordered map, create a token of the whole std::string which
     * associates the given styles to the string. When the std::string is then printed with a token printing
     * function from stevensTerminal, the styles will be applied to the printed text.
     *
     * @param str The std::string to encapsulate with a style token
     * @param styleMap The styles to include in the token
     * @return The original std::string encapsulated with a style token that has the styles from the input styleMap associated with it
     */
    std::string addStyleToken(std::string str,
                              std::unordered_map<std::string,std::string> styleMap);


    /**
     * @brief Wrap a std::string in a style token with the given styles. Cleaner-named alias for
     *        addStyleToken(), e.g. style(text, {{"textColor","bright-magenta"}}).
     *
     * @param str The std::string to style
     * @param styleMap The styles to apply (e.g. {{"textColor","bright-magenta"},{"bgColor","black"}})
     * @return str wrapped in a style token carrying the given styles
     */
    std::string style(std::string str,
                      std::unordered_map<std::string,std::string> styleMap);


    /**
     * @brief Insert a style token into a std::string at the token's specified position
     *
     * @param str The std::string we are inserting a style token into by reference
     * @param token The PrintToken object that we are inserting into str as an inline style directive
     */
    void insertStyleToken(std::string & str,
                          const PrintToken & token);


    /**
     * @brief Remove all style tokenization from a string, leaving only content
     *
     * Given a std::string potentially containing stevensTerminal style tokens, remove all of the characters
     * that constitute the styling directives for each token, leaving the content of the tokens in-tact.
     *
     * Example: resultString = removeAllStyleTokenization("{{The quick brown fox}$[textColor=brown] jumps over the lazy dog.}$[textColor=purple,bgColor=yellow]")
     *          // resultString == "The quick brown fox jumps over the lazy dog."
     *
     * @param str The std::string potentially containing stevensTerminal style tokens
     * @return The std::string str with all of its content untokenized by stevensTerminal style tokens
     */
    std::string removeAllStyleTokenization(std::string str);


    /**
     * @brief Resize a styled std::string while preserving style tokens where possible
     *
     * Given a string that may include style tokens, resize it in a very similar way to std::string.resize()
     * such that resizing the std::string does not necessarily damage or delete style tokens from the string.
     *
     * Example:
     *   input = "{Eggs and }$[textcolor=black]{spam}$[textcolor=red]";
     *   int desiredLength = 4;
     *   resizeStyledString(input, 4);
     *   output == "{Eggs}$[textColor=black]"
     *
     * Essentially, this function resizes the content contained within the style tokens. If the resize removes
     * all of the content that a style token would contain, it is then removed from the string.
     *
     * @param str The styled std::string we are resizing
     * @param desiredLength The desired length of content we want the std::string str to have after resizing
     * @param fillChar Character to use for padding if growing the std::string
     * @return The styled std::string str resized to the desired length of content
     */
    std::string resizeStyledString(std::string str, const size_t desiredLength, const char fillChar = ' ');


    /**
     * @brief Print a horizontal border pattern across the terminal width
     *
     * Given a pattern of characters in a std::string object, repeat them horizontally to cover the width of the terminal
     * by printing them with the main print function.
     *
     * @param borderPattern A pattern of characters in a std::string object to repeat across the horizontal width
     * @param color Optional name of the color to print the pattern in
     * @param bgColor Optional name of the background color to print the pattern in
     * @param colorVector Optional vector parameter that allows you to decide the color for each index of the border pattern.
     *                    For example, the border pattern "~-" could be colored an alternating yellow and green with a multicolor
     *                    parameter of ["yellow","green"]. A defined multicolor parameter always overrides any passed-in color parameter.
     * @param bgColorVector Optional vector parameter that allows you to decide the background color for each index of the border pattern.
     *                      Functions similarly to the colorVector parameter.
     */
    void printHorizontalBorder(std::string borderPattern,
                               std::string color = "default",
                               std::string bgColor = "default",
                               std::vector<std::string> colorVector = {},
                               std::vector<std::string> bgColorVector = {});

    
    // /**
    //  * @brief Get the color std::string for a numeric value (bonus/penalty display)
    //  *
    //  * This is useful for displaying bonuses, penalties, and modifiers with appropriate coloring.
    //  * Positive values are green (good), negative values are red (bad), zero is neutral/default.
    //  *
    //  * @param value The numeric value to get a color for
    //  * @return "bright-green" for positive, "bright-red" for negative, "default" for zero
    //  */
    // std::string getValueColor(float value)
    // {
    //     if (value > 0.0f)
    //     {
    //         return "bright-green";
    //     }
    //     else if (value < 0.0f)
    //     {
    //         return "bright-red";
    //     }
    //     else
    //     {
    //         return "default";
    //     }
    // }

} // namespace stevensTerminal
