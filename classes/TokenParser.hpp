#pragma once

#include <string>
#include <vector>

/**
 * @file TokenParser.hpp
 * @brief Single source of truth for parsing stevensTerminal style tokens.
 *
 * Style tokens have the form  {content}$[style]  where content may itself
 * contain nested tokens of the same form.  Every location in the library
 * that needs to locate a token, extract its content, or extract its style
 * string delegates here.  Brace-depth tracking is implemented once, in
 * parseAt(), and nowhere else.
 *
 * Depth rule: only the three-character sequence '}$[' (not a bare '}')
 * closes a depth level.  A '}' not immediately followed by "$[" is treated
 * as ordinary content.  This makes the parser lenient toward content that
 * happens to contain a lone '}' character.
 *
 * TODO: the same leniency does not exist for '{' — every '{' increases depth
 * and requires an eventual matching '}$[' closer, so a token whose *content*
 * is meant to contain a literal, non-nesting '{' will fail to parse (see the
 * July 2026 New Game art regression, where plain content got misidentified
 * as an unterminated nested token). Consider an escape sequence (e.g. "\{")
 * so callers can author a literal brace in content without it being treated
 * as the start of a nested token.
 *
 * State machine (per parseAt):
 *   Searching    — validate that str[pos] == '{'
 *   InContent    — scan content, tracking depth; '}$[' closes a level
 *   InStyle      — scan from first style char to closing ']'
 */

namespace stevensTerminal {
namespace TokenParser {


// ---------------------------------------------------------------------------
// Result type
// ---------------------------------------------------------------------------

/**
 * @brief Positional span of a single style token within a source string.
 *
 * All indices refer to the source string passed to parseAt() / findNext().
 * A default-constructed (or failed) span has valid == false and
 * std::string::npos for every index field.
 */
struct TokenSpan
{
    size_t start        = std::string::npos; ///< position of the opening '{'
    size_t contentStart = std::string::npos; ///< first content char (one past '{')
    size_t contentEnd   = std::string::npos; ///< position of the depth-matched '}'
    size_t styleStart   = std::string::npos; ///< first style char (one past '$[')
    size_t styleEnd     = std::string::npos; ///< position of the closing ']'
    bool   valid        = false;
};


// ---------------------------------------------------------------------------
// Internal detail
// ---------------------------------------------------------------------------

namespace detail {

/// True when str[pos] is the start of the closing sequence '}$['.
inline bool isClosingBrace(const std::string & str, size_t pos)
{
    return pos + 2 < str.length() &&
           str[pos]     == '}' &&
           str[pos + 1] == '$' &&
           str[pos + 2] == '[';
}

} // namespace detail


// ---------------------------------------------------------------------------
// Core parser
// ---------------------------------------------------------------------------

/**
 * @brief Parse the style token that begins at str[pos].
 *
 * str[pos] must be '{'.  Runs the InContent state (brace-depth tracking)
 * then the InStyle state (scan to ']').  Returns an invalid span if the
 * characters at pos do not form a complete, well-formed token.
 *
 * This is the single function that owns all brace-depth logic.
 */
inline TokenSpan parseAt(const std::string & str, size_t pos)
{
    TokenSpan span;

    if (pos >= str.length() || str[pos] != '{')
    {
        return span; // invalid: not at an opening brace
    }

    span.start        = pos;
    span.contentStart = pos + 1;

    // --- State: InContent ---
    // Track brace depth. Only '}$[' (not a bare '}') closes a depth level so
    // that lone '}' characters inside content are treated as plain text.
    int    depth = 1;
    size_t i     = pos + 1;
    const size_t len = str.length();

    while (i < len)
    {
        if (str[i] == '{')
        {
            depth++;
            i++;
        }
        else if (detail::isClosingBrace(str, i))
        {
            depth--;
            if (depth == 0)
            {
                span.contentEnd = i; // position of the depth-matched '}'
                i += 3;              // consume '}$[', land on first style char
                break;
            }
            i += 3; // inner '}$[' — skip without exiting
        }
        else
        {
            i++;
        }
    }

    if (span.contentEnd == std::string::npos)
    {
        return span; // never found the depth-matched closing brace
    }

    // --- State: InStyle ---
    // i is now positioned on the first style character (right after '$[').
    // Scan forward to the closing ']'.
    span.styleStart = i;
    size_t styleEnd = str.find(']', i);
    if (styleEnd == std::string::npos)
    {
        return span; // no closing ']' — malformed token
    }

    span.styleEnd = styleEnd;
    span.valid    = true;
    return span;
}


/**
 * @brief Scan str starting at `from` and return the span of the first
 *        valid style token found.  Returns an invalid span if none exists.
 */
inline TokenSpan findNext(const std::string & str, size_t from = 0)
{
    size_t pos = from;
    while (pos < str.length())
    {
        size_t openBrace = str.find('{', pos);
        if (openBrace == std::string::npos) { break; }

        TokenSpan span = parseAt(str, openBrace);
        if (span.valid) { return span; }

        pos = openBrace + 1; // this '{' didn't start a valid token; keep scanning
    }
    return TokenSpan{}; // invalid
}


/**
 * @brief Collect the spans of every top-level style token in str, in order.
 *
 * Nested tokens are not returned separately — only the outermost span that
 * contains them is returned.
 */
inline std::vector<TokenSpan> parseAll(const std::string & str)
{
    std::vector<TokenSpan> spans;
    size_t from = 0;
    while (from < str.length())
    {
        TokenSpan span = findNext(str, from);
        if (!span.valid) { break; }
        spans.push_back(span);
        from = span.styleEnd + 1;
    }
    return spans;
}


// ---------------------------------------------------------------------------
// Convenience extractors
// ---------------------------------------------------------------------------

/// Content string of the token: the substring between '{' and the
/// depth-matched '}'.  May itself contain nested tokens.
inline std::string getContent(const std::string & str, const TokenSpan & span)
{
    if (!span.valid || span.contentEnd <= span.contentStart) { return ""; }
    return str.substr(span.contentStart, span.contentEnd - span.contentStart);
}

/// Style string of the token: the substring between '$[' and ']'.
inline std::string getStyle(const std::string & str, const TokenSpan & span)
{
    if (!span.valid || span.styleEnd < span.styleStart) { return ""; }
    return str.substr(span.styleStart, span.styleEnd - span.styleStart);
}

/// The complete raw token: the substring from '{' through ']' inclusive.
inline std::string getRaw(const std::string & str, const TokenSpan & span)
{
    if (!span.valid) { return ""; }
    return str.substr(span.start, span.styleEnd - span.start + 1);
}


} // namespace TokenParser
} // namespace stevensTerminal