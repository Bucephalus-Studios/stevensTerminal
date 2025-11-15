/**
 * A class used by the stevensTerminalLibrary to hold information about tokenized strings
 * being printed using the print methods;
 */

#ifndef STEVENS_TERMINAL_PRINTTOKEN_H
#define STEVENS_TERMINAL_PRINTTOKEN_H

class s_TerminalPrintToken
{
    public:
        /*** Member variables ***/
        bool styled; //Is this tokenized bit of text being specifically styled? If so, true. False otherwise.
        int existsAtIndex; //The index of the source string that the token exists at.
        std::string content; //The string content of the token.
        std::string rawToken; //All of the token from the original string
        std::string textColor; //The color of the text in the token
        std::string bgColor; //The backgroud ncolor of the text in the token
        bool bold; //True if the text in the token is bold. False if otherwise.
        bool blink; //True if the text in the token should flash. False if otherwise.

        /*** Constructors ***/

        //Default
        s_TerminalPrintToken()
        {
            styled = false;
            existsAtIndex = -1;
            content = "";
            rawToken = "";
            textColor = "default";
            bgColor = "default";
            bold = false;
            blink = false;
        }


        //Parameters
        s_TerminalPrintToken(   std::string contentParam,
                                std::string rawTokenParam = "",
                                int existsAtIndexParam = -1,
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
        }

        
        /*** Methods ***/
        /**
         * @brief Gets a string indicating the styles that this particular s_TerminalPrintToken uses.
         *        This style string can be used within the inline style directive block of a tokenized
         *        string to style the text.
         */
        std::string getStyleString() const
        {
            std::string styleString;

            //textColor
            styleString += "textColor=" + textColor + ",";
            //bgColor
            styleString += "bgColor=" + bgColor;
            //blink
            if(blink)
            {
                styleString += ",blink=true";
            }
            else
            {
                styleString += ",blink=false";
            }
            //bold
            if(bold)
            {
                styleString += ",bold=true";
            }
            else
            {
                styleString += ",bold=false";
            }

            return styleString;
        }


        /**
         * @brief Inherit the styles of the parent token if this token is a child token.
         */
        void inheritStyle( const s_TerminalPrintToken & parentToken )
        {
            if(textColor == "default")
            {
                textColor = parentToken.textColor;
            }
            if(bgColor == "default")
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
        }


    private:


};

#endif // STEVENS_TERMINAL_PRINTTOKEN_H