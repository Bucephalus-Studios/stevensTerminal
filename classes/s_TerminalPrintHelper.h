/**
 * Class containing useful functions for managing printing to the terminal and text
 * styling with the stevensTerminal library. 
 */

#ifndef STEVENSTERMINAL_PRINTHELPER
#define STEVENSTERMINAL_PRINTHELPER

#include <sstream>

namespace s_TerminalPrintHelper
{
    /*** Member variables ***/
	//Map containing all of the textColors and their respective codes for coloring text in the terminal
	std::unordered_map<std::string,std::string> textColorCodes = {  {"red",     "31"},
																	{"orange",  "38:5:208"},
																	{"yellow",  "33"},
																	{"green",   "32"},
																	{"blue",    "34"},
																	{"cyan",    "36"},
																	{"purple",  "35"},
																	{"black",   "30"},
																	{"grey",    "90"},
																	{"white",   "97"}    };
	//Map containing all of the bgColors and their respective codes for coloring backgrounds of text in the terminal
	std::unordered_map<std::string,std::string> bgColorCodes = {    {"red",     "41"},
																	{"orange",  "48:5:208"},
																	{"yellow",  "43"},
																	{"green",   "42"},
																	{"blue",    "44"},
																	{"cyan",    "46"},
																	{"purple",  "45"},
																	{"black",   "40"},
																	{"grey",    "100"},
																	{"white",   "47"}  };
	std::unordered_map<std::string,int> curses_colors; //Map containing curses foreground and background colors as keys and their assigned color pair as values
	std::unordered_map<std::string,int> curses_colorCodes; //Map containing all of the curses color names and their integer values to use them in a color pair
	const std::string curses_default_textColor = "bright-white"; //The default color to print text in curses
	const std::string curses_default_backgroundColor = "black"; //The default text background color in curses

	/**
	 * @brief Data structure for defining color pairs in a data-driven way
	 *
	 * This structure allows us to define color pairs with their associated names
	 * in a compact, maintainable format rather than hundreds of lines of repetitive code.
	 */
	struct ColorPairDefinition {
		int pairNum;
		int foreground;
		int background;
		std::vector<std::string> names;
	};

	// /*** Constructor ***/
	// #ifndef curses
	// s_TerminalPrintHelper()
	// {
	// 	//Define all of the textColor and bgColor codes
	// 	textColorCodes = {  {"red",     "31"},
	// 						{"orange",  "38:5:208"},
	// 						{"yellow",  "33"},
	// 						{"green",   "32"},
	// 						{"blue",    "34"},
	// 						{"cyan",    "36"},
	// 						{"purple",  "35"},
	// 						{"black",   "30"},
	// 						{"grey",    "90"},
	// 						{"white",   "97"}    };

	// 	bgColorCodes = {    {"red",     "41"},
	// 						{"orange",  "48:5:208"},
	// 						{"yellow",  "43"},
	// 						{"green",   "42"},
	// 						{"blue",    "44"},
	// 						{"cyan",    "46"},
	// 						{"purple",  "45"},
	// 						{"black",   "40"},
	// 						{"grey",    "100"},
	// 						{"white",   "47"}  };
	// }
	// #else
	// s_TerminalPrintHelper()
	// {
	// 	//Define all of the textColor and bgColor codes
	// 	textColorCodes = {  {"red",     "31"},
	// 						{"orange",  "38:5:208"},
	// 						{"yellow",  "33"},
	// 						{"green",   "32"},
	// 						{"blue",    "34"},
	// 						{"cyan",    "36"},
	// 						{"purple",  "35"},
	// 						{"black",   "30"},
	// 						{"grey",    "90"},
	// 						{"white",   "97"}    };

	// 	bgColorCodes = {    {"red",     "41"},
	// 						{"orange",  "48:5:208"},
	// 						{"yellow",  "43"},
	// 						{"green",   "42"},
	// 						{"blue",    "44"},
	// 						{"cyan",    "46"},
	// 						{"purple",  "45"},
	// 						{"black",   "40"},
	// 						{"grey",    "100"},
	// 						{"white",   "47"}  };
	// 	curses_default_textColor = "bright-white";
	// 	curses_default_backgroundColor = "black";
	// }
	// #endif


	/*** Methods ***/
	// PROTOTYPES //
	std::vector<s_TerminalPrintToken> tokenizePrintString(	std::string & input	);
	std::string styleNonTokenText(	std::string str,
									std::unordered_map<std::string,std::string> style,
									std::vector<s_TerminalPrintToken> & tokens);
	std::string styleTokens( 	std::string str,
								std::vector<s_TerminalPrintToken> & tokensToStyle	);
	std::string ignoreTokenStyling(	std::string str,
									std::vector<s_TerminalPrintToken> tokensToStyle	);
	void wrap(	std::string const &input,
				int width,
				std::ostream &os,
				size_t indent,
				bool newLines	);
	void curses_wwrap(	WINDOW * win,
						int yMove,
						int xMove,
						std::string printString,
						size_t indent = 0,
						std::unordered_map<std::string,std::string> style = {});
	void curses_wwrap_withTokens(	WINDOW * win,
									int yMove,
									int xMove,
									std::vector<s_TerminalPrintToken> tokens,
									std::unordered_map<std::string,std::string> style,
									std::unordered_map<std::string,std::string> format,
									bool textStyling	);
	// END PROTOTYPES //
	
	
	#if defined(__linux__) || defined (__APPLE__)
	std::string styleText	(	std::string input,
								std::string textColor = "default",
								std::string bgColor = "default", 
								bool bold = false, 
								bool flash = false)
	/*
	** Changes the color in the terminal for linux computers.
	** Input:
	**		string & input - the string to have its color modified
	**		string textColor - the color to change the text to
	**		string bgColor - the color to change the background of the text to
	** Output:
	**		The string we input, but modified with ANSI codes to color it.
	*/
	{
		std::string colorCode = "\033[";
		int styleChanges = 0;

		/// Text Colors ///
		if(textColor != "default")
		{
			if(textColorCodes.contains(textColor))
			{
				colorCode += textColorCodes[textColor];
			}
			else
			{
				std::cerr << "Error: textColor requested for recolorText() not supported" << std::endl;
			}
			styleChanges++;
		}

		/// Background Colors ///
		if(bgColor != "default")
		{
			if (styleChanges > 0) //If we have previous style changes, we separate the ANSI codes with a colon
			{
				colorCode += ";";
				styleChanges = 0;
			}
			if(bgColorCodes.contains(bgColor))
			{
				colorCode += bgColorCodes[bgColor];
			}
			else
			{
				std::cerr << "Error: bgColor requested for recolorText() not supported" << std::endl;
			}
			styleChanges++;
		}

		// Bold //
		if(bold)
		{
			if(styleChanges > 0)
			{
				colorCode += ";";
				styleChanges = 0;
			}
			colorCode += "1";
			styleChanges++;
		}

		// Flash //
		if(flash)
		{
			if(styleChanges > 0)
			{
				colorCode += ";";
			}
			colorCode += "5";
			styleChanges++;
		}

		//If we requested a change to our input's text color, we finished our color code here.
		if(colorCode != "\033[")
		{
			colorCode += "m";
			input = colorCode + input + "\033[0m";
		}

		return input;
	}


	/*
	Handles any output in the terminal to make it look good and fit within a certain character range
	
	Input:
		string input: the string to print
		bool usingTextStyling: Users have the ability to tokenize groups of text in their input with curly brackets immediately
								followed by square brackets. It may look like something like this in their input:
							"{The quick brown fox}[name = fox, textColor = red, bgColor = yellow, bold = true, flash = true] jumps over the lazy dog"
								This way, users can style their printing within the input parameter. If true, we process these style requests
								when printing the input. If false, each instance of text styling we cut it out of being printed. For example, the example
								sentence above would just be printed as:
								"The quick brown fox jumps over the lazy dog"

		unordered_map<std::string, std::String> format -	A map containing all of the possible formatting options
															for the text being printed. The possible key-value pairs
															are as follows:
													"wrapped", "true"/"false"
														-Would you like the input wrapped in such a way that strings too big to
														fit on the screen horizontally wrap around to the next line? Yes if true,
														false if no.
													"indent",	numeric
														-
													"preserve newlines on wrap", "true"/"false"
														-If the input string includes newlines, should we print those newlines?
															Yes if true, no if false.
													TODO:
													Add the ability to style the entire body of text with ANSI codes. Styling text
													this way should affect everything printed except the individually styled 
													tokens.
													"textColor"
													"bgColor"
													"flash"
													"bold"
		
													

	Output:
		Prints text to stdout with cout.
	*/
	void print(	std::string input,
				std::unordered_map<std::string, std::string> style,
				std::unordered_map<std::string, std::string> format,
				bool usingTextStyling,
				std::pair<int,int> screenSize,
				std::map<std::string, s_TerminalDisplayMode> displayModes, 
				std::string currentDisplayMode  )
	{
		//First, we process our input to find all instances of tokenized text, we'll call them tokens from now on
		std::vector<s_TerminalPrintToken> tokens = tokenizePrintString(input);
		std::string printString = "";

		//Check to see if we're doing text styling
		if(usingTextStyling)
		{
			//Style all non-tokenized text
			printString = styleNonTokenText(input, style, tokens);
			//Then we apply the styling and remove the curly and square brackets around the tokens
			printString = styleTokens(printString, tokens);
		}
		else
		{
			//If we are not, then we skip applying the styles. We just remove the curly and square brackets around the tokens
			printString = ignoreTokenStyling(input, tokens);
		}

		//How do we want the text formatted when printed to the screen? Wrapped or not?
		if(strlib::stringToBool(format["wrap"]))
		{
			if(displayModes.contains(currentDisplayMode))
			{
				wrap(	printString,
						displayModes[currentDisplayMode].minSize.first,
						std::cout,
						0,
						strlib::stringToBool(format["preserve newlines on wrap"])	);
			}
			else
			{
				wrap(	printString,
						screenSize.first-1,
						std::cout,
						0,
						strlib::stringToBool(format["preserve newlines on wrap"])	);
			}
		}
		else
		{
			std::cout << printString;
		}
	}
	#endif


	//Taken from: https://stackoverflow.com/questions/61570201/is-there-a-way-to-prevent-words-from-getting-cut-in-c-output
	/*
	**
	** Takes a string as a input and outputs it to a stream, adding in newlines where relevant to
	** prevent the text from running off too far to one side of the screen.
	** Input:
	** 		string const &input - the input string
	**		int width - the max character width of a line
	**		ostream &os - the stream to output the wrapped text to
	** 		size_t indent - the amount of whitespace to indent at the start of every line
	**		bool newLines - dictates whether or not we pay attention to newlines in our stringstream
	** Output:
	**		The wrapped text to the given output stream
	*/
	void wrap(	std::string const &input,
				int width,
				std::ostream &os,
				size_t indent,
				bool newLines	) 
	{
		std::istringstream in(input);
		os << std::string(indent, ' ');
		size_t current = indent;
		std::string line;
		std::string word;
		std::string output = "";
		int lineCutOffIndex = 0;
		int numberOfLines = strlib::countLines(input);
		int currLineNum = 0;

		while(getline(in,line))
		{
			//cout << "newline";
			lineCutOffIndex = 0;
			
			//Second method, taking each line and and cutting off a word that is too big (first space we find from the end is the delimiter)
			while(true)
			{
				//Check to see if we need to wrap this line around
				if(indent + line.length() > width)
				{
					//Find the last space in the string that fits on the line
					lineCutOffIndex = line.rfind(' ', (indent + width));
					//If we can't find a space in the string and the string is too long, we just cut the line and wrap to the next line
					if(lineCutOffIndex == std::string::npos)
					{
						//Add as much of the line as we can to the output and then add a newline
						output += std::string(indent, ' ') + line.substr(0, (width-indent)) + "\n";
						//Continue looping until the rest of the line is added to the output
						line = line.substr(width-indent+1);
					}
					//If we find a space...
					else
					{
						//Add the part of the string before the cut off point to the output
						output += std::string(indent,' ') + line.substr(0, lineCutOffIndex) + "\n";
						//Set the line equal to the cut off portion of the string and continue looping until the rest of the line is added to the output
						line = line.substr(lineCutOffIndex+1);
					}
				}
				else
				{
					output += line;
					break;
				}
			}
			currLineNum++;
			//Add a new line when printing the next line from the string, or print a newline if we only have a newline character
			if((currLineNum < numberOfLines) || ((line.length()) == 0 && (numberOfLines == 1)))
			{
				output += "\n" + std::string(indent, ' ');
			}
		}

		os << output;
	}


	/**
	 * Given a string str and a vector containing the tokens appearing in str in order, return a vector 
	 * of tokens containing the sections of unstyled text and the tokens of the string in order.
	 * 
	 * Example:
	 * 
	 * str = "The quick brown {fox}$[textColor=red] jumps over the lazy {dog}$[textColor=blue]"
	 * tokens (their content) = ["fox","dog"]
	 * Returns (tokens with content): ["The quick brown ","fox","jumps over the lazy ","dog"]
	 * 
	 * This is used for printing with curses, mainly.
	 * 
	 * Parameters:
	 * 	std::string str - The string we would like to create unstyled tokens from between the tokens that we already have
	 * 	std::vector<s_TerminalPrintTokens> tokens  - The tokenized parts of the string str that we already have identified
	 * 
	 * Returns:
	 * 	std::vector<s_TerminalPrintToken> - Vector containing print token objects for every part of the print string, not just the specficially
	 * 										styled tokens we already have (which are included in the output vector)
	*/
	std::vector<s_TerminalPrintToken> tokenizeBetweenTokens(	std::string & str,
																std::vector<s_TerminalPrintToken> tokens	)
	{
		//Start at the beginning of str
		int startingIndex = 0;
		std::string contentToTokenize = "";
		//Initialize the vector to return
		std::vector<s_TerminalPrintToken> tokensToReturn = {};

		//For each token, tokenize everything up to where it occurs in str
		for(int i = 0; i < tokens.size(); i++)
		{
			//Before we try to tokenize everything up until the token we're looking at, let's see if there's actually any content up until the token
			if(startingIndex == tokens[i].existsAtIndex)
			{
				//The starting index is where the token starts -- This means there's nothing before the token to tokenize
				//Take the token and store it in the return vector
				tokensToReturn.push_back(tokens[i]);
				startingIndex += tokens[i].rawToken.length();
				continue;
			}

			//Grab everything up until the start of the token
			contentToTokenize = str.substr(startingIndex, (tokens[i].existsAtIndex-startingIndex));

			//If the content to tokenize is empty, we don't need to do anything
			if(contentToTokenize.empty())
			{
				//Just store the token in the return vector
				tokensToReturn.push_back(tokens[i]);
				startingIndex += tokens[i].rawToken.length();
				continue;
			}

			//Create a token for the content we just grabbed
			std::string rawBetweenToken = s_TerminalPrintTokenHelper::tokenize(contentToTokenize, 0, contentToTokenize.length());
			s_TerminalPrintToken betweenToken = s_TerminalPrintTokenHelper::parseRawToken(rawBetweenToken);
			betweenToken.existsAtIndex = startingIndex;
			betweenToken.styled = false; //This token is not styled, it is just the content between the tokens

			//Change the str parameter to have the tokenized content
			str.replace(startingIndex, betweenToken.content.length(), betweenToken.rawToken);

			// Increment the existsAtIndex of all tokens that come after the current token in the vector
			size_t lengthDifference = betweenToken.rawToken.length() - contentToTokenize.length();
			for (int j = i; j < tokens.size(); j++) {
				tokens[j].existsAtIndex += lengthDifference;
			}

			//Store the betweenToken in our list of tokens
			tokensToReturn.push_back(betweenToken);
			//Also store the token we previously found which follows
			tokensToReturn.push_back(tokens[i]);

			//Set the next starting index to be after 
			startingIndex = tokens[i].existsAtIndex + tokens[i].rawToken.length();
		}
		//If there's any more text content in the string after all of the tokens we have appear, account for that here
		if(startingIndex < str.length())
		{
			//Grab everything up until the end of the string
			contentToTokenize = str.substr(startingIndex);
			std::string rawBetweenToken = s_TerminalPrintTokenHelper::tokenize(contentToTokenize, 0, contentToTokenize.length());
			s_TerminalPrintToken betweenToken = s_TerminalPrintTokenHelper::parseRawToken(rawBetweenToken);
			betweenToken.existsAtIndex = startingIndex;
			betweenToken.styled = false; //This token is not styled, it is just the content after the last token

			//Change the str parameter to have the tokenized content
			str.replace(startingIndex, betweenToken.content.length(), betweenToken.rawToken);

			//Store it as our last token
			tokensToReturn.push_back(betweenToken);
		}

		return tokensToReturn;
	}


	/**
	 * Given a string that is being printed and may have formatting brackets,
	 * tokenize all the the parts of the string that have formatting brackets
	 * into s_TerminalPrintToken objects and store them in a vector that will
	 * be returned.
	 * 
	 * Parameter:
	 * 	std::string input - A string that possibly contains tokens to process.
	 * 
	 * Returns:
	 * 	vector<s_TerminalPrintToken> - A vector containing tokens found in the input string.
	*/
	
	// Forward declarations
	std::string processNestedContent(const std::string& content, const std::string& parentStyle);
	std::string inheritParentStyle(const std::string& nestedStyle, const std::string& parentStyle);
	
	// OPTIMIZED: Fast inline pattern matching without substr()
	inline bool matchesTokenEnd(const std::string& input, size_t pos) {
		return pos + 2 < input.length() && 
			   input[pos] == '}' && 
			   input[pos + 1] == '$' && 
			   input[pos + 2] == '[';
	}
	
	// OPTIMIZED: Single-pass brace matching with optimized pattern detection
	size_t findMatchingTokenEnd(const std::string& input, size_t openBrace) {
		int braceCount = 1;
		size_t pos = openBrace + 1;
		const size_t inputLen = input.length();
		
		while (pos < inputLen && braceCount > 0) {
			const char c = input[pos];
			if (c == '{') {
				braceCount++;
				pos++;
			} else if (c == '}' && matchesTokenEnd(input, pos)) {
				braceCount--;
				if (braceCount == 0) {
					// Found the matching }$[ - now find the end of the style section
					size_t styleEnd = input.find(']', pos + 3);
					return styleEnd;
				}
				pos += 3; // Skip the }$[ pattern
			} else {
				pos++;
			}
		}
		return std::string::npos;
	}

	// OPTIMIZED: Combined brace matching and content extraction in single pass
	struct TokenInfo {
		size_t contentStart;
		size_t contentEnd;
		size_t styleStart;
		size_t styleEnd;
		bool hasNestedTokens;
	};
	
	// OPTIMIZED: Single-pass token info extraction
	TokenInfo extractTokenInfo(const std::string& input, size_t openBrace) {
		TokenInfo info = {0, 0, 0, 0, false};
		int braceCount = 1;
		size_t pos = openBrace + 1;
		const size_t inputLen = input.length();
		bool foundDollarBracket = false;
		
		info.contentStart = openBrace + 1;
		
		while (pos < inputLen && braceCount > 0) {
			const char c = input[pos];
			if (c == '{') {
				braceCount++;
				// Check if we're still in the root content for nested token detection
				if (braceCount == 2 && !foundDollarBracket) {
					info.hasNestedTokens = true;
				}
				pos++;
			} else if (c == '}' && matchesTokenEnd(input, pos)) {
				braceCount--;
				if (braceCount == 0) {
					// This is the matching }$[ for our opening brace
					info.contentEnd = pos;
					info.styleStart = pos + 3;
					info.styleEnd = input.find(']', pos + 3);
					foundDollarBracket = true;
					break;
				}
				pos += 3;
			} else {
				pos++;
			}
		}
		
		return info;
	}
	
	// OPTIMIZED: Fast preprocessing with reduced string operations
	std::string preprocessNestedTokens(std::string input) {
		// Quick early exit if no tokens present
		if (input.find("}$[") == std::string::npos) {
			return input;
		}
		
		// Pre-reserve space to reduce reallocations during replacement
		std::string result;
		result.reserve(input.length() * 2); // Heuristic: nested tokens typically expand
		
		size_t pos = 0;
		const size_t inputLen = input.length();
		
		while (pos < inputLen) {
			size_t openBrace = input.find('{', pos);
			if (openBrace == std::string::npos) break;
			
			TokenInfo tokenInfo = extractTokenInfo(input, openBrace);
			
			if (tokenInfo.styleEnd == std::string::npos || tokenInfo.contentEnd == 0) {
				pos = openBrace + 1;
				continue;
			}
			
			// Only process tokens that have nested content
			if (tokenInfo.hasNestedTokens) {
				// Extract style directly without creating intermediate strings
				std::string parentStyle(input, tokenInfo.styleStart, tokenInfo.styleEnd - tokenInfo.styleStart);
				std::string content(input, tokenInfo.contentStart, tokenInfo.contentEnd - tokenInfo.contentStart);
				
				std::string processedContent = processNestedContent(content, parentStyle);
				
				// Replace the original token with processed version
				input.replace(openBrace, tokenInfo.styleEnd - openBrace + 1, processedContent);
				pos = openBrace + processedContent.length();
				continue;
			}
			
			pos = openBrace + 1;
		}
		
		return input;
	}
	
	// OPTIMIZED: High-performance recursive nested content processor
	void processNestedContent(const std::string& content, const std::string& parentStyle, std::string& result) {
		const size_t contentLen = content.length();
		size_t pos = 0;
		
		// Pre-reserve space to avoid reallocations
		if (result.capacity() < result.size() + contentLen * 2) {
			result.reserve(result.size() + contentLen * 2);
		}
		
		while (pos < contentLen) {
			size_t nextToken = content.find('{', pos);
			if (nextToken == std::string::npos) {
				// No more tokens, add remaining content with parent styling
				if (pos < contentLen) {
					result += "{";
					result.append(content, pos, contentLen - pos);
					result += "}$[" + parentStyle + "]";
				}
				break;
			}
			
			// Add content before the token with parent styling
			if (nextToken > pos) {
				result += "{";
				result.append(content, pos, nextToken - pos);
				result += "}$[" + parentStyle + "]";
			}
			
			// Extract token info in single pass
			TokenInfo tokenInfo = extractTokenInfo(content, nextToken);
			
			if (tokenInfo.styleEnd != std::string::npos && tokenInfo.contentEnd > nextToken) {
				// Extract strings directly using string constructor (more efficient than substr)
				std::string nestedContent(content, tokenInfo.contentStart, tokenInfo.contentEnd - tokenInfo.contentStart);
				std::string nestedStyle(content, tokenInfo.styleStart, tokenInfo.styleEnd - tokenInfo.styleStart);
				
				// RECURSIVELY PROCESS: Check if nested content contains deeper tokens
				if (tokenInfo.hasNestedTokens) {
					// This nested token has even deeper tokens - process them recursively!
					std::string inheritedStyle = inheritParentStyle(nestedStyle, parentStyle);
					processNestedContent(nestedContent, inheritedStyle, result);
				} else {
					// Leaf node - no deeper nesting, just inherit and add
					std::string inheritedStyle = inheritParentStyle(nestedStyle, parentStyle);
					result += "{" + nestedContent + "}$[" + inheritedStyle + "]";
				}
				
				pos = tokenInfo.styleEnd + 1;
			} else {
				pos = nextToken + 1;
			}
		}
	}
	
	// OPTIMIZED: Wrapper function for backward compatibility
	std::string processNestedContent(const std::string& content, const std::string& parentStyle) {
		std::string result;
		result.reserve(content.length() * 2); // Heuristic for nested expansion
		processNestedContent(content, parentStyle, result);
		return result;
	}
	
	// OPTIMIZED: Fast style parsing without stringstream overhead
	void parseStyleString(const std::string& style, std::unordered_map<std::string, std::string>& styleMap) {
		if (style.empty()) return;
		
		size_t pos = 0;
		const size_t styleLen = style.length();
		
		while (pos < styleLen) {
			size_t commaPos = style.find(',', pos);
			if (commaPos == std::string::npos) commaPos = styleLen;
			
			size_t eqPos = style.find('=', pos);
			if (eqPos != std::string::npos && eqPos < commaPos) {
				// Use string_view-like approach to avoid creating temporary strings
				std::string key(style, pos, eqPos - pos);
				std::string value(style, eqPos + 1, commaPos - eqPos - 1);
				styleMap[key] = value;
			}
			
			pos = commaPos + 1;
		}
	}
	
	// OPTIMIZED: Fast style inheritance with reused maps
	std::string inheritParentStyle(const std::string& nestedStyle, const std::string& parentStyle) {
		// Use static thread-local maps to avoid repeated allocations
		static thread_local std::unordered_map<std::string, std::string> parentMap;
		static thread_local std::unordered_map<std::string, std::string> nestedMap;
		
		// Clear and reuse existing maps
		parentMap.clear();
		nestedMap.clear();
		
		// Parse both styles efficiently
		parseStyleString(parentStyle, parentMap);
		parseStyleString(nestedStyle, nestedMap);
		
		// Inherit parent values for missing keys
		for (const auto& pair : parentMap) {
			if (nestedMap.find(pair.first) == nestedMap.end()) {
				nestedMap[pair.first] = pair.second;
			}
		}
		
		// Build result string with pre-calculated capacity
		std::string result;
		result.reserve(nestedStyle.length() + parentStyle.length()); // Heuristic
		
		bool first = true;
		for (const auto& pair : nestedMap) {
			if (!first) result += ",";
			first = false;
			result += pair.first;
			result += "=";
			result += pair.second;
		}
		
		return result;
	}

	std::vector<s_TerminalPrintToken> tokenizePrintString(	std::string & input	)
	{
		// First, preprocess any nested tokens
		input = preprocessNestedTokens(input);
		
		//Keeps track of what stage of token finding we're in...
		// 0 - Looking for open curlybrace
		// 1 - Looking for closed curlybrace (with proper nesting depth)
		// 2 - Looking for open squarebracket
		// 3 - Looking for closed squarebracket
		short styled_tokenFindStage = 0;
		std::string rawToken = "";
		std::vector<int> unmatchedOpenBracePositions;
		std::vector<s_TerminalPrintToken> tokenVector = {};
		std::vector<s_TerminalPrintToken> nestedTokenVector = {};
		int currentBraceDepth = 0; // Track brace depth for proper nested token parsing
		//std::stack<int> parentTokenPositions = {};

		//Iterate over each character in the string
		for(int i = 0; i < input.length(); i++)
		{
			switch(styled_tokenFindStage)
			{
				//Search for an open curly brace
				case 0:
					if(input[i] == '{')
					{
						styled_tokenFindStage = 1;
						unmatchedOpenBracePositions.push_back(i);
						currentBraceDepth = 1; // Start tracking depth for this token
					}
					break;

				//Search for a closed curly brace (accounting for nesting)
				case 1:
					if(input[i] == '{')
					{
						//Found nested opening brace, add to tracking  
						unmatchedOpenBracePositions.push_back(i);
						currentBraceDepth++;
					}
					else if(input[i] == '}')
					{
						currentBraceDepth--;
						if(currentBraceDepth == 0)
						{
							//Found the matching closing brace for our outermost token
							styled_tokenFindStage = 2;
						}
						//Note: We keep all brace positions in unmatchedOpenBracePositions for later nested token processing
					}
					break;

				//Is there a dollar sign directly after the closed curly brace?
				case 2:
					if(input[i] == '$')
					{
						//We may have found a token...
						styled_tokenFindStage = 3;
					}
					else if(input[i] == '{')
					{
						//Found another position token start - restart parsing
						unmatchedOpenBracePositions.push_back(i);
						currentBraceDepth = 1;
						styled_tokenFindStage = 1;
					}
					else
					{
						//If not, we're still not sure if we've found a token yet - restart parsing
						// Reset depth based on remaining unmatched braces
						currentBraceDepth = unmatchedOpenBracePositions.size();
						styled_tokenFindStage = 1;
					}
					break;

				//Is there an open square bracket directly after the dollar sign?
				case 3:
					if(input[i] == '[')
					{
						//We REALLY may have found a token...
						styled_tokenFindStage = 4;
					}
					else if(input[i] == '{')
					{
						//Found another position token start - restart parsing
						unmatchedOpenBracePositions.push_back(i);
						currentBraceDepth = 1;
						styled_tokenFindStage = 1;
					}
					else
					{
						//If not, we haven't found a token - restart parsing
						// Reset depth based on remaining unmatched braces
						currentBraceDepth = unmatchedOpenBracePositions.size();
						styled_tokenFindStage = 1;
					}
					break;
					
				//If so, is there a closed square bracket after the opened square bracket?
				case 4:
					if (input[i] == ']')
					{
						// Found a complete token
						int tokenStartPosition = unmatchedOpenBracePositions.back();
						
						// Remove all braces that are part of this completed token
						// We need to remove braces from the back until we reach the start of this token
						while(!unmatchedOpenBracePositions.empty() && unmatchedOpenBracePositions.back() >= tokenStartPosition)
						{
							unmatchedOpenBracePositions.pop_back();
						}

						rawToken = input.substr(tokenStartPosition, (i - tokenStartPosition + 1));
						s_TerminalPrintToken token = s_TerminalPrintTokenHelper::parseRawToken(rawToken);
						token.existsAtIndex = tokenStartPosition;

						// Check if this token is potentially nested within a parent token
						if (!unmatchedOpenBracePositions.empty()) 
						{
							// Add the potential nested token to the nested token vector
							nestedTokenVector.push_back(token);
						} 
						else 
						{
							// If it's not nested, check to see if it's a parent token!
							if(!nestedTokenVector.empty()) //IF A PARENT TOKEN...
							{
								size_t beforeNestedTokenStart = tokenStartPosition;
								size_t beforeTokenContentLength;
								size_t afterNestedTokenStart;
								std::string modifiedInput = "";
								size_t  lengthDifference = 0; // Tracks the total length difference introduced by modifications

								//We split the parent token into tokens before and after each nested token, applying the style of the parent token
								for( int n = 0; n < nestedTokenVector.size(); n++)
								{
									/*** Create token before nested token ***/
									//Get the nested token location
									size_t nestedTokenStart = nestedTokenVector[n].existsAtIndex;

									//Tokenize the string between the the tokenizeStartPosition and the start of the nested token
									std::string beforeTokenContent = input.substr(beforeNestedTokenStart+1, (nestedTokenStart - beforeNestedTokenStart - 1));
									beforeTokenContentLength = beforeTokenContent.length();
									s_TerminalPrintToken tokenBefore;

									if(!beforeTokenContent.empty())
									{
										std::string rawTokenBefore = s_TerminalPrintTokenHelper::tokenize( beforeTokenContent, 0, beforeTokenContent.length(), token.getStyleString() );
										//Create a new token object for the tokenized content before the nested token
										tokenBefore = s_TerminalPrintTokenHelper::parseRawToken(rawTokenBefore);
										tokenBefore.existsAtIndex = beforeNestedTokenStart;
										//Add it to the token before the nested token to the token vector
										tokenVector.push_back(tokenBefore);
										modifiedInput += tokenBefore.rawToken;
										// Update the cumulative length difference
    									lengthDifference += tokenBefore.rawToken.length() - beforeTokenContent.length();
									}

									/*** Add nested token ***/
									//Apply inherited styles to nested token
									nestedTokenVector[n].inheritStyle( token );
									nestedTokenVector[n].existsAtIndex += lengthDifference - 1; // Adjust the existsAtIndex based on cumulative length difference
									tokenVector.push_back(nestedTokenVector[n]);
									modifiedInput += nestedTokenVector[n].rawToken;

									//Set the afterNestedTokenStart
									afterNestedTokenStart = nestedTokenStart + nestedTokenVector[n].rawToken.length();
								}

								/*** Create a token after the nested tokens ***/
								//If there is any content after the nested token
								std::string afterTokenContent = input.substr(afterNestedTokenStart, token.content.length() - (beforeTokenContentLength + nestedTokenVector.back().rawToken.length()));
								if(!afterTokenContent.empty())
								{
									std::string rawTokenAfter = s_TerminalPrintTokenHelper::tokenize( afterTokenContent, 0, std::string::npos, token.getStyleString() );
									//Create a new token object for the tokenized content after the nested token
									s_TerminalPrintToken tokenAfter = s_TerminalPrintTokenHelper::parseRawToken(rawTokenAfter);
									tokenAfter.existsAtIndex = afterNestedTokenStart + lengthDifference - 1; // Adjust the existsAtIndex based on cumulative length difference
									//Add it to the token after the nested token to the token vector
									tokenVector.push_back(tokenAfter);
									modifiedInput += tokenAfter.rawToken;

									//Update the cumulate length difference
									lengthDifference = tokenAfter.rawToken.length() - tokenAfter.content.length(); // Update the cumulative length difference
								}

								//Replace the content of the original raw parent token with the modified input
								size_t originalLength = token.rawToken.length();
								size_t modifiedLength = modifiedInput.length();
								input.replace(tokenStartPosition, originalLength, modifiedInput);

								// Adjust the existsAtIndex values of all tokens after the modified token
								size_t totalLengthDifference = modifiedLength - originalLength;

								// Because we modified the input string, we need to adjust i
								i += totalLengthDifference;
								
								//Clear the nested token vector
								nestedTokenVector.clear();
							}
							else //IF NOT A PARENT TOKEN
							{
								//No nested tokens and not a parent token. Just add it to the token vector normally.
								//std::cout << rawToken << std::endl;
								tokenVector.push_back(token);
							}
						}

						// Reset the state
						if (unmatchedOpenBracePositions.empty()) 
						{
							styled_tokenFindStage = 0;
						} 
						else 
						{
							styled_tokenFindStage = 1;
						}
					} 
					else if (input[i] == '{') 
					{
						unmatchedOpenBracePositions.push_back(i);
					}
					break;
				default:
					break;
			}
		}

		return tokenVector;
	}


	/**
	 * Given a vector of s_TerminalPrintTokens, log them and all of their member variables to a file.
	 * Useful for debugging and understanding what is and isn't being tokenized.
	 * 
	 * Parameters:
	*/
	void logTokens(	std::vector<s_TerminalPrintToken> tokens)
	{
		//Create a file.
		std::ofstream tokenLog ("s_Terminal_tokenLog.txt", std::ios::out | std::ios::app);
		if (tokenLog.is_open())
		{
			//Opening brace
			tokenLog << "{";
			//Now we write to the token log for each token we have and all of its variables
			for(int i = 0; i < tokens.size(); i++)
			{	
				tokenLog << "\n\t{\n";
				tokenLog << "\t\tstyled : " 		<< tokens[i].styled << "\n";
				tokenLog << "\t\texistsAtIndex : " 	<< tokens[i].existsAtIndex << "\n";
				tokenLog << "\t\tcontent : "		<< tokens[i].content << "\n";
				tokenLog << "\t\trawToken : " 		<< tokens[i].rawToken << "\n";
				tokenLog << "\t\ttextColor : " 		<< tokens[i].textColor << "\n";
				tokenLog << "\t\tbgColor : " 		<< tokens[i].bgColor << "\n";
				tokenLog << "\t\tbold : " 			<< tokens[i].bold << "\n";
				tokenLog << "\t}\n";
			}
			//Closing brace
			tokenLog << "}\n";
		}
		else 
		{
			std::cout << "s_Terminal:s_TerminalPrintHelper: logTokens: Unable to open or write to s_Terminal_tokenLog.txt!" << std::endl;
		}
	}


	/**
	 * Given a string str, a map containing style directives, and a vector of style tokens contained in str,
	 * apply ANSI styling to all non-tokenized parts of str.
	 * 
	 * Parameters:
	 * 	std::string str - The string to add ANSI style to all non-tokenized parts of.
	 * 	std::unordered_map<std::string,std::string> style - The styling directives for all non-tokenized parts of str.
	 * 	std::vector<s_TerminalPrintToken> tokens - All of the tokens that have been found to be a part of str. They are 
	 * 											   ordered in this vector in the order they appear in str.
	 * 
	 * Returns:
	 * 	std::string - An edited
	 * 	
	*/
	std::string styleNonTokenText(	std::string str,
									std::unordered_map<std::string,std::string> style,
									std::vector<s_TerminalPrintToken> & tokens)
	{
		style = s_TerminalPrintTokenStyling::setMissingStylesToDefault(style);

		//If we have no tokens in our text, just style it right away
		if(tokens.size() == 0)
		{
			return styleText(	str,
								style["textColor"],
								style["bgColor"],
								strlib::stringToBool(style["bold"]),
								strlib::stringToBool(style["flash"]));
		}
		else
		{
			std::string nonTokenTextContent = "";
			std::string styledContent = "";
			int startingIndexAdjustment = 0; //The amount we are incrementing future indexes of all ahead tokens
			int styleStartIndex = 0; //The index we are beginning styling at

			//For every token we have, style all the non-token text right up to before it
			for(int i = 0; i < tokens.size(); i++)
			{
				//Get all the text right up to before the token
				nonTokenTextContent = str.substr(styleStartIndex, (tokens[i].existsAtIndex-styleStartIndex));

				//Style the text
				styledContent = styleText(	nonTokenTextContent,
											style["textColor"],
											style["bgColor"],
											strlib::stringToBool(style["bold"]),
											strlib::stringToBool(style["flash"])	);

				//Erase the non-token text content from the original string
				str.erase(styleStartIndex, nonTokenTextContent.length());			

				//Insert the styled content into the original string
				str.insert(styleStartIndex, styledContent);

				//After we style the text, we check to see how much longer the styled text is from the original text
				startingIndexAdjustment = styledContent.length() - nonTokenTextContent.length();
				
				//For each remaining token, adjust the indices at which the tokens exist based upon the size change of str
				for(int k = i; k < tokens.size(); k++)
				{
					tokens[k].existsAtIndex += startingIndexAdjustment;
				}

				//Begin our next non-token styling right after the end of the token
				styleStartIndex += tokens[i].rawToken.length();
			}
		}

		return str;
	}


	/**
	 * Given a string str and a vector of s_TerminalPrintTokens found in str, restyle the str based on the styling instructions indicated 
	 * in the vector of tokens.
	 * 
	 * Parameters:
	 * 	std::string str - The string we intend to restyle.
	 * 	std::vector<s_TerminalPrintToken> tokensToStyle - A vector containing s_TerminalPrintToken objects, which describe the style
	 * 													  tokens contained in the str parameter. Ordered in the order the tokens exist
	 * 													  in the string.
	 * 
	 * Returns:
	 * 	std::string - A string styled appropriately with ANSI codes as directed from the tokensToStyle vector
	 */
	std::string styleTokens( 	std::string str,
								std::vector<s_TerminalPrintToken> & tokensToStyle	)
	{
		if(tokensToStyle.size() == 0)
		{
			return str;
		}
		else
		{
			std::string styledContent = "";
			int startingIndexAdjustment = 0;

			//For each token in the tokensToStyle vector
			for(int i = 0; i < tokensToStyle.size(); i++)
			{
				//Style the content from the token
				styledContent = styleText(	tokensToStyle[i].content,
											tokensToStyle[i].textColor,
											tokensToStyle[i].bgColor,
											tokensToStyle[i].bold,
											tokensToStyle[i].blink	);

				//Erase the token from the original string
				str.erase(tokensToStyle[i].existsAtIndex, tokensToStyle[i].rawToken.length());			

				//Insert the styled content into the original string
				str.insert(tokensToStyle[i].existsAtIndex, styledContent);

				//For each remaining token, adjust the indices at which the tokens exist based upon the size change of str
				startingIndexAdjustment = styledContent.length() - tokensToStyle[i].rawToken.length();
				//startingIndexAdjustment = tokensToStyle[i].rawToken.length() - styledContent.length();
				for(int k = i; k < tokensToStyle.size(); k++)
				{
					tokensToStyle[k].existsAtIndex += startingIndexAdjustment;
				}
			}
		}
		
		return str;
	}


	/**
	 * Given a string, str, and std::vector<s_TerminalPrintToken>, find all the instances of the
	 * raw tokens in str and erase them.
	 * 
	 * Parameters:
	 * 	std::string str - The string we with to erase raw tokens from.
	 *	std::vector<s_TerminalPrintToken> - A vector of tokens that the string str has included within in it in raw string form.
		* 
		* Returns:
		* 	std::string - The input string str, but with all of the raw tokens removed.
		*/
	std::string ignoreTokenStyling(	std::string str,
									std::vector<s_TerminalPrintToken> tokensToStyle	)
	{
		    // Track the cumulative length difference caused by replacements
			int cumulativeLengthDifference = 0;

			// For each token in the tokensToStyle vector
			for (int i = 0; i < tokensToStyle.size(); i++) {
				// Adjust the existsAtIndex of the current token based on the cumulative length difference
				tokensToStyle[i].existsAtIndex += cumulativeLengthDifference;
		
				// Replace the raw token in the string with its content
				str.replace(tokensToStyle[i].existsAtIndex, tokensToStyle[i].rawToken.length(), tokensToStyle[i].content);
		
				// Calculate the length difference caused by this replacement
				int lengthDifference = tokensToStyle[i].content.length() - tokensToStyle[i].rawToken.length();
		
				// Update the cumulative length difference
				cumulativeLengthDifference += lengthDifference;
		
				// Debugging: Print the updated string
				// std::cout << "Updated string: " << str << std::endl;
			}
		
			return str;
	}


	#ifdef curses
	/**
	 * @brief Detects how many curses colors the terminal can display, and appropriately populates a map (curses_colorCodes)
	 * where each color is mapped to its integer value.
	 * 
	 * @return void
	 */
	void curses_setup_colorCodes()
	{
		//First 2 colors
		if(COLORS >= 2)
		{
			curses_colorCodes["black"] = COLOR_BLACK;
			curses_colorCodes["white"] = COLOR_WHITE;
		}
		else
		{
			cout << "Error from s_TerminalPrintHelper - less than 2 colors available for use detected! Closing program." << endl;
			//TODO: Work on this later, but we need to throw some kind of error if we can't support more than 2 colors in a terminal
			exit(EXIT_FAILURE);
		}
		//8 colors
		if(COLORS >= 8)
		{
			curses_colorCodes["red"] 		= COLOR_RED;
			curses_colorCodes["green"] 		= COLOR_GREEN;
			curses_colorCodes["yellow"]		= COLOR_YELLOW;
			curses_colorCodes["blue"]		= COLOR_BLUE;
			curses_colorCodes["magenta"]	= COLOR_MAGENTA;
			curses_colorCodes["cyan"]		= COLOR_CYAN;
		}
		//16 colors
		if(COLORS >= 16)
		{
			curses_colorCodes["bright-black"]	= 8;	curses_colorCodes["grey"] = 8;
			curses_colorCodes["bright-red"]		= 9;
			curses_colorCodes["bright-green"]	= 10;
			curses_colorCodes["bright-yellow"] 	= 11;
			curses_colorCodes["bright-blue"]	= 12;	curses_colorCodes["aqua"] = 12;
			curses_colorCodes["bright-magenta"] = 13;	curses_colorCodes["pink"] = 13;
			curses_colorCodes["bright-cyan"]	= 14;
			curses_colorCodes["bright-white"]	= 15;
		}
	}


	/**
	 * @brief Helper function to register a batch of color pair definitions
	 *
	 * @param definitions Vector of ColorPairDefinition structs to register
	 */
	inline void registerColorPairs(const std::vector<ColorPairDefinition>& definitions) {
		for (const auto& def : definitions) {
			init_pair(def.pairNum, def.foreground, def.background);
			for (const auto& name : def.names) {
				curses_colors[name] = def.pairNum;
			}
		}
	}

	/**
	 * @brief Helper to generate color pair definitions for a given background across all foregrounds
	 *
	 * @param startPairNum Starting pair number
	 * @param bgColor Background color code
	 * @param bgName Background color name (e.g., "black", "red", "bright-blue")
	 * @param fgColors Vector of foreground color codes
	 * @param fgNames Vector of foreground color names (parallel to fgColors)
	 * @param extraAliases Map of pair offsets to additional alias names
	 * @return Vector of ColorPairDefinition structs
	 */
	inline std::vector<ColorPairDefinition> generateBackgroundPairs(
		int startPairNum,
		int bgColor,
		const std::string& bgName,
		const std::vector<int>& fgColors,
		const std::vector<std::string>& fgNames,
		const std::unordered_map<int, std::vector<std::string>>& extraAliases = {}
	) {
		std::vector<ColorPairDefinition> defs;
		for (size_t fgIndex = 0; fgIndex < fgColors.size(); ++fgIndex) {
			std::vector<std::string> names = {fgNames[fgIndex] + "_on_" + bgName};

			// Add any extra aliases for this pair
			int pairOffset = static_cast<int>(fgIndex);
			auto aliasIt = extraAliases.find(pairOffset);
			if (aliasIt != extraAliases.end()) {
				names.insert(names.end(), aliasIt->second.begin(), aliasIt->second.end());
			}

			defs.push_back({
				startPairNum + static_cast<int>(fgIndex),
				fgColors[fgIndex],
				bgColor,
				names
			});
		}
		return defs;
	}

	/**
	 * @brief Helper to generate all 16 foreground combinations (8 standard + 8 bright) for a bright background
	 *
	 * Handles the complex aliasing where bright backgrounds have alternate names (grey, aqua, pink)
	 * and bright foregrounds also have these aliases.
	 *
	 * @param startPairNum Starting pair number
	 * @param bgColor Background color code (8-15)
	 * @param bgName Background color name (e.g., "bright-black")
	 * @param bgAlias Optional alias for background (e.g., "grey")
	 * @return Vector of ColorPairDefinition structs for all 16 foregrounds
	 */
	inline std::vector<ColorPairDefinition> generateBrightBackgroundPairs(
		int startPairNum,
		int bgColor,
		const std::string& bgName,
		const std::string& bgAlias = ""
	) {
		std::vector<ColorPairDefinition> defs;

		// Standard 8 foregrounds
		const std::vector<int> std8FG = {COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
		                                  COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE};
		const std::vector<std::string> std8Names = {"black", "red", "green", "yellow",
		                                             "blue", "magenta", "cyan", "white"};

		for (size_t i = 0; i < 8; ++i) {
			std::vector<std::string> names = {std8Names[i] + "_on_" + bgName};
			if (!bgAlias.empty()) {
				names.push_back(std8Names[i] + "_on_" + bgAlias);
			}
			defs.push_back({startPairNum + static_cast<int>(i), std8FG[i], bgColor, names});
		}

		// Bright 8 foregrounds (8-15) with their aliases
		const std::vector<int> bright8FG = {8, 9, 10, 11, 12, 13, 14, 15};
		const std::vector<std::string> bright8Names = {
			"bright-black", "bright-red", "bright-green", "bright-yellow",
			"bright-blue", "bright-magenta", "bright-cyan", "bright-white"
		};
		const std::vector<std::string> bright8Aliases = {"grey", "", "", "", "aqua", "pink", "", ""};

		for (size_t i = 0; i < 8; ++i) {
			std::vector<std::string> names = {bright8Names[i] + "_on_" + bgName};
			if (!bgAlias.empty()) {
				names.push_back(bright8Names[i] + "_on_" + bgAlias);
			}

			// Add foreground alias variants if applicable
			if (!bright8Aliases[i].empty()) {
				names.push_back(bright8Aliases[i] + "_on_" + bgName);
				if (!bgAlias.empty()) {
					names.push_back(bright8Aliases[i] + "_on_" + bgAlias);
				}
			}

			// Special case: when both FG and BG are the same aliased color
			if (bright8FG[i] == bgColor && !bright8Aliases[i].empty() && !bgAlias.empty()) {
				names.push_back(bright8Aliases[i] + "_on_" + bright8Names[i]);
				if (bright8Aliases[i] == bgAlias) {
					names.push_back(bgAlias + "_on_" + bgAlias);
				}
			}

			defs.push_back({startPairNum + 8 + static_cast<int>(i), bright8FG[i], bgColor, names});
		}

		return defs;
	}

	/**
	 * Sets up color pairs for every combination of the default colors in the curses library:
	 * COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
	 * COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE
	 *
	 * Also the extended bright colors:
	 * 8  - Bright black  (GREY)
	 * 9  - Bright red
	 * 10 - Bright green
	 * 11 - Bright yellow
	 * 12 - Bright blue   (AQUA)
	 * 13 - Bright magenta (PINK)
	 * 14 - Bright cyan
	 * 15 - Bright white
	 *
	 * Additionally, populates the curses_colors map.
	 *
	 * This function uses a data-driven approach to reduce code repetition
	 * from 320+ lines to ~100 lines while maintaining identical functionality.
	 */
	void curses_setup_colorPairs()
	{
		// Syntax is: init_pair(keyInCOLOR_PAIR, foregroundColor, backgroundColor)

		// If we support just one color pair, then we just do white on black (default)
		if ((COLORS >= 2) && (COLOR_PAIRS >= 1)) {
			registerColorPairs({{0, COLOR_WHITE, COLOR_BLACK, {"default"}}});
		} else {
			// Error: Cannot support colors
			std::cout << "Error from s_TerminalPrintHelper - less than 2 colors available for use detected! Closing program." << std::endl;
			exit(EXIT_FAILURE);
		}

		// If we support two color pairs, we also do black on black
		if (COLOR_PAIRS >= 2) {
			registerColorPairs({{1, COLOR_BLACK, COLOR_BLACK, {"black_on_black"}}});
		}

		// Define standard 8 colors for reuse
		const std::vector<int> standard8Colors = {
			COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
			COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE
		};
		const std::vector<std::string> standard8Names = {
			"black", "red", "green", "yellow",
			"blue", "magenta", "cyan", "white"
		};

		// If we support 8 color pairs, do the rest of black backgrounds (already did black_on_black)
		if ((COLORS >= 8) && (COLOR_PAIRS >= 8)) {
			// Black background: red through white (pairs 2-8)
			registerColorPairs(generateBackgroundPairs(2, COLOR_BLACK, "black",
				{COLOR_RED, COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE},
				{"red", "green", "yellow", "blue", "magenta", "cyan", "white"}));
		}

		// If we support up to 16 color pairs, do red backgrounds
		if ((COLORS >= 8) && (COLOR_PAIRS >= 16)) {
			registerColorPairs(generateBackgroundPairs(9, COLOR_RED, "red", standard8Colors, standard8Names));
		}

		// If we support up to 64 color pairs, do the rest of the standard 8 color backgrounds
		if ((COLORS >= 8) && (COLOR_PAIRS >= 64)) {
			// Green, yellow, blue, magenta, cyan, white backgrounds (pairs 17-64)
			const std::vector<std::pair<int, std::string>> backgrounds = {
				{COLOR_GREEN, "green"}, {COLOR_YELLOW, "yellow"}, {COLOR_BLUE, "blue"},
				{COLOR_MAGENTA, "magenta"}, {COLOR_CYAN, "cyan"}, {COLOR_WHITE, "white"}
			};

			int pairNum = 17;
			for (const auto& [bgColor, bgName] : backgrounds) {
				registerColorPairs(generateBackgroundPairs(pairNum, bgColor, bgName,
					standard8Colors, standard8Names));
				pairNum += 8;
			}
		}
		// If we support up to 16 colors and 128 color pairs, add pairs for colors 8-15 as foregrounds on the 0-7 backgrounds
		if ((COLORS >= 16) && (COLOR_PAIRS >= 128)) {
			// Define extended 8 bright colors (8-15)
			const std::vector<int> bright8Colors = {8, 9, 10, 11, 12, 13, 14, 15};
			const std::vector<std::string> bright8Names = {
				"bright-black", "bright-red", "bright-green", "bright-yellow",
				"bright-blue", "bright-magenta", "bright-cyan", "bright-white"
			};

			// Aliases for certain bright colors
			const std::unordered_map<int, std::vector<std::string>> brightColorAliases = {
				{0, {"grey"}},           // bright-black = grey
				{4, {"aqua"}},           // bright-blue = aqua
				{5, {"pink"}}            // bright-magenta = pink
			};

			// Background names for the standard 8 backgrounds
			const std::vector<std::pair<int, std::string>> std8Backgrounds = {
				{COLOR_BLACK, "black"}, {COLOR_RED, "red"}, {COLOR_GREEN, "green"},
				{COLOR_YELLOW, "yellow"}, {COLOR_BLUE, "blue"}, {COLOR_MAGENTA, "magenta"},
				{COLOR_CYAN, "cyan"}, {COLOR_WHITE, "white"}
			};

			int pairNum = 65;
			for (const auto& [bgColor, bgName] : std8Backgrounds) {
				registerColorPairs(generateBackgroundPairs(pairNum, bgColor, bgName,
					bright8Colors, bright8Names, brightColorAliases));
				pairNum += 8;
			}
		}
		// Create a color pair for each one of the 8 additional brightened colors as background (pairs 129-256)
		if ((COLORS >= 16) && (COLOR_PAIRS >= 256)) {
			// Define bright backgrounds with their aliases
			const std::vector<std::tuple<int, std::string, std::string>> brightBackgrounds = {
				{8, "bright-black", "grey"},
				{9, "bright-red", ""},
				{10, "bright-green", ""},
				{11, "bright-yellow", ""},
				{12, "bright-blue", "aqua"},
				{13, "bright-magenta", "pink"},
				{14, "bright-cyan", ""},
				{15, "bright-white", ""}
			};

			int pairNum = 129;
			for (const auto& [bgColor, bgName, bgAlias] : brightBackgrounds) {
				registerColorPairs(generateBrightBackgroundPairs(pairNum, bgColor, bgName, bgAlias));
				pairNum += 16; // Each bright background gets 16 foregrounds
			}
		}
	}


	/**
	 * Given an s_TerminalPrintToken, retrieve all of the data for attributes we need to turn on.
	 * 
	 * Parameters:
	 * 	s_TerminalPrintToken token - The token we are enabling the style attributes for.
	 * 
	 * Returns:
	 * 	void
	*/
	unordered_map<std::string,int>	curses_styleToken(	s_TerminalPrintToken token	)
	{
		unordered_map<std::string,int> curses_attribute_data = {};

		/*** COLOR ***/
		if(token.textColor == "default")
		{
			token.textColor = curses_default_textColor;
		}
		if(token.bgColor == "default")
		{
			token.bgColor = curses_default_backgroundColor;
		}
		std::string color_key = token.textColor + "_on_" + token.bgColor;
		curses_attribute_data["colorPair"] = COLOR_PAIR(curses_colors[color_key]);

		/*** BLINK ***/
		if( token.blink )
		{
			curses_attribute_data["blink"] = A_BLINK;
		}
		else
		{
			curses_attribute_data["blink"] = 0;
		}

		/*** BOLD ***/
		if( token.bold )
		{
			curses_attribute_data["bold"] = A_BOLD;
		}
		else
		{
			curses_attribute_data["bold"] = 0;
		}

		return curses_attribute_data;
	}


	/**
	 * Given a map describing the styles to be applied to non-tokenized text, create a map of attribute
	 * names and their associated values which should be turned on in curses to style the text.
	 * 
	 * Parameter:
	 * 	unordered_map<std::string,std::string> style -	An unordered_map object containing keys of specific style options for text
	 * 													with associated values which describe how to express the style option.
	 * 													{
	 * 														{"textColor"	: the color of the text in the foreground},
	 * 														{"bgColor"		: the color of the background of the text},
	 * 														{"flash"		: true/false of whether or not to have the text flash},
	 * 														{"bold"			: true/false of whether or not to the have text bold}
	 * 													}
	 * 
	 * Returns:
	 * 	void
	*/
	unordered_map<std::string,int> curses_styleAttributes(	unordered_map<std::string,std::string> style	)
	{
		unordered_map<std::string,int> curses_attribute_data = {};

		/*** COLOR ***/
		if(style["textColor"] == "default")
		{
			style["textColor"] = curses_default_textColor;
		}
		if(style["bgColor"] == "default")
		{
			style["bgColor"] = curses_default_backgroundColor;
		}
		std::string color_key = style["textColor"] + "_on_" + style["bgColor"];

		curses_attribute_data["colorPair"] = COLOR_PAIR(curses_colors[color_key]);

		//printw("%s", color_key.c_str());
		/*** BlINK ***/
		if(style["blink"] == "true")
		{
			curses_attribute_data["blink"] = A_BLINK;
		}
		else
		{
			curses_attribute_data["blink"] = 0;
		}

		/*** BOLD ***/
		if(style["bold"] == "true")
		{
			curses_attribute_data["bold"] = A_BOLD;
		}
		else
		{
			curses_attribute_data["bold"] = 0;
		}

		return curses_attribute_data;
	}


	/**
	 * For a curses window, turns on the attributes that are defined in the passed-in map.
	 * 
	 * Parameters:
	 * 	WINDOW * win - The window which we are turning on curses attributes for.
	 * 	unordered_map<std::string, int> curses_attribute_data - A map containing all of the attribute names (keys) and the attribute values (values)
	 * 															to turn on for the curses window.
	 * 
	 * Returns:
	 * 	void;
	*/
	void curses_wAttrOn(	WINDOW * win,
							unordered_map<std::string, int> curses_attribute_data	)
	{
		for (auto const& [attrName, attrVal] : curses_attribute_data)
		{
			wattron(win, attrVal);
		}
	}


	/**
	 * Turns off curses attributes for a window that are defined in a passed-map.. 
	 * 
	 * Parameters:
	 * 	WINDOW * win - The wind which we are turning off curses attributes for.
	 * 	unordered_map<std::string, int> curses_attribute_data - A map containing all of the attribute names (keys) and the attribute values (values)
	 * 															to turn off for the curses window.
	 * 
	 * 	Returns:
	 * 		void;
	*/
	void curses_wAttrOff(	WINDOW * win,
							unordered_map<std::string, int> curses_attribute_data	)
	{
		for (auto const& [attrName, attrVal] : curses_attribute_data)
		{
			wattroff(win, attrVal);
		}
	}


	/**
	 * For curses stdscr, turns on the attributes that are defiend in the passed-in map.
	 * 
	 * Parameters:
	 * 	unordered_map<std::string, int> curses_attribute_data - A map containing all of the attribute names (keys) and the attribute values (values)
	 * 															to turn on for curses stdscr.
	 * 
	 * Returns:
	 * 	void;
	*/
	void curses_attrOn(		unordered_map<std::string, int> curses_attribute_data	)
	{
		for (auto const& [attrName, attrVal] : curses_attribute_data)
		{
			attron(attrVal);
		}
	}


	//TODO: Needs to be tested for multiple tokens, Needs to print individual lines at a time to account for fitting inside of borders
	/**
	 * Prints text with style tokens to a curses window.
	 * 
	 * Parameters:
	 * 	WINDOW * win - The curses window we are printing to.
	 * 	int yMove - How far down to move within the curses window before we begin printing.
	 * 	int xMove - How far right to move within the curses window before we begin printing.
	 * 	std::string printString - The string we will be printing to the curses window.
	 * 	std::vector<s_TerminalPrintToken> tokens - An ordered collection of s_TerminalPrintToken objects which will be printed to the curses window.
	 * 	unordered_map<std::string,std::string> style - Styling options for all unstyled tokens.
	 * 	unordereD_map<std::string,std::string> format - Advanced formatting options for printing.
	 * 													Valid key-value pairs are:
	 * 													{"avoid borders","true"/"false"}
	 * 														-Avoid reprinting over 
	 * 
	 * Returns:
	 * 	void
	*/
	void curses_wprint_withTokens(	WINDOW * win,
									int yMove,
									int xMove,
									std::vector<s_TerminalPrintToken> tokens,
									unordered_map<std::string,std::string> style,
									unordered_map<std::string,std::string> format,
									bool textStyling	)
	{
		//Holds data for which attributes to use before printing text
		unordered_map<std::string, int> curses_attribute_data = {};
		//Process the style map's attributes
		unordered_map<std::string, int> style_attribute_data = curses_styleAttributes(style);

		/*** Formatting ***/
		//Very important - get the window size
		int winHeight;
		int winWidth;
		getmaxyx(win, winHeight, winWidth);
		//Where the lines will start and end
		int lineStart = 0;
		int lineEnd = winWidth + 1;
		//If we are avoiding window borders while printing, we make sure are starting our printing within the window size
		if(format.contains("avoid borders"))
		{
			if(stevensStringLib::stringToBool(format["avoid borders"]))
			{
				//Don't print on the left border
				if(xMove < 1)
				{
					xMove = 1;
				}
				//Don't print on the right border
				if(xMove >= winWidth)
				{
					xMove = winWidth - 1;
				}
				//Don't print on the top border
				if(yMove < 1)
				{
					yMove = 1;
				}
				//Don't print on the bottom border
				if(yMove >= winHeight)
				{
					yMove = winHeight - 1;
				}

				//Window width and height modifier for borders
				winWidth -= 2;
				winHeight -= 2;
				//Make the lines start and end within the border of the window
				lineStart = 1;
				lineEnd = winWidth;
			}
		}

		//For each token we are printing:
		for(int i = 0; i < tokens.size(); i++)
		{
			//Is the token specifically styled?
			if(tokens[i].styled)
			{
				curses_attribute_data = curses_styleToken(tokens[i]);
			}
			//Are there any styles to apply to all of the non-specifically styled tokens?
			else
			{
				curses_attribute_data = style_attribute_data;
			}

			//Turn on the attributes specified in our styles
			curses_wAttrOn(win, curses_attribute_data);

			//Print each line

			//Print!
			mvwprintw(win, yMove, xMove, "%s", tokens[i].content.c_str());
			
			//Turn off all attributes
			curses_wAttrOff(win, curses_attribute_data);
		}
	}


	/**
	 * Prints a string to a curses window with advanced formatting options.
	 * 
	 * Parameters:
	 * 	WINDOW * win - The curses window we are printing to.
	 * 	int yMove - How far down to move within the curses window before we begin printing.
	 * 	int xMove - How far right to move within the curses window before we begin printing.
	 * 	std::string input - The string of text we want to print to the curses window.
	 * 	unordered_map<std::string,std::string> style - The styling options that we will apply to the text that we print to the curses window.
	 * 	unordered_map<std::string,std::string> format - Any advanced formatting options we wish to apply while printing to the curses window.
	 * 													Valid key-value pairs are:
	 * 													{
	 * 													}
	 * 	bool textStyling - True if we are styling our text. False if we're not styling our text.
	 * 
	 * Returns:
	 * 	void
	*/
	void curses_wprint( WINDOW * win,
						int yMove,
						int xMove,
						std::string input,
						unordered_map<std::string,std::string> style,
						unordered_map<std::string,std::string> format,
						bool textStyling	)
	{
		//Tokenize what we're going to be printing, just to see if a user included any inline style tokens
		std::vector<s_TerminalPrintToken> tokens = tokenizePrintString(input);

		//Check to see if we're doing text styling
		if(textStyling)
		{
			//For any space in between tokens, we tokenize that with default styling
			tokens = tokenizeBetweenTokens(input, tokens);

			//Check to see if the style map is complete before we print
			style = s_TerminalPrintTokenStyling::setMissingStylesToDefault(style);

			//TODO
			//Were there any style tokens in the input? If not, just print regularly with style
			//This should speed up printing of borders

			//Are we wrapping the text within the window?
			if(format.contains("wrap"))
			{
				if(stevensStringLib::stringToBool(format["wrap"]))
				{
					curses_wwrap_withTokens(	win,
												yMove,
												xMove,
												tokens,
												style,
												format,
												textStyling	);
					if(format.contains("debug"))
					{					
						std::cout << "finished print" << std::endl;
						getch();
					}
					return;
				}
			}
			//Print the tokens to the window
			curses_wprint_withTokens(	win,
										yMove,
										xMove,
										tokens,
										style,
										format,
										textStyling	);
		}
		else
		{
			//If we are not, then we skip applying the styles. We just remove the curly and square brackets around the tokens
			std::string printString = ignoreTokenStyling(input, tokens);

			if(format.contains("wrap"))
			{
				if(stevensStringLib::stringToBool(format["wrap"]))
				{
					curses_wwrap( 	win,
									yMove,
									xMove,
									printString,
									0	);
					return;
				}
			}

			//Print string to the screen
			mvwprintw(win, yMove, xMove, "%s", printString.c_str());
		}
		return;
	}


	/**
	 * Prints and styles a string into a curses window, making sure to wrap the text around the curses window so nothing is cut off.
	 * 
	 * Parameters:
	 *  WINDOW * win - The curses window we are printing to
	 * 	int yMove - The number of character columns down to begin printing at
	 * 	int xMove - The number of character columns to the right to begin printing at
	 * 	std::string printString - The string we wish to print to the curses window
	 * 	size_t indent - The amount of spaces to print right after a newline
	 * 	unordered_map<std::string,std::string> style - The style to apply to the text that we print	
	 * 
	 * Returns:
	*/
	void curses_wwrap(	WINDOW * win,
						int yMove,
						int xMove,
						std::string printString,
						size_t indent,
						unordered_map<std::string,std::string> style )
	{
		std::istringstream in(printString);
		std::string line;
		std::string output;
		int lineCutOffIndex = 0;
		int numberOfLines = strlib::countLines(printString);
		int currLineNum = 0;
		int width;
		int height;
		getmaxyx(win, height, width); //Curses function to get the width of the window we are printing to

		//Turn on any styles we might be using
		std::unordered_map<std::string,int> curses_style_data;
		if(!style.empty())
		{
			//Turn on the style attributes
			curses_style_data = curses_styleAttributes(style);
			curses_wAttrOn(win, curses_style_data);
		}

		while(getline(in,line))
		{
			lineCutOffIndex = 0;

			while(true)
			{
				//Check to see if we need to wrap this line around
				if(indent + line.length() > width)
				{
					//Find the last space in the string that fits on the line
					lineCutOffIndex = line.rfind(" ", (indent + width));
					//If we can't find a space in the string and the string is too long, we just cut the line and wrap to the next line
					if(lineCutOffIndex == std::string::npos)
					{
						//Add as much of the line as we can to the output and then add a newline
						output = std::string(indent, ' ') + line.substr(0, (width-indent));
						mvwprintw(win,yMove,xMove,"%s", output.c_str());
						yMove++;
						//Continue looping until the rest of the line is added to the output
						line = line.substr(width-indent+1);
					}
					//If we find a space...
					else
					{
						//Add the part of the string before the cut off point to the output
						output = std::string(indent,' ') + line.substr(0, lineCutOffIndex);
						mvwprintw(win,yMove,xMove,"%s", output.c_str());
						yMove++;
						//Set the line equal to the cut off portion of the string and continue looping until the rest of the line is added to the output
						line = line.substr(lineCutOffIndex+1);
					}
				}
				else
				{
					output = line;
					mvwprintw(win, yMove, xMove, "%s", output.c_str());
					break;
				}
			}
			currLineNum++;
			//Add a new line when printing the next line from the string, or print a newline if we only have a newline character
			if(currLineNum < numberOfLines)
			{
				yMove++;
				wmove(win, yMove, xMove);
			}
		}

		//Turn off any styles we may have turned on for this function
		if(!style.empty())
		{
			curses_wAttrOff(win, curses_style_data);
		}
	}


	/**
	 * Given a vector of tokens, style and print each of them to a curses window.
	 * 
	 * Parameters:
	 * 
	 * 
	 * Returns:
	 * 	void
	*/
	void curses_wwrap_withTokens(	WINDOW * win,
									int yMove,
									int xMove,
									std::vector<s_TerminalPrintToken> tokens,
									unordered_map<std::string,std::string> style,
									unordered_map<std::string,std::string> format,
									bool textStyling	)
	{
		//Holds data for which attributes to use before printing text
		unordered_map<std::string, int> curses_attribute_data = {};
		//Process the style map's attributes and store them in a map that can be understood by curses
		unordered_map<std::string, int> style_attribute_data = curses_styleAttributes(style);
		/*** Formatting ***/
		//Set the level of indenting each time text wraps around
		int indent = 0;
		if(format.contains("indent"))
		{
			if(stevensStringLib::isInteger(format["indent"]))
			{
				indent = std::stoi(format["indent"]);
			}
		}
		//Get the window size that we're printing to
		int width;
		int height;
		//Keep track of the number of characters currently printed to the line
		int charsPrintedToLine = 0;
		getmaxyx(win, height, width); //Curses function to get the width of the window we are printing to
		//Set the yMove and xMove origins
		int yMoveOrigin = yMove;
		int xMoveOrigin = xMove;
		bool retainXMoveOnNewline = false;
		//retain xmove on newline - whenever a newline is input, start printing after the newline at xMoveOrigin
		if(format.contains("retain xmove on newline"))
		{
			if(stevensStringLib::stringToBool(format["retain xmove on newline"]))
			{
				retainXMoveOnNewline = true;
			}	
		}
		//Border adjustment
		bool avoidBorders = false;
		int borderAdjustment = 0;
		if(format.contains("avoid borders"))
		{
			//If we are set to avoid borders while printing, check that here
			if(stevensStringLib::stringToBool(format["avoid borders"]))
			{
				borderAdjustment = 1;
				avoidBorders = true;
			}
		}

		//Get the total amount of lines of content in all of the tokens that we're printing
		int totalLines = 0;
		int currLineNum = 0;
		for(int i = 0; i < tokens.size(); i++)
		{
			totalLines += stevensStringLib::countLines(tokens[i].content);
		}

		//For each token we are printing:
		for(int i = 0; i < tokens.size(); i++)
		{
			if(format.contains("debug"))
			{
				printw("Token #%d", i);
				std::cout << tokens[i].content << std::endl;
				getch();
			}
			
			std::istringstream in(tokens[i].content);
			std::string line;
			std::string output;
			int lineCutOffIndex = 0;
			int tokenLines = stevensStringLib::countLines(tokens[i].content);
			int currTokenLineNum = 0;
			//printw("lines:%d", numberOfLines);

			//Is the token specifically styled?
			if(tokens[i].styled)
			{
				curses_attribute_data = curses_styleToken(tokens[i]);
			}
			//Are there any styles to apply to all of the non-specifically styled tokens?
			else
			{
				curses_attribute_data = style_attribute_data;
			}

			s_TerminalPrintHelper::curses_wAttrOn(win, curses_attribute_data);

			//Print each line of the token
			while(getline(in,line))
			{
				lineCutOffIndex = 0;
				while(!line.empty())
				{
					// if(format.contains("debug"))
					// {
					// 	cout << line << std::endl;
					// 	getch();
					// }
					//printw("%s %ld ", "Line will finish printing at:", (xMove+line.length()));
					//printw("%s:%d / %s:%ld / %s:%d", "cursor x", xMove, "line length", line.length(), "width with adjustment", (width-borderAdjustment));
					//getch();
					//Check to see if we need to wrap this line around TODO: Get the current X position in the window and add it here. Make sure to account for window borders.
					if(((xMove-borderAdjustment) + line.length()) > (width - borderAdjustment))
					{
						// printw("%s", line.c_str());
						if(format.contains("debug"))
						{
							std::cout << "need to wrap" << std::endl;
							getch();
						}
						size_t maxLength = ((width-borderAdjustment)-xMove);
						if(maxLength > line.length()) maxLength = line.length();
						std::string bitOfLineThatCanFit = line.substr(0, maxLength);
						//Find the last space in the string that fits on the line
						lineCutOffIndex = bitOfLineThatCanFit.rfind(" ", (indent + width));
						if (lineCutOffIndex == std::string::npos) 
						{
							// Handle case where no space is found
							if (line.length() > (width - indent)) 
							{
								//output = std::string(indent, ' ') + line.substr(0, (width - indent));
								size_t outputLength = (width - xMove);
								if(outputLength > line.length()) outputLength = line.length();
								output = line.substr(0, outputLength);
								mvwprintw(win, yMove, xMove, "%s", output.c_str());
								yMove++;
								charsPrintedToLine = 0;
								size_t remainingStart = (width - xMove);
								if(remainingStart >= line.length()) {
									line.clear();
								} else {
									line = line.substr(remainingStart); // Safely truncate the line
								}
								// //Check to see if we want to retain the xMove for the next
								// if(retainXMoveOnNewline)
								// {
								// 	xMove = xMoveOrigin;
								// }
							} 
							else 
							{
								// If the line is shorter than the width, print it as is
								output = line;
								mvwprintw(win, yMove, xMove, "%s", output.c_str());
								line.clear(); // Clear the line to exit the loop
							}
						}
						else 
						{
							// Handle case where a space is found
							//output = std::string(indent, ' ') + line.substr(0, lineCutOffIndex);
							if(lineCutOffIndex <= line.length()) {
								output = line.substr(0, lineCutOffIndex);
							} else {
								output = line; // Use whole line if cutoff is beyond bounds
							}
							mvwprintw(win, yMove, xMove, "%s", output.c_str());
							yMove++;
							charsPrintedToLine = 0;
							if (lineCutOffIndex + 1 < line.length()) {
								line = line.substr(lineCutOffIndex + 1); // Safely truncate the line
							} else {
								line.clear(); // Clear the line to exit the loop
							}
							if(retainXMoveOnNewline)
							{
								xMove = xMoveOrigin;
								wmove(win, yMove, xMove);
							}
						}
						// lineCutOffIndex = bitOfLineThatCanFit.rfind(" ", (indent + width));
						// //If we can't find a space in the string and the string is too long, we just cut the line and wrap to the next line
						// if(lineCutOffIndex == std::string::npos)
						// {
						// 	//printw("Cannot find space, just breaking to next line...\n");
						// 	//Add as much of the line as we can to the output and then add a newline
						// 	if(format.contains("debug"))
						// 	{
						// 		std::cout << "printing as much of the line as we can" << std::endl;
						// 		getch();
						// 	}
						// 	output = std::string(indent, ' ') + line.substr(0, (width-indent));
						// 	mvwprintw(win,yMove,xMove,"%s", output.c_str());
						// 	//mvwprintw(win,yMove,xMove,"%s", "NEWLINE BY BREAK");
						// 	//getch();
						// 	yMove++;
						// 	charsPrintedToLine = 0;
						// 	//Continue looping until the rest of the line is added to the output
						// 	line = line.substr(width-indent);
						// }
						// //If we find a space...
						// else
						// {
						// 	if(format.contains("debug"))
						// 	{
						// 		std::cout << "found a space" << std::endl;
						// 		getch();
						// 	}
						// 	//Add the part of the string before the cut off point to the output
						// 	output = std::string(indent,' ') + line.substr(0, lineCutOffIndex);
						// 	mvwprintw(win, yMove, xMove, "%s", output.c_str());
						// 	//mvwprintw(win,yMove,xMove,"NEWLINE BY WRAP");
						// 	//getch();
						// 	//Move to the next line to continue printing
						// 	yMove++;
						// 	charsPrintedToLine = 0;
						// 	if(avoidBorders)
						// 	{
						// 		xMove = 1;
						// 	}
						// 	else
						// 	{
						// 		xMove = 0;
						// 	}
						// 	//Set the line equal to the cut off portion of the string and continue looping until the rest of the line is added to the output
						// 	line = line.substr(lineCutOffIndex+1);
						// }
					}
					else
					{
						// std::cout << "Printed a full line!" << std::endl;
						// getch();
						output = line;
						mvwprintw(win, yMove, xMove, "%s", output.c_str());
						break;
					}
				}
				currTokenLineNum++;
				currLineNum++;
				//Add a new line when printing the next line from the string, or print a newline if we only have a newline character
				if((currTokenLineNum <= tokenLines))
				{
					if(retainXMoveOnNewline)
					{
						xMove = xMoveOrigin;
					}
					else if(avoidBorders)
					{
						xMove = 1;
					}
					else
					{
						xMove = 0;
					}
					yMove++;
					wmove(win, yMove, xMove);
				}
			}
			//If the token content ends with a newline character, move the cursor
			// if(tokens[i].content.back() == '\n')
			// {
			// 	if(avoidBorders)
			// 	{
			// 		xMove = 1;
			// 	}
			// 	else
			// 	{
			// 		xMove = 0;
			// 	}
			// 	yMove++;
			// 	wmove(win, yMove, xMove);
			// }
			//After we're done prinnting all the lines of the token, turn off its attributes
			getyx(win, yMove, xMove);
			curses_wAttrOff(win, curses_attribute_data);
		}

		// std::cout << "after" << std::endl;
		// getch();
	}


	#endif
}
#endif