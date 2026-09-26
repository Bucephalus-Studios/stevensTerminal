#pragma once

#include <string>
#include <unordered_map>

/**
 * A class used by the stevensTerminal library to hold a typed set of the styling attributes
 * style()'s string-keyed styleMap accepts (mirrors PrintToken's style fields). Exists so callers
 * building a style get named, typed members instead of a loosely-keyed map of magic strings.
 *
 * A default-constructed StyleStruct is unstyled (isSet() == false).
 */

namespace stevensTerminal {

class StyleStruct
{
    public:
        /*** Member variables ***/
        // Default member initializers (not a user-declared constructor) so StyleStruct stays an
        // aggregate -- callers can use designated-initializer syntax, e.g.
        // StyleStruct{.textColor = "bright-yellow"}.
        std::string textColor = ""; //The color of the text
        std::string bgColor = "";   //The background color of the text
        bool bold      = false; //True if the text should be bold
        bool blink     = false; //True if the text should flash
        bool underline = false; //True if the text should be underlined
        bool reverse   = false; //True if fg/bg colors should be swapped
        bool dim       = false; //True if the text should be half-bright/dimmed
        bool italic    = false; //True if the text should be italicized

        /*** Methods ***/

        /**
         * @brief True if any attribute has been set away from its default (unstyled) value.
         */
        bool isSet() const
        {
            return !textColor.empty() || !bgColor.empty() ||
                   bold || blink || underline || reverse || dim || italic;
        }

        /**
         * @brief Converts to the string-keyed map style() expects. Only non-default attributes
         *        are included.
         */
        std::unordered_map<std::string, std::string> toStyleMap() const
        {
            std::unordered_map<std::string, std::string> map;
            if(!textColor.empty()) { map["textColor"] = textColor; }
            if(!bgColor.empty())   { map["bgColor"]   = bgColor; }
            if(bold)               { map["bold"]      = "true"; }
            if(blink)              { map["blink"]     = "true"; }
            if(underline)          { map["underline"] = "true"; }
            if(reverse)            { map["reverse"]   = "true"; }
            if(dim)                { map["dim"]       = "true"; }
            if(italic)             { map["italic"]    = "true"; }
            return map;
        }
};

} // namespace stevensTerminal
