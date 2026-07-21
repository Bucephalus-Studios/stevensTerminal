#pragma once
/**
 * @file PrintTokenHelper.hpp
 * @author Jeff Stevens jeff@bucephalusstudios.com
 * @brief Functions for manipulating and extracting data from strings containing
 *        stevensTerminal style tokens.  All token parsing delegates to TokenParser
 *        so brace-depth logic lives in exactly one place.
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 */

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include "TokenParser.hpp"


namespace stevensTerminal
{
namespace PrintTokenHelper
{

    /**
     * @brief Return the index of the first style token in str at or after startFindAt.
     *        Returns std::string::npos if no token is found.
     */
    inline size_t findToken(const std::string & str,
                            const size_t startFindAt = 0)
    {
        TokenParser::TokenSpan span = TokenParser::findNext(str, startFindAt);
        return span.valid ? span.start : std::string::npos;
    }


    /**
     * @brief Get the content section of a given raw token.
     *
     * Delegates to TokenParser::parseAt so nested tokens (e.g.
     * {label: {value}$[color]}$[color]) are handled correctly via
     * brace-depth matching rather than naive first-match searching.
     */
    inline std::string getRawTokenContent(const std::string & rawToken)
    {
        TokenParser::TokenSpan span = TokenParser::parseAt(rawToken, 0);
        if (!span.valid)
        {
            std::cerr << "stevensTerminal library error: In PrintTokenHelper::getRawTokenContent()\n"
                      << "Could not parse token in rawToken: " << rawToken << "\n";
            return "";
        }
        return TokenParser::getContent(rawToken, span);
    }


    /**
     * @brief Get the style string from a raw token (the part between '$[' and ']').
     */
    inline std::string getRawTokenStyleString(const std::string & rawToken)
    {
        TokenParser::TokenSpan span = TokenParser::parseAt(rawToken, 0);
        if (!span.valid)
        {
            std::cerr << "stevensTerminal library error: In PrintTokenHelper::getRawTokenStyleString()\n"
                      << "Could not parse token in rawToken: " << rawToken << "\n";
            return "";
        }
        return TokenParser::getStyle(rawToken, span);
    }


    /**
	 *  Given a std::string representing a token from a std::string intended to be printed with the stevensTerminal library,
	 *  parse the token and turn it into a PrintToken object to more easily work with its data.
	 *
	 * 	Parameter:
	 * 		std::string rawToken - A string representing a token from a string intended to be printed with the stevensTerminal library.
	 * 							   Example: "{The quick brown fox}$[textColor=red,bgColor=yellow,flash,bold]"
	 *
	 * 	Returns:
	 * 		PrintToken - An object that conveniently holds all of the data from the rawToken as member variables.
	 * 							   Example:
	 * 								{
	 * 									content 	: "The quick brown fox",
	 * 									textColor 	: "red",
	 * 									bgColor		: "yellow",
	 * 									flash		: true,
	 * 									bold		: true
	 * 								}
	 */
    inline PrintToken parseRawToken(std::string rawToken)
    {
        /*** Parse the content ***/
        std::string content = PrintTokenHelper::getRawTokenContent(rawToken);

        if (content.empty())
        {
            // Empty token content — return unstyled token
            return PrintToken("");
        }

        /*** Parse the style code ***/
        std::string styleString = PrintTokenHelper::getRawTokenStyleString(rawToken);
        std::unordered_map<std::string,std::string> styleMap = PrintTokenStyling::processPrintTokenStyle(styleString);

        PrintToken token = PrintToken();
        token.rawToken = rawToken;

        token.content   = content;
        token.textColor = styleMap["textColor"];
        token.bgColor   = styleMap["bgColor"];
        token.blink     = stevensStringLib::stringToBool(styleMap["blink"]);
        token.bold      = stevensStringLib::stringToBool(styleMap["bold"]);
        token.underline = stevensStringLib::stringToBool(styleMap["underline"]);
        token.reverse   = stevensStringLib::stringToBool(styleMap["reverse"]);
        token.dim       = stevensStringLib::stringToBool(styleMap["dim"]);
        token.italic    = stevensStringLib::stringToBool(styleMap["italic"]);
        token.styled    = true;

        return token;
    }


    /**
     * @brief Given a string and the position of a style token within it, return
     *        a PrintToken describing that token.  Throws if no valid token starts at tokenPosition.
     */
    inline PrintToken getToken(const std::string & str,
                               const size_t tokenPosition)
    {
        TokenParser::TokenSpan span = TokenParser::parseAt(str, tokenPosition);
        if (!span.valid)
        {
            throw std::invalid_argument(
                "PrintTokenHelper::getToken — could not find a token in \""
                + str + "\" at position " + std::to_string(tokenPosition));
        }

        std::string rawToken = TokenParser::getRaw(str, span);
        PrintToken token     = PrintTokenHelper::parseRawToken(rawToken);
        token.existsAtIndex  = span.start;
        token.rawToken       = rawToken;
        return token;
    }


    /**
     * @brief Return a vector of PrintToken objects for every top-level style token in str.
     *
     * Nested tokens are not returned separately — only the outermost token that
     * contains them is returned.  Advances past each token correctly so adjacent
     * tokens are never missed.
     */
    inline std::vector<PrintToken> getAllTokens(const std::string & str)
    {
        std::vector<PrintToken> tokens;
        size_t from = 0;
        while (from < str.length())
        {
            TokenParser::TokenSpan span = TokenParser::findNext(str, from);
            if (!span.valid) { break; }
            PrintToken token    = PrintTokenHelper::getToken(str, span.start);
            tokens.push_back(token);
            from = span.styleEnd + 1;
        }
        return tokens;
    }


    /**
     * @brief Remove the braces and styling directive from the token that starts at
     *        tokenPosition in str, leaving only the token's content in place.
     *
     * Example: untokenize("are you done with those {errands}$[textColor=red]?", 24)
     *          => "are you done with those errands?"
     */
    inline std::string untokenize(std::string str,
                                  const size_t tokenPosition)
    {
        PrintToken token = PrintTokenHelper::getToken(str, tokenPosition);
        str.replace(token.existsAtIndex, token.rawToken.length(), token.content);
        return str;
    }


    /**
     * @brief Wrap the substring of str starting at tokenPosition (length tokenLength)
     *        in a style token with the given styleString.
     */
    inline std::string tokenize(std::string str,
                                const size_t tokenPosition,
                                const size_t tokenLength = std::string::npos,
                                const std::string styleString = "textColor=default,bgColor=default")
    {
        std::string tokenContent = str.substr(tokenPosition, tokenLength);
        std::string rawToken     = "{" + tokenContent + "}$[" + styleString + "]";
        str.replace(tokenPosition, rawToken.length(), rawToken);
        return str;
    }


} // namespace PrintTokenHelper
} // namespace stevensTerminal