/**
 * @file s_TerminalPrintTokenStyling.hpp
 * @author Jeff Stevens jeff@bucephalusstudios.com
 * @brief Some functions used to help style s_TerminalPrintTokens.
 * @version 0.1
 * @date 2024-03-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */


namespace s_TerminalPrintTokenStyling
{
    /**
	 * Given an unordered map inteded to describe styling instructions for text that will be printed with the stevensTerminal print
	 * function, we check to see what keys are defined. If a certain key that should be defined is not defined, we simply insert that
	 * key into the style map and set it equal to its default value. The following is a map of all style keys supported and their
	 * default values:
	 *  {
	 *      {"textColor", "default"},
	 *      {"bgColor", "default"},
	 *      {"flash", "false"},
	 *      {"bold", "false"}
	 *  }
	 * 
	 * Parameter:
	 *  unordered_map<std::string,std::string> - A map of styles inteded for use with the stevensTerminal print function.
	 * 
	 * Returns:
	 *  unordered_map<std::string,std::string> - The input style map with all of its keys defined if they aren't already.
	*/
	std::unordered_map<std::string,std::string> setMissingStylesToDefault( std::unordered_map<std::string,std::string> styleMap )
	{
		//Text color
		if (!styleMap.contains("textColor"))
		{
			styleMap["textColor"] = "default";
		}

		//Background color
		if(!styleMap.contains("bgColor"))
		{
			styleMap["bgColor"] = "default";
		}

		//Flash
		if(!styleMap.contains("blink"))
		{
			styleMap["blink"] = "false";
		}

		//Bold
		if(!styleMap.contains("bold"))
		{
			styleMap["bold"] = "false";
		}

		return styleMap;
	}


    /**
	 *	Given a style string from the parseRawToken function, interpret what styling is required and 
    *  insert those style instructions into an unordered_map<std::string,std::string> to return.
    * 
    *	Parameter:
    *		std::string styleString - A string containing style properties, equal signs, values, and commas.
    *                                An example: "textColor=red,bgColor = blue,flash=true "
    *                                (Spaces in between words, equals signs, or commas are ignored)
    *
    * 	Returns:
    * 		map<std::string,std::string> - A map containing key value pairs corresponding to what was read
    * 												 from the style string. For example, the returned map from the 
    * 												 string above would be:
    * 												{
    * 													"textColor" : "red",
    * 													"bgColor"	: "blue",
    * 													"flash"		: "true"
    * 												}
    * 		
    */
	std::unordered_map<std::string,std::string> processPrintTokenStyle( std::string styleString)
	{
		std::unordered_map<std::string,std::string> styleMap = strlib::unorderedMapifyString(styleString,"=",",");

		styleMap = s_TerminalPrintTokenStyling::setMissingStylesToDefault(styleMap);

		return styleMap;
	}


}