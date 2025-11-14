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
	 * Sets up color pairs for every combination of the default colors in the curses library:
	 * COLOR_BLACK 
     * COLOR_RED
     * COLOR_GREEN
     * COLOR_YELLOW
     * COLOR_BLUE
     * COLOR_MAGENTA
     * COLOR_CYAN
     * COLOR_WHITE
	 * 
	 * Also the extended bright colors:
	 * 8	- Bright black	(GREY)
	 * 9	- Bright red
	 * 10	- Bright green
	 * 11	- Bright yellow
	 * 12	- Bright blue	(AQUA)
	 * 13	- Bright magenta(PINK)
	 * 14	- Bright cyan
	 * 15 	- Bright white
	 * 
	 * Additionally, populate  the curses colors map.
	*/
	void curses_setup_colorPairs()
	{
		//Syntax is: init_pair(keyInCOLOR_PAIR, foregroundColor, backgroundColor)
	
		//If we support just one color pair, then we just do black on white
		if((COLORS >= 2) && (COLOR_PAIRS >= 1))
		{
			//DEFAULT COLOR PAIR
			init_pair(0, COLOR_WHITE,	COLOR_BLACK); 	curses_colors["default"] 			= 0;
		}
		else
		{
			//TODO: Work on this later, but we need to throw some kind of error if we can't support more than 2 colors in a terminal
			cout << "Error from s_TerminalPrintHelper - less than 2 colors available for use detected! Closing program." << endl;
			exit(EXIT_FAILURE);
		}
		//If we support two color pairs, we also do black on black
		if((COLOR_PAIRS >= 2))
		{
			init_pair(1, COLOR_BLACK,	COLOR_BLACK);	curses_colors["black_on_black"] 	= 1;
		}
		//If we support 8 color pairs, do the rest of of black backgounds
		if((COLORS >= 8) && (COLOR_PAIRS >= 8))
		{
			/*** Black background ***/
			init_pair(2, COLOR_RED,		COLOR_BLACK);	curses_colors["red_on_black"]		= 2;
			init_pair(3, COLOR_GREEN,	COLOR_BLACK);	curses_colors["green_on_black"] 	= 3;
			init_pair(4, COLOR_YELLOW,	COLOR_BLACK);	curses_colors["yellow_on_black"]	= 4;
			init_pair(5, COLOR_BLUE,	COLOR_BLACK);	curses_colors["blue_on_black"]		= 5;
			init_pair(6, COLOR_MAGENTA,	COLOR_BLACK);	curses_colors["magenta_on_black"]	= 6;
			init_pair(7, COLOR_CYAN,	COLOR_BLACK);	curses_colors["cyan_on_black"]		= 7;
			init_pair(8, COLOR_WHITE,	COLOR_BLACK);	curses_colors["white_on_black"]		= 8;
		}
		//If we support up to 16 color pairs, do red backgrounds
		if((COLORS >= 8) && (COLOR_PAIRS >= 16))
		{
			/*** Red background ***/
			init_pair(9,	COLOR_BLACK,	COLOR_RED);	curses_colors["black_on_red"]		= 9;
			init_pair(10,	COLOR_RED,		COLOR_RED);	curses_colors["red_on_red"]			= 10;
			init_pair(11,	COLOR_GREEN,	COLOR_RED);	curses_colors["green_on_red"]		= 11;
			init_pair(12,	COLOR_YELLOW,	COLOR_RED);	curses_colors["yellow_on_red"]		= 12;
			init_pair(13,	COLOR_BLUE,		COLOR_RED);	curses_colors["blue_on_red"]		= 13;
			init_pair(14,	COLOR_MAGENTA,	COLOR_RED);	curses_colors["magenta_on_red"]		= 14;
			init_pair(15,	COLOR_CYAN,		COLOR_RED);	curses_colors["cyan_on_red"]		= 15;
			init_pair(16,	COLOR_WHITE,	COLOR_RED);	curses_colors["white_on_red"]		= 16;
		}
		//If we support up to 64 color pairs, do the rest of the standard 8 color backgrounds
		if((COLORS >= 8) && (COLOR_PAIRS >= 64))
		{
			/*** Green Background ***/
			init_pair(17,	COLOR_BLACK,	COLOR_GREEN); curses_colors["black_on_green"]	= 17;
			init_pair(18,	COLOR_RED,		COLOR_GREEN); curses_colors["red_on_green"]		= 18;
			init_pair(19,	COLOR_GREEN,	COLOR_GREEN); curses_colors["green_on_green"]	= 19;
			init_pair(20,	COLOR_YELLOW,	COLOR_GREEN); curses_colors["yellow_on_green"]	= 20;
			init_pair(21,	COLOR_BLUE,		COLOR_GREEN); curses_colors["blue_on_green"]	= 21;
			init_pair(22,	COLOR_MAGENTA,	COLOR_GREEN); curses_colors["magenta_on_green"]	= 22;
			init_pair(23,	COLOR_CYAN,		COLOR_GREEN); curses_colors["cyan_on_green"]	= 23;
			init_pair(24,	COLOR_WHITE,	COLOR_GREEN); curses_colors["white_on_green"]	= 24;
			/*** Yellow background ***/
			init_pair(25,	COLOR_BLACK,	COLOR_YELLOW); curses_colors["black_on_yellow"] = 25;
			init_pair(26,	COLOR_RED,		COLOR_YELLOW); curses_colors["red_on_yellow"]	= 26;
			init_pair(27,	COLOR_GREEN,	COLOR_YELLOW); curses_colors["green_on_yellow"]	= 27;
			init_pair(28,	COLOR_YELLOW,	COLOR_YELLOW); curses_colors["yellow_on_yellow"]= 28;
			init_pair(29,	COLOR_BLUE,		COLOR_YELLOW); curses_colors["blue_on_yellow"]	= 29;
			init_pair(30,	COLOR_MAGENTA,	COLOR_YELLOW); curses_colors["magenta_on_yellow"]=30;
			init_pair(31,	COLOR_CYAN,		COLOR_YELLOW); curses_colors["cyan_on_yellow"]	= 31;
			init_pair(32,	COLOR_WHITE,	COLOR_YELLOW); curses_colors["white_on_yellow"]	= 32;
			/*** Blue background ***/
			init_pair(33,	COLOR_BLACK,	COLOR_BLUE); curses_colors["black_on_blue"]		= 33;
			init_pair(34,	COLOR_RED,		COLOR_BLUE); curses_colors["red_on_blue"]		= 34;
			init_pair(35,	COLOR_GREEN,	COLOR_BLUE); curses_colors["green_on_blue"]		= 35;
			init_pair(36,	COLOR_YELLOW,	COLOR_BLUE); curses_colors["yellow_on_blue"]	= 36;
			init_pair(37,	COLOR_BLUE,		COLOR_BLUE); curses_colors["blue_on_blue"]		= 37;
			init_pair(38,	COLOR_MAGENTA,	COLOR_BLUE); curses_colors["magenta_on_blue"]	= 38;
			init_pair(39,	COLOR_CYAN,		COLOR_BLUE); curses_colors["cyan_on_blue"]		= 39;
			init_pair(40,	COLOR_WHITE,	COLOR_BLUE); curses_colors["white_on_blue"]		= 40;
			/*** Magenta background ***/
			init_pair(41,	COLOR_BLACK,	COLOR_MAGENTA);	curses_colors["black_on_magenta"]	= 41;
			init_pair(42,	COLOR_RED,		COLOR_MAGENTA);	curses_colors["red_on_magenta"]		= 42;
			init_pair(43,	COLOR_GREEN,	COLOR_MAGENTA);	curses_colors["green_on_magenta"]	= 43;
			init_pair(44,	COLOR_YELLOW,	COLOR_MAGENTA);	curses_colors["yellow_on_magenta"]	= 44;
			init_pair(45,	COLOR_BLUE,		COLOR_MAGENTA); curses_colors["blue_on_magenta"]	= 45;
			init_pair(46,	COLOR_MAGENTA,	COLOR_MAGENTA); curses_colors["magenta_on_magenta"]	= 46;
			init_pair(47,	COLOR_CYAN,		COLOR_MAGENTA); curses_colors["cyan_on_magenta"]	= 47;
			init_pair(48,	COLOR_WHITE,	COLOR_MAGENTA); curses_colors["white_on_magenta"]	= 48;
			/*** Cyan background ***/
			init_pair(49,	COLOR_BLACK,	COLOR_CYAN);	curses_colors["black_on_cyan"]	= 49;
			init_pair(50,	COLOR_RED,		COLOR_CYAN);	curses_colors["red_on_cyan"]	= 50;
			init_pair(51,	COLOR_GREEN,	COLOR_CYAN);	curses_colors["green_on_cyan"]	= 51;
			init_pair(52,	COLOR_YELLOW,	COLOR_CYAN);	curses_colors["yellow_on_cyan"]	= 52;
			init_pair(53,	COLOR_BLUE,		COLOR_CYAN);	curses_colors["blue_on_cyan"]	= 53;
			init_pair(54,	COLOR_MAGENTA,	COLOR_CYAN);	curses_colors["magenta_on_cyan"]= 54;
			init_pair(55,	COLOR_CYAN,		COLOR_CYAN);	curses_colors["cyan_on_cyan"]	= 55;
			init_pair(56,	COLOR_WHITE,	COLOR_CYAN);	curses_colors["white_on_cyan"]	= 56;
			/*** White background ***/
			init_pair(57,	COLOR_BLACK,	COLOR_WHITE);	curses_colors["black_on_white"]	= 57;
			init_pair(58,	COLOR_RED,		COLOR_WHITE);	curses_colors["red_on_white"]	= 58;
			init_pair(59,	COLOR_GREEN,	COLOR_WHITE);	curses_colors["green_on_white"]	= 59;
			init_pair(60,	COLOR_YELLOW,	COLOR_WHITE);	curses_colors["yellow_on_white"]= 60;
			init_pair(61,	COLOR_BLUE,		COLOR_WHITE);	curses_colors["blue_on_white"]	= 61;
			init_pair(62,	COLOR_MAGENTA,	COLOR_WHITE);	curses_colors["magenta_on_white"]=62;
			init_pair(63,	COLOR_CYAN,		COLOR_WHITE);	curses_colors["cyan_on_white"]	= 63;
			init_pair(64,	COLOR_WHITE,	COLOR_WHITE);	curses_colors["white_on_white"]	= 64;
		}
		//If we support up to 16 colors and 128 color pairs, add pairs for colors 8-15 as foregrounds on the 0-7 backgrounds
		if((COLORS >= 16) && (COLOR_PAIRS >= 128))
		{
			/***** Additional 8 Colors *****/
			/*** Black background - 8 more colors ***/
			init_pair(65,	8,		COLOR_BLACK);	curses_colors["bright-black_on_black"]	=	65;		curses_colors["grey_on_black"]	=	65;
			init_pair(66,	9,		COLOR_BLACK);	curses_colors["bright-red_on_black"]	=	66;
			init_pair(67,	10,		COLOR_BLACK);	curses_colors["bright-green_on_black"]	=	67;
			init_pair(68,	11,		COLOR_BLACK);	curses_colors["bright-yellow_on_black"] =	68;
			init_pair(69,	12,		COLOR_BLACK);	curses_colors["bright-blue_on_black"]	=	69;		curses_colors["aqua_on_black"]	=	69;
			init_pair(70,	13,		COLOR_BLACK);	curses_colors["bright-magenta_on_black"]=	70;		curses_colors["pink_on_black"]	=	70;
			init_pair(71,	14,		COLOR_BLACK);	curses_colors["bright-cyan_on_black"]	=	71;
			init_pair(72,	15,		COLOR_BLACK);	curses_colors["bright-white_on_black"] 	= 	72;
			/*** Red background - 8 more colors ***/
			init_pair(73,	8,		COLOR_RED);	curses_colors["bright-black_on_red"]		=	73;		curses_colors["grey_on_red"]	=	73;
			init_pair(74,	9,		COLOR_RED);	curses_colors["bright-red_on_red"]			=	74;
			init_pair(75,	10,		COLOR_RED);	curses_colors["bright-green_on_red"]		=	75;
			init_pair(76,	11,		COLOR_RED);	curses_colors["bright-yellow_on_red"] 		=	76;
			init_pair(77,	12,		COLOR_RED);	curses_colors["bright-blue_on_red"]			=	77;		curses_colors["aqua_on_red"]	=	77;
			init_pair(78,	13,		COLOR_RED);	curses_colors["bright-magenta_on_red"]		=	78;		curses_colors["pink_on_red"]	=	78;
			init_pair(79,	14,		COLOR_RED);	curses_colors["bright-cyan_on_red"]			=	79;
			init_pair(80,	15,		COLOR_RED);	curses_colors["bright-white_on_red"] 		= 	80;
			/*** Green background - 8 more colors ***/
			init_pair(81,	8,		COLOR_GREEN);	curses_colors["bright-black_on_green"]	=	81;		curses_colors["grey_on_red"]	=	81;
			init_pair(82,	9,		COLOR_GREEN);	curses_colors["bright-red_on_green"]	=	82;
			init_pair(83,	10,		COLOR_GREEN);	curses_colors["bright-green_on_green"]	=	83;
			init_pair(84,	11,		COLOR_GREEN);	curses_colors["bright-yellow_on_green"] =	84;
			init_pair(85,	12,		COLOR_GREEN);	curses_colors["bright-blue_on_green"]	=	85;		curses_colors["aqua_on_red"]	=	85;
			init_pair(86,	13,		COLOR_GREEN);	curses_colors["bright-magenta_on_green"]=	86;		curses_colors["pink_on_red"]	=	86;
			init_pair(87,	14,		COLOR_GREEN);	curses_colors["bright-cyan_on_green"]	=	87;
			init_pair(88,	15,		COLOR_GREEN);	curses_colors["bright-white_on_green"] 	= 	88;
			/*** Yellow background - 8 more colors ***/
			init_pair(89,	8,		COLOR_YELLOW);	curses_colors["bright-black_on_yellow"]		=	89;		curses_colors["grey_on_yellow"]	=	89;
			init_pair(90,	9,		COLOR_YELLOW);	curses_colors["bright-red_on_yellow"]		=	90;
			init_pair(91,	10,		COLOR_YELLOW);	curses_colors["bright-green_on_yellow"]		=	91;
			init_pair(92,	11,		COLOR_YELLOW);	curses_colors["bright-yellow_on_yellow"]	=	92;
			init_pair(93,	12,		COLOR_YELLOW);	curses_colors["bright-blue_on_yellow"]		=	93;		curses_colors["aqua_on_yellow"]	=	93;
			init_pair(94,	13,		COLOR_YELLOW);	curses_colors["bright-magenta_on_yellow"]	=	94;		curses_colors["pink_on_yellow"]	=	94;
			init_pair(95,	14,		COLOR_YELLOW);	curses_colors["bright-cyan_on_yellow"]		=	95;
			init_pair(96,	15,		COLOR_YELLOW);	curses_colors["bright-white_on_yellow"] 	= 	96;
			/*** Blue background - 8 more colors ***/
			init_pair(97,	8,		COLOR_BLUE);	curses_colors["bright-black_on_blue"]		=	97;		curses_colors["grey_on_blue"]	=	97;
			init_pair(98,	9,		COLOR_BLUE);	curses_colors["bright-red_on_blue"]			=	98;
			init_pair(99,	10,		COLOR_BLUE);	curses_colors["bright-green_on_blue"]		=	99;
			init_pair(100,	11,		COLOR_BLUE);	curses_colors["bright-yellow_on_blue"]		=	100;
			init_pair(101,	12,		COLOR_BLUE);	curses_colors["bright-blue_on_blue"]		=	101;	curses_colors["aqua_on_blue"]	=	101;
			init_pair(102,	13,		COLOR_BLUE);	curses_colors["bright-magenta_on_blue"]		=	102;	curses_colors["pink_on_blue"]	=	102;
			init_pair(103,	14,		COLOR_BLUE);	curses_colors["bright-cyan_on_blue"]		=	103;
			init_pair(104,	15,		COLOR_BLUE);	curses_colors["bright-white_on_blue"] 		= 	104;
			/*** Magenta background - 8 more colors ***/
			init_pair(105,	8,		COLOR_MAGENTA);	curses_colors["bright-black_on_magenta"]	=	105;	curses_colors["grey_on_magenta"]	=	105;
			init_pair(106,	9,		COLOR_MAGENTA);	curses_colors["bright-red_on_magenta"]		=	106;
			init_pair(107,	10,		COLOR_MAGENTA);	curses_colors["bright-green_on_magenta"]	=	107;
			init_pair(108,	11,		COLOR_MAGENTA);	curses_colors["bright-yellow_on_magenta"]	=	108;
			init_pair(109,	12,		COLOR_MAGENTA);	curses_colors["bright-blue_on_magenta"]		=	109;	curses_colors["aqua_on_magenta"]	=	109;
			init_pair(110,	13,		COLOR_MAGENTA);	curses_colors["bright-magenta_on_magenta"]	=	110;	curses_colors["pink_on_magenta"]	=	110;
			init_pair(111,	14,		COLOR_MAGENTA);	curses_colors["bright-cyan_on_magenta"]		=	111;
			init_pair(112,	15,		COLOR_MAGENTA);	curses_colors["bright-white_on_blue"] 		= 	112;
			/*** Cyan background - 8 more colors ***/
			init_pair(113,	8,		COLOR_CYAN);	curses_colors["bright-black_on_cyan"]		=	113;	curses_colors["grey_on_cyan"]	=	113;
			init_pair(114,	9,		COLOR_CYAN);	curses_colors["bright-red_on_cyan"]			=	114;
			init_pair(115,	10,		COLOR_CYAN);	curses_colors["bright-green_on_cyan"]		=	115;
			init_pair(116,	11,		COLOR_CYAN);	curses_colors["bright-yellow_on_cyan"]		=	116;
			init_pair(117,	12,		COLOR_CYAN);	curses_colors["bright-blue_on_cyan"]		=	117;	curses_colors["aqua_on_cyan"]	=	117;
			init_pair(118,	13,		COLOR_CYAN);	curses_colors["bright-magenta_on_cyan"]		=	118;	curses_colors["pink_on_cyan"]	=	118;
			init_pair(119,	14,		COLOR_CYAN);	curses_colors["bright-cyan_on_cyan"]		=	119;
			init_pair(120,	15,		COLOR_CYAN);	curses_colors["bright-white_on_cyan"] 		= 	120;
			/*** White background - 8 more colors ***/
			init_pair(121,	8,		COLOR_WHITE);	curses_colors["bright-black_on_white"]		=	121;	curses_colors["grey_on_white"]	=	121;
			init_pair(122,	9,		COLOR_WHITE);	curses_colors["bright-red_on_white"]		=	122;
			init_pair(123,	10,		COLOR_WHITE);	curses_colors["bright-green_on_white"]		=	123;
			init_pair(124,	11,		COLOR_WHITE);	curses_colors["bright-yellow_on_white"]		=	124;
			init_pair(125,	12,		COLOR_WHITE);	curses_colors["bright-blue_on_white"]		=	125;	curses_colors["aqua_on_white"]	=	125;
			init_pair(126,	13,		COLOR_WHITE);	curses_colors["bright-magenta_on_white"]	=	126;	curses_colors["pink_on_white"]	=	126;
			init_pair(127,	14,		COLOR_WHITE);	curses_colors["bright-cyan_on_white"]		=	127;
			init_pair(128,	15,		COLOR_WHITE);	curses_colors["bright-white_on_white"] 		= 	128;
		}
		/***** Create a color pair for each one of the 8 additional brightened colors as background *****/
		if((COLORS >= 16) && (COLOR_PAIRS >= 256))
		{
			/*** Bright-black (GREY) background ***/
			init_pair(129,	COLOR_BLACK,	8);	curses_colors["black_on_bright-black"]		= 129;		curses_colors["black_on_grey"]	=	129;
			init_pair(130,	COLOR_RED,		8);	curses_colors["red_on_bright-black"]		= 130;		curses_colors["red_on_grey"]	=	130;
			init_pair(131,	COLOR_GREEN,	8);	curses_colors["green_on_bright-black"]		= 131;		curses_colors["green_on_grey"]	=	131;
			init_pair(132,	COLOR_YELLOW,	8);	curses_colors["yellow_on_bright-black"]		= 132;		curses_colors["yellow_on_grey"]	=	132;
			init_pair(133,	COLOR_BLUE,		8);	curses_colors["blue_on_bright-black"]		= 133;		curses_colors["blue_on_grey"]	=	133;
			init_pair(134,	COLOR_MAGENTA,	8);	curses_colors["magenta_on_bright-black"]	= 134;		curses_colors["magenta_on_grey"]=	134;
			init_pair(135,	COLOR_CYAN,		8);	curses_colors["cyan_on_bright-black"]		= 135;		curses_colors["cyan_on_grey"]	=	135;
			init_pair(136,	COLOR_WHITE,	8);	curses_colors["white_on_bright-black"]		= 136;		curses_colors["white_on_grey"]	=	136;
			init_pair(137,	8,		8);			curses_colors["bright-black_on_bright-black"]		= 137;		curses_colors["bright-black_on_grey"]	=	137;		curses_colors["grey_on_grey"]	=	137;		curses_colors["grey_on_bright-black"]	=	137;
			init_pair(138,	9,		8);			curses_colors["bright-red_on_bright-black"]			= 138;		curses_colors["bright-red_on_grey"]		=	138;
			init_pair(139,	10,		8);			curses_colors["bright-green_on_bright-black"]		= 139;		curses_colors["bright-green_on_grey"]	=	139;
			init_pair(140,	11,		8);			curses_colors["bright-yellow_on_bright-black"]		= 140;		curses_colors["bright-yellow_on_grey"]	=	140;		
			init_pair(141,	12,		8);			curses_colors["bright-blue_on_bright-black"]		= 141;		curses_colors["bright-blue_on_grey"]	=	141;		curses_colors["aqua_on_grey"]	=	141;		curses_colors["aqua_on_bright-black"]	=	141;
			init_pair(142,	13,		8);			curses_colors["bright-magenta_on_bright-black"]		= 142;		curses_colors["bright-magenta_on_grey"]	=	142;		curses_colors["pink_on_grey"]	=	142;		curses_colors["pink_on_bright_black"]	=	142;
			init_pair(143,	14,		8);			curses_colors["bright-cyan_on_bright-black"]		= 143;		curses_colors["bright-cyan_on_grey"]	=	143;
			init_pair(144,	15,		8);			curses_colors["bright-white_on_bright-black"]		= 144;		curses_colors["bright-white_on_grey"]	=	144;
			/*** Bright-red background ***/
			init_pair(145,	COLOR_BLACK,	9);	curses_colors["black_on_bright-red"]		= 145;
			init_pair(146,	COLOR_RED,		9);	curses_colors["red_on_bright-red"]			= 146;
			init_pair(147,	COLOR_GREEN,	9);	curses_colors["green_on_bright-red"]		= 147;
			init_pair(148,	COLOR_YELLOW,	9);	curses_colors["yellow_on_bright-red"]		= 148;
			init_pair(149,	COLOR_BLUE,		9);	curses_colors["blue_on_bright-red"]			= 149;
			init_pair(150,	COLOR_MAGENTA,	9);	curses_colors["magenta_on_bright-red"]		= 150;
			init_pair(151,	COLOR_CYAN,		9);	curses_colors["cyan_on_bright-red"]			= 151;
			init_pair(152,	COLOR_WHITE,	9);	curses_colors["white_on_bright-red"]		= 152;
			init_pair(153,	8,		9);			curses_colors["bright-black_on_bright-red"]			= 153;		curses_colors["grey_on_bright-red"]	=	153;
			init_pair(154,	9,		9);			curses_colors["bright-red_on_bright-red"]			= 154;
			init_pair(155,	10,		9);			curses_colors["bright-green_on_bright-red"]			= 155;
			init_pair(156,	11,		9);			curses_colors["bright-yellow_on_bright-red"]		= 156;
			init_pair(157,	12,		9);			curses_colors["bright-blue_on_bright-red"]			= 157;		curses_colors["aqua_on_bright-red"]	=	157;
			init_pair(158,	13,		9);			curses_colors["bright-magenta_on_bright-red"]		= 158;		curses_colors["pink_on_bright-red"]	=	158;
			init_pair(159,	14,		9);			curses_colors["bright-cyan_on_bright-red"]			= 159;
			init_pair(160,	15,		9);			curses_colors["bright-white_on_bright-red"]			= 160;
			/*** Bright-green background ***/
			init_pair(161,	COLOR_BLACK,	10);	curses_colors["black_on_bright-green"]		= 161;
			init_pair(162,	COLOR_RED,		10);	curses_colors["red_on_bright-green"]		= 162;
			init_pair(163,	COLOR_GREEN,	10);	curses_colors["green_on_bright-green"]		= 163;
			init_pair(164,	COLOR_YELLOW,	10);	curses_colors["yellow_on_bright-green"]		= 164;
			init_pair(165,	COLOR_BLUE,		10);	curses_colors["blue_on_bright-green"]		= 165;
			init_pair(166,	COLOR_MAGENTA,	10);	curses_colors["magenta_on_bright-green"]	= 166;
			init_pair(167,	COLOR_CYAN,		10);	curses_colors["cyan_on_bright-green"]		= 167;
			init_pair(168,	COLOR_WHITE,	10);	curses_colors["white_on_bright-green"]		= 168;
			init_pair(169,	8,		10);	curses_colors["bright-black_on_bright-green"]		= 169;		curses_colors["grey_on_bright-green"]	=	169;
			init_pair(170,	9,		10);	curses_colors["bright-red_on_bright-green"]			= 170;
			init_pair(171,	10,		10);	curses_colors["bright-green_on_bright-green"]		= 171;
			init_pair(172,	11,		10);	curses_colors["bright-yellow_on_bright-green"]		= 172;
			init_pair(173,	12,		10);	curses_colors["bright-blue_on_bright-green"]		= 173;		curses_colors["aqua_on_bright-green"]	=	173;
			init_pair(174,	13,		10);	curses_colors["bright-magenta_on_bright-green"]		= 174;		curses_colors["pink_on_bright-green"]	=	174;
			init_pair(175,	14,		10);	curses_colors["bright-cyan_on_bright-green"]		= 175;
			init_pair(176,	15,		10);	curses_colors["bright-white_on_bright-green"]		= 176;
			/*** Bright-yellow background ***/
			init_pair(177,	COLOR_BLACK,	11);	curses_colors["black_on_bright-yellow"]		= 177;
			init_pair(178,	COLOR_RED,		11);	curses_colors["red_on_bright-yellow"]		= 178;
			init_pair(179,	COLOR_GREEN,	11);	curses_colors["green_on_bright-yellow"]		= 179;
			init_pair(180,	COLOR_YELLOW,	11);	curses_colors["yellow_on_bright-yellow"]	= 180;
			init_pair(181,	COLOR_BLUE,		11);	curses_colors["blue_on_bright-yellow"]		= 181;
			init_pair(182,	COLOR_MAGENTA,	11);	curses_colors["magenta_on_bright-yellow"]	= 182;
			init_pair(183,	COLOR_CYAN,		11);	curses_colors["cyan_on_bright-yellow"]		= 183;
			init_pair(184,	COLOR_WHITE,	11);	curses_colors["white_on_bright-yellow"]		= 184;
			init_pair(185,	8,		11);	curses_colors["bright-black_on_bright-yellow"]		= 185;		curses_colors["grey_on_bright-yellow"]	=	185;
			init_pair(187,	9,		11);	curses_colors["bright-red_on_bright-yellow"]		= 186;
			init_pair(187,	10,		11);	curses_colors["bright-green_on_bright-yellow"]		= 187;
			init_pair(188,	11,		11);	curses_colors["bright-yellow_on_bright-yellow"]		= 188;
			init_pair(189,	12,		11);	curses_colors["bright-blue_on_bright-yellow"]		= 189;		curses_colors["aqua_on_bright-yellow"]	=	189;
			init_pair(190,	13,		11);	curses_colors["bright-magenta_on_bright-yellow"]	= 190;		curses_colors["pink_on_bright-yellow"]	=	190;
			init_pair(191,	14,		11);	curses_colors["bright-cyan_on_bright-yellow"]		= 191;
			init_pair(192,	15,		11);	curses_colors["bright-white_on_bright-yellow"]		= 192;
			/*** Bright-blue (aqua) background ***/
			init_pair(193,	COLOR_BLACK,	12);	curses_colors["black_on_bright-blue"]		= 193;		curses_colors["black_on_aqua"]	=	193;
			init_pair(194,	COLOR_RED,		12);	curses_colors["red_on_bright-blue"]			= 194;		curses_colors["red_on_aqua"]	=	194;
			init_pair(195,	COLOR_GREEN,	12);	curses_colors["green_on_bright-blue"]		= 195;		curses_colors["green_on_aqua"]	=	195;
			init_pair(196,	COLOR_YELLOW,	12);	curses_colors["yellow_on_bright-blue"]		= 196;		curses_colors["yellow_on_aqua"]	=	196;
			init_pair(197,	COLOR_BLUE,		12);	curses_colors["blue_on_bright-blue"]		= 197;		curses_colors["blue_on_aqua"]	=	197;
			init_pair(198,	COLOR_MAGENTA,	12);	curses_colors["magenta_on_bright-blue"]		= 198;		curses_colors["magenta_on_aqua"]=	198;
			init_pair(199,	COLOR_CYAN,		12);	curses_colors["cyan_on_bright-blue"]		= 199;		curses_colors["cyan_on_aqua"]	=	199;
			init_pair(200,	COLOR_WHITE,	12);	curses_colors["white_on_bright-blue"]		= 200;		curses_colors["white_on_aqua"]	=	200;
			init_pair(201,	8,		12);			curses_colors["bright-black_on_bright-blue"]		= 201;		curses_colors["bright-black_on_aqua"]	=	201;		curses_colors["grey_on_aqua"]	=	201;		curses_colors["grey_on_bright-blue"]	=	201;
			init_pair(202,	9,		12);			curses_colors["bright-red_on_bright-blue"]			= 202;		curses_colors["bright-red_on_aqua"]		=	202;
			init_pair(203,	10,		12);			curses_colors["bright-green_on_bright-blue"]		= 203;		curses_colors["bright-green_on_aqua"]	=	203;
			init_pair(204,	11,		12);			curses_colors["bright-yellow_on_bright-blue"]		= 204;		curses_colors["bright-yellow_on_aqua"]	=	204;		
			init_pair(205,	12,		12);			curses_colors["bright-blue_on_bright-blue"]			= 205;		curses_colors["bright-blue_on_aqua"]	=	205;		curses_colors["aqua_on_aqua"]	=	205;		curses_colors["aqua_on_bright-blue"]	=	205;
			init_pair(206,	13,		12);			curses_colors["bright-magenta_on_bright-blue"]		= 206;		curses_colors["bright-magenta_on_aqua"]	=	206;		curses_colors["pink_on_aqua"]	=	206;		curses_colors["pink_on_bright_blue"]	=	206;
			init_pair(207,	14,		12);			curses_colors["bright-cyan_on_bright-blue"]			= 207;		curses_colors["bright-cyan_on_aqua"]	=	207;
			init_pair(208,	15,		12);			curses_colors["bright-white_on_bright-blue"]		= 208;		curses_colors["bright-white_on_aqua"]	=	208;
			/*** Bright-magenta (pink) background ***/
			init_pair(209,	COLOR_BLACK,	13);	curses_colors["black_on_bright-magenta"]		= 209;		curses_colors["black_on_pink"]	=	209;
			init_pair(210,	COLOR_RED,		13);	curses_colors["red_on_bright-magenta"]			= 210;		curses_colors["red_on_pink"]	=	210;
			init_pair(211,	COLOR_GREEN,	13);	curses_colors["green_on_bright-magenta"]		= 211;		curses_colors["green_on_pink"]	=	211;
			init_pair(212,	COLOR_YELLOW,	13);	curses_colors["yellow_on_bright-magenta"]		= 212;		curses_colors["yellow_on_pink"]	=	212;
			init_pair(213,	COLOR_BLUE,		13);	curses_colors["blue_on_bright-magenta"]			= 213;		curses_colors["blue_on_pink"]	=	213;
			init_pair(214,	COLOR_MAGENTA,	13);	curses_colors["magenta_on_bright-magenta"]		= 214;		curses_colors["magenta_on_pink"]=	214;
			init_pair(215,	COLOR_CYAN,		13);	curses_colors["cyan_on_bright-magenta"]			= 215;		curses_colors["cyan_on_pink"]	=	215;
			init_pair(216,	COLOR_WHITE,	13);	curses_colors["white_on_bright-magenta"]		= 216;		curses_colors["white_on_pink"]	=	216;
			init_pair(217,	8,		13);			curses_colors["bright-black_on_bright-magenta"]	= 217;		curses_colors["bright-black_on_pink"]	=	217;		curses_colors["grey_on_pink"]	=	217;		curses_colors["grey_on_bright-magenta"]	=	217;
			init_pair(218,	9,		13);			curses_colors["bright-red_on_bright-magenta"]	= 218;		curses_colors["bright-red_on_pink"]		=	218;
			init_pair(219,	10,		13);			curses_colors["bright-green_on_bright-magenta"]	= 219;		curses_colors["bright-green_on_pink"]	=	219;
			init_pair(220,	11,		13);			curses_colors["bright-yellow_on_bright-magenta"]= 220;		curses_colors["bright-yellow_on_pink"]	=	220;		
			init_pair(221,	12,		13);			curses_colors["bright-blue_on_bright-magenta"]	= 221;		curses_colors["bright-blue_on_pink"]	=	221;		curses_colors["aqua_on_pink"]	=	221;		curses_colors["aqua_on_bright-magenta"]	=	221;
			init_pair(222,	13,		13);			curses_colors["bright-magenta_on_bright-magenta"]	= 222;		curses_colors["bright-magenta_on_pink"]	=	222;		curses_colors["pink_on_pink"]	=	222;		curses_colors["pink_on_bright_blue"]	=	222;
			init_pair(223,	14,		13);			curses_colors["bright-cyan_on_bright-magenta"]	= 223;		curses_colors["bright-cyan_on_pink"]	=	223;
			init_pair(224,	15,		13);			curses_colors["bright-white_on_bright-magenta"]	= 224;		curses_colors["bright-white_on_pink"]	=	224;
			/*** Bright-cyan background ***/
			init_pair(225,	COLOR_BLACK,	14);	curses_colors["black_on_bright-cyan"]		= 225;
			init_pair(226,	COLOR_RED,		14);	curses_colors["red_on_bright-cyan"]			= 226;
			init_pair(227,	COLOR_GREEN,	14);	curses_colors["green_on_bright-cyan"]		= 227;
			init_pair(228,	COLOR_YELLOW,	14);	curses_colors["yellow_on_bright-cyan"]		= 228;
			init_pair(229,	COLOR_BLUE,		14);	curses_colors["blue_on_bright-cyan"]		= 229;
			init_pair(230,	COLOR_MAGENTA,	14);	curses_colors["magenta_on_bright-cyan"]		= 230;
			init_pair(231,	COLOR_CYAN,		14);	curses_colors["cyan_on_bright-cyan"]		= 231;
			init_pair(232,	COLOR_WHITE,	14);	curses_colors["white_on_bright-cyan"]		= 232;
			init_pair(233,	8,		14);			curses_colors["bright-black_on_bright-cyan"]		= 233;		curses_colors["grey_on_bright-cyan"]	=	233;
			init_pair(234,	9,		14);			curses_colors["bright-red_on_bright-cyan"]			= 234;
			init_pair(235,	10,		14);			curses_colors["bright-green_on_bright-cyan"]		= 235;
			init_pair(236,	11,		14);			curses_colors["bright-yellow_on_bright-cyan"]		= 236;
			init_pair(237,	12,		14);			curses_colors["bright-blue_on_bright-cyan"]			= 237;		curses_colors["aqua_on_bright-cyan"]	=	237;
			init_pair(238,	13,		14);			curses_colors["bright-magenta_on_bright-cyan"]		= 238;		curses_colors["pink_on_bright-cyan"]	=	238;
			init_pair(239,	14,		14);			curses_colors["bright-cyan_on_bright-cyan"]			= 239;
			init_pair(240,	15,		14);			curses_colors["bright-white_on_bright-cyan"]		= 240;
			/***  Bright-white backgroudn ***/
			init_pair(241,	COLOR_BLACK,	15);	curses_colors["black_on_bright-white"]		= 241;
			init_pair(242,	COLOR_RED,		15);	curses_colors["red_on_bright-white"]		= 242;
			init_pair(243,	COLOR_GREEN,	15);	curses_colors["green_on_bright-white"]		= 243;
			init_pair(244,	COLOR_YELLOW,	15);	curses_colors["yellow_on_bright-white"]		= 244;
			init_pair(245,	COLOR_BLUE,		15);	curses_colors["blue_on_bright-white"]		= 245;
			init_pair(246,	COLOR_MAGENTA,	15);	curses_colors["magenta_on_bright-white"]	= 246;
			init_pair(247,	COLOR_CYAN,		15);	curses_colors["cyan_on_bright-white"]		= 247;
			init_pair(248,	COLOR_WHITE,	15);	curses_colors["white_on_bright-white"]		= 248;
			init_pair(249,	8,		15);			curses_colors["bright-black_on_bright-white"]		= 249;		curses_colors["grey_on_bright-white"]	=	249;
			init_pair(250,	9,		15);			curses_colors["bright-red_on_bright-white"]			= 250;
			init_pair(251,	10,		15);			curses_colors["bright-green_on_bright-white"]		= 251;
			init_pair(252,	11,		15);			curses_colors["bright-yellow_on_bright-white"]		= 252;
			init_pair(253,	12,		15);			curses_colors["bright-blue_on_bright-white"]		= 253;		curses_colors["aqua_on_bright-white"]	=	253;
			init_pair(254,	13,		15);			curses_colors["bright-magenta_on_bright-white"]		= 254;		curses_colors["pink_on_bright-white"]	=	254;
			init_pair(255,	14,		15);			curses_colors["bright-cyan_on_bright-white"]		= 255;
			init_pair(256,	15,		15);			curses_colors["bright-white_on_bright-white"]		= 256;
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