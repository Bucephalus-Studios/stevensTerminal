/**
 * @file Styling.hpp
 * @brief Text styling and token manipulation functions for stevensTerminal
 * 
 * This header provides functions for manipulating style tokens, applying
 * text formatting, and managing styled string operations.
 * 
 * Part of the Stevens Terminal Library
 * Originally created July 2019
 */

#ifndef STEVENS_TERMINAL_STYLING_HPP
#define STEVENS_TERMINAL_STYLING_HPP

namespace stevensTerminal {

    /**
     * @brief Create a style token around a string with given styles
     * 
     * Given a string and some styles in an unordered map, create a token of the whole string which
     * associates the given styles to the string. When the string is then printed with a token printing
     * function from stevensTerminal, the styles will be applied to the printed text.
     * 
     * @param str The string to encapsulate with a style token
     * @param styleMap The styles to include in the token
     * @return The original string encapsulated with a style token that has the styles from the input styleMap associated with it
     */
    std::string addStyleToken(std::string str, std::unordered_map<std::string,std::string> styleMap) {
        // Read the contents of the style map into a string list of keys and values associated by '=' and delimited by ','
        std::string styleString = strlib::stringifyMap(styleMap, "=", ",");
        str = "{" + str + "}$[" + styleString + "]";
        return str;
    }

    /**
     * @brief Insert a style token into a string at the token's specified position
     * 
     * @param str The string we are inserting a style token into by reference
     * @param token The s_TerminalPrintToken object that we are inserting into str as an inline style directive
     */
    void insertStyleToken(std::string & str, const s_TerminalPrintToken & token) {
        // First check to see if the index that the token exists at is valid
        if( token.existsAtIndex >= str.length() ) {
            // If not, just return without doing anything
            return;
        }
        // Otherwise, begin by placing the left content bound character at the token's existsAtIndex
        str.insert(token.existsAtIndex, 1, '{');
        // To find the placement of the right content bound character, step over the length of the content from the existsAtIndex
        size_t indexOfRightContentBound = token.existsAtIndex + token.content.length() + 1;
        // If this would cause us to exceed the length of the string, just place the indexOfRightContentBound at the end of the string
        if(indexOfRightContentBound >= str.length() ) {
            indexOfRightContentBound = str.length();
        }
        // Get the directives to place in the style string
        std::string styleString = token.getStyleString();
        // Insert the right bound and the style directives
        std::string endOfToken = "}$[" + styleString + "]";
        str.insert(indexOfRightContentBound, endOfToken);
    }

    /**
     * @brief Remove all style tokenization from a string, leaving only content
     * 
     * Given a string potentially containing stevensTerminal style tokens, remove all of the characters
     * that constitute the styling directives for each token, leaving the content of the tokens in-tact.
     * 
     * Example: resultString = removeAllStyleTokenization("{{The quick brown fox}$[textColor=brown] jumps over the lazy dog.}$[textColor=purple,bgColor=yellow]")
     *          // resultString == "The quick brown fox jumps over the lazy dog."
     * 
     * @param str The string potentially containing stevensTerminal style tokens
     * @return The string str with all of its content untokenized by stevensTerminal style tokens
     */
    std::string removeAllStyleTokenization(std::string str) {
        // Find a token return its position in the string
        size_t tokenPosition = s_TerminalPrintTokenHelper::findToken(str);
        // If findToken return std::string::npos, then no token was found
        while( tokenPosition != std::string::npos ) {
            // Remove its braces and styling directives
            str = s_TerminalPrintTokenHelper::untokenize(str, tokenPosition);
            // Try to find the position of the next token
            tokenPosition = s_TerminalPrintTokenHelper::findToken(str);
        }
        return str;
    }

    /**
     * @brief Resize a styled string while preserving style tokens where possible
     * 
     * Given a string that may include style tokens, resize it in a very similar way to std::string.resize()
     * such that resizing the string does not necessarily damage or delete style tokens from the string.
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
     * @param str The styled string we are resizing
     * @param desiredLength The desired length of content we want the string str to have after resizing
     * @param fillChar Character to use for padding if growing the string
     * @return The styled string str resized to the desired length of content
     */
    std::string resizeStyledString(std::string str, const size_t desiredLength, const char fillChar = ' ') {
        // First, get all of the style tokens that the string contains
        std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintTokenHelper::getAllTokens(str);

        // If there are no style tokens, just return a call of the std::string.resize() function
        if(tokens.empty()) {
            str.resize(desiredLength, fillChar);
            return str;
        }

        // Otherwise, now that we have all of the style tokens, we take just their content and turn it into a string
        std::string contentString = stevensTerminal::removeAllStyleTokenization(str);
        // We resize this string
        contentString.resize(desiredLength, fillChar);
        std::string resizedStr = contentString;
        // We now place the style tokens back into this resized string
        for(int i = 0; i < tokens.size(); i++) {
            // For each style token, check to see if the index which it starts relative to the content and all other tokens
            // is still in bounds of the string's length
            if(tokens.at(i).existsAtIndex+1 < resizedStr.length()) {
                // If so, then we add the style token back at its original location
                stevensTerminal::insertStyleToken(resizedStr, tokens.at(i));
            } else {
                // Otherwise, all other style tokens will no longer be in bounds either, as they come after the current one
                // in order, so we break and finish
                break;
            }
        }

        // Return the resized styled string
        return resizedStr;
    }

    /**
     * @brief Print a horizontal border pattern across the terminal width
     * 
     * Given a pattern of characters in a string object, repeat them horizontally to cover the width of the terminal
     * by printing them with the main print function.
     * 
     * @param borderPattern A pattern of characters in a string object to repeat across the horizontal width
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
                               std::vector<std::string> bgColorVector = {}) {
        // Get the width of our current display setting
        int displayWidth = displayModes[currentDisplayMode].minSize.first;
        // If we have -1, meaning no set display boundary, just take the size of the terminal window
        if(displayWidth == -1) {
            displayWidth = get_screen_size().first-1;
        }

        print("\n");

        // Check to see if our horizontal border is going to be a single color/bgcolor or multiple colors/bgcolors
        if(colorVector.size() == 0 && bgColorVector.size() == 0) { // Single color/bgcolor
            // Gradually build up a string containing the full border that we will print once to the terminal
            std::string output = "";
            while(output.length() < displayWidth) {
                for(int i = 0; i < borderPattern.length(); i++) {
                    if(output.length() < displayWidth) {
                        output += borderPattern[i];
                    }
                }
            }
            print(output, {{"textColor",color}, {"bgColor",bgColor}});
        } else { // Multiple colors/bgcolors
            // Keep track of how many characters we have printed to the terminal for the border pattern
            int outputLength = 0;
            std::string colorToPrint = "default";
            std::string bgColorToPrint = "default";
            // Until the horizontal length of the screen is filled, we repeatedly print the pattern
            while(outputLength < displayWidth) {
                // Print each character of the border pattern out individually
                for(int i = 0; i < borderPattern.length(); i++) {
                    if(outputLength < displayWidth) {
                        // Get the text color to print
                        if((colorVector.size()-1) >= i) {
                            colorToPrint = colorVector[i];
                        } else {
                            colorToPrint = "default";
                        }
                        // Get the text background color to print
                        if((bgColorVector.size()-1) >= i) {
                            bgColorToPrint = bgColorVector[i];
                        } else {
                            bgColorToPrint = "default";
                        }
                        print(std::to_string(borderPattern[i]), {{"textColor",colorToPrint}, {"bgColor",bgColorToPrint}});
                        outputLength++;
                    }
                }
            }
        }

        print("\n");
    }

    /**
     * @brief Get the color string for a numeric value (bonus/penalty display)
     *
     * This is useful for displaying bonuses, penalties, and modifiers with appropriate coloring.
     * Positive values are green (good), negative values are red (bad), zero is neutral/default.
     *
     * @param value The numeric value to get a color for
     * @return "bright-green" for positive, "bright-red" for negative, "default" for zero
     */
    std::string getValueColor(float value)
    {
        if (value > 0.0f)
        {
            return "bright-green";
        }
        else if (value < 0.0f)
        {
            return "bright-red";
        }
        else
        {
            return "default";
        }
    }

} // namespace stevensTerminal

#endif // STEVENS_TERMINAL_STYLING_HPP