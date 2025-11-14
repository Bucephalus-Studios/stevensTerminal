/**
 * @file s_TerminalTokenHelper.hpp
 * @author Jeff Stevens jeff@bucephalusstudios.com
 * @brief Contains functions for manipulation and obtaining data for strings containing stevensTerminal style tokens.
 * @version 0.1
 * @date 2024-03-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */


namespace s_TerminalPrintTokenHelper
{

    /**
     * @brief Given a string containing stevensTerminal style tokens, returns the index position of the first token in the string.
     *        If no tokens are found, returns std::string::npos.
     * 
     * @param str The string containing stevensTerminal style tokens.
     * @param startFindAt The index position in str to begin looking for the first token at and after.
     * 
     * @return size_t The position of the first token in the string, 
     */
    size_t findToken(   const std::string & str,
                        const size_t startFindAt = 0    )
    {
        int styled_tokenFindStage = 0; //The stage we are at in the process of trying to find a token in the string
        size_t styled_startIndex = startFindAt; //The index which the token begins at
        int braceDepth = 0; //Track nesting depth of curly braces for proper nested token support

        for(int i = startFindAt; i < str.length(); i++)
		{
			switch(styled_tokenFindStage)
			{
				//Search for an open curly brace
				case 0:
					if(str[i] == '{')
					{
						styled_tokenFindStage = 1;
						styled_startIndex = i;
						braceDepth = 1; //Start tracking depth from this opening brace
					}
					break;
				//Search for the matching closed curly brace (accounting for nesting)
				case 1:
					if(str[i] == '{')
					{
						braceDepth++; //Found nested opening brace, increase depth
					}
					else if(str[i] == '}')
					{
						braceDepth--; //Found closing brace, decrease depth
						if(braceDepth == 0)
						{
							//Found the matching closing brace for our token
							styled_tokenFindStage = 2;
						}
					}
					break;
				//Is there a dollar sign directly after the closed curly brace?
				case 2:
					if(str[i] == '$')
					{
						//We may have found a token...
						styled_tokenFindStage = 3;
					}
					else
					{
						//If not, we're still not sure if we've found a token yet
						styled_tokenFindStage = 1;
					}
					break;
				//Is there an open square bracket directly after the dollar sign?
				case 3:
					if(str[i] == '[')
					{
						//We may have found a token...
						styled_tokenFindStage = 4;
					}
					else
					{
						//If not, we haven't found a token
						styled_tokenFindStage = 1;
					}
					break;
				//If so, is there a closed square bracket after the opened square bracket?
				case 4:
					if(str[i] == ']')
					{
                        //Found a complete token, now we just return its starting position
						return styled_startIndex;
					}
					break;
				default:
					break;
			}
        }
        //If we loop over the entire string and find no tokens, return std::string::npos, signifying that we didn't find a token
        return std::string::npos;
    }


	/**
	 * @brief Get the content section of a given raw token.
	 */
	std::string getRawTokenContent(	std::string rawToken	)
	{
		//First find the boundaries of the content
		int contentLeftBoundary = rawToken.find("{");
		int contentRightBoundary = rawToken.find("}$");

		//If the content boundaries around present (one of them returns npos), then we throw an error
		if(	contentLeftBoundary == std::string::npos ||
			contentRightBoundary == std::string::npos	)
		{
			std::string errorMessage = "stevensTerminal library error: In s_TerminalTokenHelper parseRawToken(std::string rawtoken)\n"
										"Could not find left or right content boundary in rawToken: " + rawToken;
			std::cerr << errorMessage;
		}

		//Check to see if we have any content within the curly braces
		if(contentRightBoundary == 1)
		{
			//Empty content
			return "";
		}

		return rawToken.substr((contentLeftBoundary + 1), (contentRightBoundary - contentLeftBoundary - 1));
	}


	/**
	 * @brief Given the raw token, get the styling string from it. This is the part of the token that comes after the {xyz}$ part
	 * 		  and is contained within [ ] brackets.
	 */
	std::string getRawTokenStyleString(	std::string rawToken	)
	{
		int contentRightBoundary = rawToken.rfind("}$");
		std::string rawToken_styleString = rawToken.substr(contentRightBoundary + 2);
		//Check to see if the square brackets are present in the token
		int styleLeftBoundary = rawToken_styleString.rfind("[");
		int styleRightBoundary = rawToken_styleString.rfind("]");
		//If they aren't present, we throw an error
		if(	styleLeftBoundary == std::string::npos ||
			styleRightBoundary == std::string::npos )
		{
			std::string errorMessage = "stevensTerminal library error: In s_TerminalPrintToken parseRawToken(std::string rawtoken)\n"
										"Could not find left or right style boundary in rawToken_styleString: " + rawToken_styleString;
			std::cerr << errorMessage;
		}
		//Get the style string
		return stevensStringLib::trim(rawToken_styleString, 1);
	}


    /**
	 *	Given a string representing a token from a string intended to be printed with the stevensTerminal library,
		*	parse the token and turn it into a s_TerminalPrintToken object to more easily work with its data.
		*
		* 	Parameter:
		* 		std::string rawToken - A string representing a token from a string intended to be printed with the stevensTerminal library.
		* 							   Example: "{The quick brown fox}$[textColor=red,bgColor=yellow,flash,bold]"
		* 	
		* 	Returns:
		* 		s_TerminalPrintToken - An object that conveniently holds all of the data from the rawToken as member variables. 
		* 							   Example:
		* 								{
		* 									content 	: "The quick brown fox",
		* 									textColor 	: "red",
		* 									bgColor		: "yellow",
		* 									flash		: true,
		* 									bold		: true
		* 								}
		*/
	s_TerminalPrintToken parseRawToken(std::string rawToken)
	{
		//std::cout << rawToken << std::endl;

		/*** Parse the content ***/
		std::string content = s_TerminalPrintTokenHelper::getRawTokenContent( rawToken );
		// std::cout << "Raw token content: " << content << std::endl;

		if(content.empty())
		{
			//If we have empty token content, don't bother styling it -- just return it.
			return s_TerminalPrintToken("");
		}

		/*** Parse the style code ***/
		std::string styleString = s_TerminalPrintTokenHelper::getRawTokenStyleString( rawToken );
		//Read the style string 
		std::unordered_map<std::string,std::string> styleMap = s_TerminalPrintTokenStyling::processPrintTokenStyle(styleString);

		//Construct the empty token object
		s_TerminalPrintToken token = s_TerminalPrintToken();
		token.rawToken = rawToken;

		//Set the variables of the token to the ones we found in the style map
		token.content = content;
		token.textColor = styleMap["textColor"];
		token.bgColor = styleMap["bgColor"];
		token.blink = stevensStringLib::stringToBool(styleMap["blink"]);
		token.bold = stevensStringLib::stringToBool(styleMap["bold"]);

		token.styled = true; //If we're parsing a token, it's because it has been individually styled

		return token;
	}


    /**
     * @brief Given a string with a stevensTerminal styling token within it and the position which it begins,
     *        get a s_TerminalPrintToken object containing data about the token.
     * 
     * @param str The string we are creating a token object from.
     * @param tokenPosition The position in string str that a token starts at.
     * @return s_TerminalPrintToken object describing the token beginning at the given token position.
     */
    s_TerminalPrintToken getToken(  const std::string & str,
                                    const size_t tokenPosition  )
    {
        //Get the raw token
        //Keeps track of what stage of token finding we're in...
		// 0 - Looking for open curlybrace
		// 1 - Looking for closed curlybrace
		// 2 - Looking for open squarebracket
		// 3 - Looking for closed squarebracket
		int styled_tokenFindStage = 0;
		int styled_startIndex = -1;
		std::string rawToken = "";

		//Find all the tokens in the input string
		for(int i = tokenPosition; i < str.length(); i++)
		{
			switch(styled_tokenFindStage)
			{
				//Search for an open curly brace
				case 0:
					if(str[i] == '{')
					{
						styled_tokenFindStage = 1;
						styled_startIndex = i;
					}
					break;
				//Search for a closed curly brace
				case 1:
					if(str[i] == '}')
					{
						styled_tokenFindStage = 2;
					}
					else if(str[i] == '{')
					{
						styled_startIndex = i;
					}
					break;
				//Is there a dollar sign directly after the closed curly brace?
				case 2:
					if(str[i] == '$')
					{
						//We may have found a token...
						styled_tokenFindStage = 3;
					}
					else
					{
						//If not, we're still not sure if we've found a token yet
						styled_tokenFindStage = 1;
					}
					break;
				//Is there an open square bracket directly after the dollar sign?
				case 3:
					if(str[i] == '[')
					{
						//We may have found a token...
						styled_tokenFindStage = 4;
					}
					else
					{
						//If not, we haven't found a token
						styled_tokenFindStage = 1;
					}
					break;
				//If so, is there a closed square bracket after the opened square bracket?
				case 4:
					if(str[i] == ']')
					{
						//If all of this is true before the end of the input string, congratulations, we have found a styled token!
						//Get the string of the raw token
						rawToken = str.substr(styled_startIndex, (i-styled_startIndex+1));
						//Create the print token obejct
						s_TerminalPrintToken token = s_TerminalPrintTokenHelper::parseRawToken(rawToken);
						token.existsAtIndex = styled_startIndex;
						token.rawToken = rawToken;
                        //Return the token object we just retrieved
                        return token;
					}
					break;
				default:
					break;
			}
		}

        //If we passed all the way through the string and did not parse a token, throw an error
        throw std::invalid_argument("Could not find  a token in string \"" + str + "\" at position " + std::to_string(tokenPosition));
    }


	/**
	 * @brief Given a string containing style tokens, return a vector containing s_TerminalPrintToken objects for each token present 
	 * 		  in the given string.
	 * 
	 * @param str The string we are scanning to obtain s_TerminalPrintToken objects from.
	 * 
	 * @return A std::vector containing s_TerminalPrintToken objects for each style token in the string str.
	 */
	std::vector<s_TerminalPrintToken> getAllTokens(	const std::string & str )
	{
		//Initialize a vector that will track all of the token's we've found in str
		std::vector<s_TerminalPrintToken> tokens;
		//Also initialize the index where we'll start looking for tokens within str
		size_t searchStart = 0;

		while(true)
		{
			//Search for a token
			size_t foundTokenIndex = s_TerminalPrintTokenHelper::findToken(str, searchStart);
			//If we don't find a token, break from the loop
			if(foundTokenIndex == std::string::npos)
			{
				break;
			}
			//Otherwise, get the token at the found index
			s_TerminalPrintToken token = s_TerminalPrintTokenHelper::getToken(str, foundTokenIndex);
			//Push it back into our vector
			tokens.push_back(token);
			//Increment our searchStart
			searchStart += token.rawToken.length();
		}

		return tokens;
	}


    /**
     * @brief Given a string and the starting position of a stevensTerminal style token within the string,
     *        remove the braces and the styling directive text from the string, leaving only the content 
     *        of the token in the original string.
     * 
     * Example: resultStr = s_TerminalTokenHelper("Hey Squidward, are you finished with those {errands}$[textColor=red]?", 45);
     * //resultStr == "Hey Squidward, are you finished with those errands?"
     * 
     * @param str The string containing tokens that we intend to untokenize
     */
    std::string untokenize( std::string str,
                            const size_t tokenPosition    )
    {
        //Get the data for the token we want to untokenize
        s_TerminalPrintToken token = s_TerminalPrintTokenHelper::getToken(str, tokenPosition);
        //At the token position, replace the raw tokenized text with the content of the token
        str.replace(token.existsAtIndex, token.rawToken.length(), token.content);
        //Return the modified string
        return str;
    }


	/**
	 * @brief Gven a string, a starting position of where we're like to take a string and turn it into
	 * 		  a stevensTerminal style token, and the length of the token, return the tokenized string.
	 * 
	 * @param str
	 * @param tokenPosition
	 * @param tokenLength
	 * @param styleString
	 */
	std::string tokenize(	std::string str,
							const size_t tokenPosition,
							const size_t tokenLength = std::string::npos,
							const std::string styleString = "textColor=default,bgColor=default"	)
	{
		//Get the content of the token
		std::string tokenContent = str.substr(tokenPosition, tokenLength);
		//Create the token
		std::string rawToken = "{" + tokenContent + "}$[" + styleString + "]";
		//Replace the token in the string
		str.replace(tokenPosition, rawToken.length(), rawToken);
		//Return the tokenized string
		return str;
	}


}