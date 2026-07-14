#pragma once
/**
 * A class used by the stevensTerminalLibrary to hold information about tokenized strings
 * being printed using the print methods;
 */


namespace stevensTerminal {

class PrintToken
{
    public:
        /*** Member variables ***/
        bool styled; //Is this tokenized bit of text being specifically styled? If so, true. False otherwise.
        size_t existsAtIndex; //The index of the source std::string that the token exists at. std::string::npos means unset.
        std::string content; //The string content of the token.
        std::string rawToken; //All of the token from the original string
        std::string textColor; //The color of the text in the token
        std::string bgColor; //The background color of the text in the token
        bool bold;      //True if the text in the token is bold. False if otherwise.
        bool blink;     //True if the text in the token should flash. False if otherwise.
        bool underline; //True if the text in the token is underlined. False otherwise.
        bool reverse;   //True if fg/bg colors are swapped for this token. False otherwise.
        bool dim;       //True if the text in the token is half-bright/dimmed. False otherwise.
        bool italic;    //True if the text in the token is italicized. False otherwise.

        /*** Constructors ***/

        //Default
        PrintToken()
        {
            styled = false;
            existsAtIndex = std::string::npos;
            content = "";
            rawToken = "";
            textColor = "default";
            bgColor = "default";
            bold = false;
            blink = false;
            underline = false;
            reverse = false;
            dim = false;
            italic = false;
        }


        //Parameters
        PrintToken(   std::string contentParam,
                                std::string rawTokenParam = "",
                                size_t existsAtIndexParam = std::string::npos,
                                std::string textColorParam = "default",
                                std::string bgColorParam = "default",
                                bool boldParam = false,
                                bool blinkParam = false,
                                bool styledParam = false)
        {
            content = contentParam;
            rawToken = rawTokenParam;
            existsAtIndex = existsAtIndexParam;
            textColor = textColorParam;
            bgColor = bgColorParam;
            bold = boldParam;
            blink = blinkParam;
            styled = styledParam;
            underline = false;
            reverse = false;
            dim = false;
            italic = false;
        }


        /*** Methods ***/
        /**
         * @brief Gets a std::string indicating the styles that this particular PrintToken uses.
         *        This style std::string can be used within the inline style directive block of a tokenized
         *        std::string to style the text.
         */
        std::string getStyleString() const
        {
            // Serialize only attributes that were actually set. An UNSPECIFIED attribute (empty
            // string) is omitted so it stays inheritable when this token is re-serialized and
            // re-parsed inside a parent; an explicitly-set value — including an explicit "default"
            // — is preserved. (blink/bold are bools, so only the "on" state is emitted.)
            std::string styleString;

            if(!textColor.empty())
            {
                styleString += "textColor=" + textColor;
            }
            if(!bgColor.empty())
            {
                if(!styleString.empty())
                {
                    styleString += ",";
                }
                styleString += "bgColor=" + bgColor;
            }
            if(blink)
            {
                if(!styleString.empty())
                {
                    styleString += ",";
                }
                styleString += "blink=true";
            }
            if(bold)
            {
                if(!styleString.empty())
                {
                    styleString += ",";
                }
                styleString += "bold=true";
            }
            if(underline)
            {
                if(!styleString.empty())
                {
                    styleString += ",";
                }
                styleString += "underline=true";
            }
            if(reverse)
            {
                if(!styleString.empty())
                {
                    styleString += ",";
                }
                styleString += "reverse=true";
            }
            if(dim)
            {
                if(!styleString.empty())
                {
                    styleString += ",";
                }
                styleString += "dim=true";
            }
            if(italic)
            {
                if(!styleString.empty())
                {
                    styleString += ",";
                }
                styleString += "italic=true";
            }

            return styleString;
        }


        /**
         * @brief Gets this PrintToken's styles as a map, suitable for passing directly to functions
         *        like stevensTerminal::style() that take a style map (e.g. {"textColor", "red"}).
         *
         *        Same "only actually-set attributes are included" semantics as getStyleString() (see
         *        that method's doc comment) - this is just the map form instead of the serialized
         *        "$[...]"-ready string form, for callers that want to build a style programmatically
         *        rather than round-trip through getStyleString() + a string parser.
         */
        std::unordered_map<std::string,std::string> getStyleMap() const
        {
            std::unordered_map<std::string,std::string> styleMap;

            if(!textColor.empty())
            {
                styleMap["textColor"] = textColor;
            }
            if(!bgColor.empty())
            {
                styleMap["bgColor"] = bgColor;
            }
            if(blink)
            {
                styleMap["blink"] = "true";
            }
            if(bold)
            {
                styleMap["bold"] = "true";
            }
            if(underline)
            {
                styleMap["underline"] = "true";
            }
            if(reverse)
            {
                styleMap["reverse"] = "true";
            }
            if(dim)
            {
                styleMap["dim"] = "true";
            }
            if(italic)
            {
                styleMap["italic"] = "true";
            }

            return styleMap;
        }


        /**
         * @brief Inherit the styles of the parent token if this token is a child token.
         *
         * Only UNSPECIFIED attributes (empty string) inherit from the parent. An attribute set to
         * an explicit value — including an explicit "default" — is the author's deliberate choice
         * and is left untouched, so a child can opt out of inheritance and stay terminal-default.
         */
        void inheritStyle( const PrintToken & parentToken )
        {
            if(textColor.empty())
            {
                textColor = parentToken.textColor;
            }
            if(bgColor.empty())
            {
                bgColor = parentToken.bgColor;
            }
            if(!blink)
            {
                blink = parentToken.blink;
            }
            if(!bold)
            {
                bold = parentToken.bold;
            }
            if(!underline)
            {
                underline = parentToken.underline;
            }
            if(!reverse)
            {
                reverse = parentToken.reverse;
            }
            if(!dim)
            {
                dim = parentToken.dim;
            }
            if(!italic)
            {
                italic = parentToken.italic;
            }
        }


    private:


};

} // namespace stevensTerminal
