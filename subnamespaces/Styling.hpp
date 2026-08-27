#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
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
     * @brief Style key used to request a style macro (see styleMacros below)
     */
    inline const std::string styleMacroKey = "styleMacro";

    /**
     * @brief Registry of style macros: named, code-driven styling procedures a style map can request
     *
     * A "style macro" is to style() what a CSS class is to inline styles - a single named key that
     * expands into a more complex styling procedure (e.g. per-character coloring) instead of a
     * literal key=value attribute. Request one by including {styleMacroKey, "<name>"} in a styleMap
     * passed to style(); style() looks up <name> here and, if found, delegates entirely to that
     * macro's function instead of building a plain token.
     *
     * Register a new macro by adding an entry here plus its dedicated helper function (see
     * styleRandomTextColorPerCharacter() below for the pattern).
     */
    extern const std::unordered_map<std::string, std::function<std::string(const std::string&)>> styleMacros;

    /**
     * @brief Wrap a std::string in a style token with the given styles
     *
     * Given a std::string and some styles in an unordered map, create a token of the whole std::string which
     * associates the given styles to the string. When the std::string is then printed with a token printing
     * function from stevensTerminal, the styles will be applied to the printed text.
     *
     * If styleMap contains styleMacroKey ("styleMacro"), its value is looked up in styleMacros and,
     * if found, str is passed to that macro's function instead of building a plain token (other keys
     * in styleMap are ignored in that case - the macro takes over entirely).
     *
     * @param str The std::string to style
     * @param styleMap The styles to apply (e.g. {{"textColor","bright-magenta"},{"bgColor","black"}}),
     *                 or a style macro request (e.g. {{"styleMacro","randomTextColorPerCharacter"}})
     * @return str wrapped in a style token carrying the given styles, or the result of the requested macro
     */
    std::string style(std::string str,
                      std::unordered_map<std::string,std::string> styleMap);


    /**
     * @brief Style macro: wrap each character of str in its own independently random text color
     *
     * Splits str into individual codepoints (UTF-8 safe - a multi-byte character is kept whole,
     * never split mid-character) and wraps each one in its own style() token with a color picked
     * at random (with replacement) from colorPool. Useful for a "rainbow"/celebratory text effect,
     * e.g. a "Randomized!" toast message. Registered in styleMacros under the name
     * "randomTextColorPerCharacter".
     *
     * @param str The plain-text std::string to style (should not already contain style tokens)
     * @param colorPool Map of candidate color names to numeric color codes to pick from at random
     *                  per character. If empty (the default), uses stevensTerminal::Colors::curses_colors
     *                  with "black" excluded (black text would be invisible against the default background).
     * @return str with every character wrapped in its own randomly-colored style token
     */
    std::string styleRandomTextColorPerCharacter(std::string str,
                                                 std::unordered_map<std::string,int> colorPool = {});


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
     * @param truncationSuffix Appended when truncating (e.g. "..."), so a cut-off value reads as
     *        truncated rather than merely short. Ignored when padding. Counted against
     *        desiredLength: if the suffix alone is as long as desiredLength, it's dropped and the
     *        cut falls back to plain truncation rather than emitting only the suffix. The suffix
     *        itself is appended as plain text, after any style tokens have been reinserted.
     * @return The styled std::string str resized to the desired length of content
     */
    std::string resizeStyledString(std::string str, const size_t desiredLength, const char fillChar = ' ',
                                   const std::string & truncationSuffix = "");


    // NOTE: printHorizontalBorder() was removed — unreachable dead code. See
    // stevensTerminal.cpp.


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
