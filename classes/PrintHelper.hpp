#pragma once
/**
 * Class containing useful functions for managing printing to the terminal and text
 * styling with the stevensTerminal library.
 */


#include <sstream>
#include <fstream>

namespace stevensTerminal
{
namespace PrintHelper
{
	/*** Methods ***/
	// PROTOTYPES //
	inline std::vector<PrintToken> tokenizePrintString(	std::string & input	);
	inline std::string ignoreTokenStyling(	std::string str,
									std::vector<PrintToken> tokensToStyle	);
	inline void wrap(	std::string const &input,
				int width,
				std::ostream &os,
				size_t indent,
				bool newLines	);
	inline void curses_wwrap(	WINDOW * win,
						int yMove,
						int xMove,
						std::string printString,
						size_t indent = 0,
						std::unordered_map<std::string,std::string> style = {});
	inline void curses_wwrap_withTokens(	WINDOW * win,
									int yMove,
									int xMove,
									std::vector<PrintToken> tokens,
									std::unordered_map<std::string,std::string> style,
									std::unordered_map<std::string,std::string> format,
									bool textStyling	);
	inline std::vector<std::string> computeWrapSegments(
									std::string line,
									int xMove,
									int width,
									int borderAdjustment = 0);
	// END PROTOTYPES //
	
	
	// NOTE: The ANSI-escape-code-based styleText()/print() path (Linux/macOS only)
	// and its helpers styleNonTokenText()/styleTokens() were removed — cultgame
	// exclusively uses the curses-based curses_wprint() path (curses_styleToken()
	// below), which renders color via curses color pairs on every platform,
	// including Windows (PDCurses). ignoreTokenStyling() is kept; it's shared by
	// both the (now-removed) ANSI path and curses_wwrap_withTokens().


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
	inline void wrap(	std::string const &input,
				int width,
				std::ostream &os,
				size_t indent,
				bool newLines	) 
	{
		std::istringstream in(input);
		os << std::string(indent, ' ');
		// size_t current = indent; // (unused)
		std::string line;
		std::string word;
		std::string output = "";
		int lineCutOffIndex = 0;
		int numberOfLines = stevensStringLib::countLines(input);
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
	 * 	std::vector<PrintTokens> tokens  - The tokenized parts of the string str that we already have identified
	 * 
	 * Returns:
	 * 	std::vector<PrintToken> - Vector containing print token objects for every part of the print string, not just the specficially
	 * 										styled tokens we already have (which are included in the output vector)
	*/
	inline std::vector<PrintToken> tokenizeBetweenTokens(	std::string & str,
																std::vector<PrintToken> tokens	)
	{
		//Start at the beginning of str
		int startingIndex = 0;
		std::string contentToTokenize = "";
		//Initialize the vector to return
		std::vector<PrintToken> tokensToReturn = {};

		//For each token, tokenize everything up to where it occurs in str
		for(size_t i = 0; i < tokens.size(); i++)
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
			std::string rawBetweenToken = PrintTokenHelper::tokenize(contentToTokenize, 0, contentToTokenize.length());
			PrintToken betweenToken = PrintTokenHelper::parseRawToken(rawBetweenToken);
			betweenToken.existsAtIndex = startingIndex;
			betweenToken.styled = false; //This token is not styled, it is just the content between the tokens

			//Change the str parameter to have the tokenized content
			str.replace(startingIndex, betweenToken.content.length(), betweenToken.rawToken);

			// Increment the existsAtIndex of all tokens that come after the current token in the vector
			size_t lengthDifference = betweenToken.rawToken.length() - contentToTokenize.length();
			for (size_t j = i; j < tokens.size(); j++) {
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
			std::string rawBetweenToken = PrintTokenHelper::tokenize(contentToTokenize, 0, contentToTokenize.length());
			PrintToken betweenToken = PrintTokenHelper::parseRawToken(rawBetweenToken);
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
	 * into PrintToken objects and store them in a vector that will
	 * be returned.
	 * 
	 * Parameter:
	 * 	std::string input - A string that possibly contains tokens to process.
	 * 
	 * Returns:
	 * 	std::vector<PrintToken> - A vector containing tokens found in the input string.
	*/
	
	// Forward declarations
	inline std::string processNestedContent(const std::string& content, const std::string& parentStyle);
	inline std::string inheritParentStyle(const std::string& nestedStyle, const std::string& parentStyle);
	
	// OPTIMIZED: Fast inline pattern matching without substr()
	inline bool matchesTokenEnd(const std::string& input, size_t pos) {
		return pos + 2 < input.length() && 
			   input[pos] == '}' && 
			   input[pos + 1] == '$' && 
			   input[pos + 2] == '[';
	}
	
	// OPTIMIZED: Single-pass brace matching with optimized pattern detection
	inline size_t findMatchingTokenEnd(const std::string& input, size_t openBrace) {
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
	inline TokenInfo extractTokenInfo(const std::string& input, size_t openBrace) {
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
	inline std::string preprocessNestedTokens(std::string input) {
		// Quick early exit if no tokens present
		if (input.find("}$[") == std::string::npos) {
			return input;
		}
		
		// Pre-reserve space to reduce reallocations during replacement
		std::string result;
		result.reserve(input.length() * 2); // Heuristic: nested tokens typically expand
		
		size_t pos = 0;

		while (pos < input.length()) {
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
	inline void processNestedContent(const std::string& content, const std::string& parentStyle, std::string& result) {
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
	inline std::string processNestedContent(const std::string& content, const std::string& parentStyle) {
		std::string result;
		result.reserve(content.length() * 2); // Heuristic for nested expansion
		processNestedContent(content, parentStyle, result);
		return result;
	}
	
	// OPTIMIZED: Fast style parsing without stringstream overhead
	inline void parseStyleString(const std::string& style, std::unordered_map<std::string, std::string>& styleMap) {
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
	inline std::string inheritParentStyle(const std::string& nestedStyle, const std::string& parentStyle) {
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

	inline std::vector<PrintToken> tokenizePrintString(	std::string & input	)
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
		std::vector<PrintToken> tokenVector = {};
		std::vector<PrintToken> nestedTokenVector = {};
		int currentBraceDepth = 0; // Track brace depth for proper nested token parsing
		//std::stack<int> parentTokenPositions = {};

		//Iterate over each character in the string
		for(size_t i = 0; i < input.length(); i++)
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
						PrintToken token = PrintTokenHelper::parseRawToken(rawToken);
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
								for( size_t n = 0; n < nestedTokenVector.size(); n++)
								{
									/*** Create token before nested token ***/
									//Get the nested token location
									size_t nestedTokenStart = nestedTokenVector[n].existsAtIndex;

									//Tokenize the string between the the tokenizeStartPosition and the start of the nested token
									std::string beforeTokenContent = input.substr(beforeNestedTokenStart+1, (nestedTokenStart - beforeNestedTokenStart - 1));
									beforeTokenContentLength = beforeTokenContent.length();
									PrintToken tokenBefore;

									if(!beforeTokenContent.empty())
									{
										std::string rawTokenBefore = PrintTokenHelper::tokenize( beforeTokenContent, 0, beforeTokenContent.length(), token.getStyleString() );
										//Create a new token object for the tokenized content before the nested token
										tokenBefore = PrintTokenHelper::parseRawToken(rawTokenBefore);
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
									std::string rawTokenAfter = PrintTokenHelper::tokenize( afterTokenContent, 0, std::string::npos, token.getStyleString() );
									//Create a new token object for the tokenized content after the nested token
									PrintToken tokenAfter = PrintTokenHelper::parseRawToken(rawTokenAfter);
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
	 * Given a vector of PrintTokens, log them and all of their member variables to a file.
	 * Useful for debugging and understanding what is and isn't being tokenized.
	 * 
	 * Parameters:
	*/
	inline void logTokens(	std::vector<PrintToken> tokens)
	{
		//Create a file.
		std::ofstream tokenLog ("s_Terminal_tokenLog.txt", std::ios::out | std::ios::app);
		if (tokenLog.is_open())
		{
			//Opening brace
			tokenLog << "{";
			//Now we write to the token log for each token we have and all of its variables
			for(size_t i = 0; i < tokens.size(); i++)
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
			std::cout << "stevensTerminal::PrintHelper: logTokens: Unable to open or write to s_Terminal_tokenLog.txt!" << std::endl;
		}
	}



	/**
	 * Given a string, str, and std::vector<PrintToken>, find all the instances of the
	 * raw tokens in str and erase them.
	 * 
	 * Parameters:
	 * 	std::string str - The string we with to erase raw tokens from.
	 *	std::vector<PrintToken> - A vector of tokens that the string str has included within in it in raw string form.
		* 
		* Returns:
		* 	std::string - The input string str, but with all of the raw tokens removed.
		*/
	inline std::string ignoreTokenStyling(	std::string str,
									std::vector<PrintToken> tokensToStyle	)
	{
		    // Track the cumulative length difference caused by replacements
			int cumulativeLengthDifference = 0;

			// For each token in the tokensToStyle vector
			for (size_t i = 0; i < tokensToStyle.size(); i++) {
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




	/**
	 * Given an PrintToken, retrieve all of the data for attributes we need to turn on.
	 * 
	 * Parameters:
	 * 	PrintToken token - The token we are enabling the style attributes for.
	 * 
	 * Returns:
	 * 	void
	*/
	inline std::unordered_map<std::string,chtype>	curses_styleToken(	PrintToken token	)
	{
		std::unordered_map<std::string,chtype> curses_attribute_data = {};

		/*** COLOR ***/
		// An unspecified (empty) or explicitly-"default" attribute renders as the terminal default.
		// Defaults are applied here, at print time, rather than being stamped onto tokens at parse
		// time (which would prevent inheritance — see PrintToken::inheritStyle).
		if(token.textColor == "default" || token.textColor.empty())
		{
			token.textColor = Colors::curses_default_textColor;
		}
		if(token.bgColor == "default" || token.bgColor.empty())
		{
			token.bgColor = Colors::curses_default_backgroundColor;
		}
		std::string color_key = token.textColor + "_on_" + token.bgColor;

		// Check if color pair exists, throw error if not
		if(!Colors::curses_colorPairs.contains(color_key))
		{
			std::string error_msg = "ERROR: stevensTerminal - Color pair '" + color_key +
			                       "' does not exist! Available COLORS=" +
			                       std::to_string(COLORS) + ", COLOR_PAIRS=" +
			                       std::to_string(COLOR_PAIRS) + ". " +
			                       "This combination may require more color pair support.";
			std::cerr << error_msg << std::endl;
			// Fallback to default color pair
			curses_attribute_data["colorPair"] = COLOR_PAIR(0);
		}
		else
		{
			curses_attribute_data["colorPair"] = COLOR_PAIR(Colors::curses_colorPairs[color_key]);
		}

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

		/*** UNDERLINE ***/
		if( token.underline )
		{
			curses_attribute_data["underline"] = A_UNDERLINE;
		}
		else
		{
			curses_attribute_data["underline"] = 0;
		}

		/*** REVERSE ***/
		if( token.reverse )
		{
			curses_attribute_data["reverse"] = A_REVERSE;
		}
		else
		{
			curses_attribute_data["reverse"] = 0;
		}

		/*** DIM ***/
		if( token.dim )
		{
			curses_attribute_data["dim"] = A_DIM;
		}
		else
		{
			curses_attribute_data["dim"] = 0;
		}

		/*** ITALIC ***/
		if( token.italic )
		{
			curses_attribute_data["italic"] = A_ITALIC;
		}
		else
		{
			curses_attribute_data["italic"] = 0;
		}

		return curses_attribute_data;
	}


	/**
	 * Given a map describing the styles to be applied to non-tokenized text, create a map of attribute
	 * names and their associated values which should be turned on in curses to style the text.
	 * 
	 * Parameter:
	 * 	std::unordered_map<std::string,std::string> style -	An unordered_map object containing keys of specific style options for text
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
	inline std::unordered_map<std::string,chtype> curses_styleAttributes(	std::unordered_map<std::string,std::string> style	)
	{
		std::unordered_map<std::string,chtype> curses_attribute_data = {};

		/*** COLOR ***/
		if(style["textColor"] == "default")
		{
			style["textColor"] = Colors::curses_default_textColor;
		}
		if(style["bgColor"] == "default")
		{
			style["bgColor"] = Colors::curses_default_backgroundColor;
		}
		std::string color_key = style["textColor"] + "_on_" + style["bgColor"];

		curses_attribute_data["colorPair"] = COLOR_PAIR(Colors::curses_colorPairs[color_key]);

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

		/*** UNDERLINE ***/
		if(style["underline"] == "true")
		{
			curses_attribute_data["underline"] = A_UNDERLINE;
		}
		else
		{
			curses_attribute_data["underline"] = 0;
		}

		/*** REVERSE ***/
		if(style["reverse"] == "true")
		{
			curses_attribute_data["reverse"] = A_REVERSE;
		}
		else
		{
			curses_attribute_data["reverse"] = 0;
		}

		/*** DIM ***/
		if(style["dim"] == "true")
		{
			curses_attribute_data["dim"] = A_DIM;
		}
		else
		{
			curses_attribute_data["dim"] = 0;
		}

		/*** ITALIC ***/
		if(style["italic"] == "true")
		{
			curses_attribute_data["italic"] = A_ITALIC;
		}
		else
		{
			curses_attribute_data["italic"] = 0;
		}

		return curses_attribute_data;
	}


	/**
	 * For a curses window, turns on the attributes that are defined in the passed-in map.
	 * 
	 * Parameters:
	 * 	WINDOW * win - The window which we are turning on curses attributes for.
	 * 	std::unordered_map<std::string, int> curses_attribute_data - A map containing all of the attribute names (keys) and the attribute values (values)
	 * 															to turn on for the curses window.
	 * 
	 * Returns:
	 * 	void;
	*/
	inline void curses_wAttrOn(	WINDOW * win,
							std::unordered_map<std::string, chtype> curses_attribute_data	)
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
	 * 	std::unordered_map<std::string, int> curses_attribute_data - A map containing all of the attribute names (keys) and the attribute values (values)
	 * 															to turn off for the curses window.
	 * 
	 * 	Returns:
	 * 		void;
	*/
	inline void curses_wAttrOff(	WINDOW * win,
							std::unordered_map<std::string, chtype> curses_attribute_data	)
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
	 * 	std::unordered_map<std::string, int> curses_attribute_data - A map containing all of the attribute names (keys) and the attribute values (values)
	 * 															to turn on for curses stdscr.
	 * 
	 * Returns:
	 * 	void;
	*/
	inline void curses_attrOn(		std::unordered_map<std::string, chtype> curses_attribute_data	)
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
	 * 	std::vector<PrintToken> tokens - An ordered collection of PrintToken objects which will be printed to the curses window.
	 * 	std::unordered_map<std::string,std::string> style - Styling options for all unstyled tokens.
	 * 	unordereD_map<std::string,std::string> format - Advanced formatting options for printing.
	 * 													Valid key-value pairs are:
	 * 													{"avoid borders","true"/"false"}
	 * 														-Avoid reprinting over 
	 * 
	 * Returns:
	 * 	void
	*/
	inline void curses_wprint_withTokens(	WINDOW * win,
									int yMove,
									int xMove,
									std::vector<PrintToken> tokens,
									std::unordered_map<std::string,std::string> style,
									std::unordered_map<std::string,std::string> format,
									bool textStyling	)
	{
		//Holds data for which attributes to use before printing text
		std::unordered_map<std::string, chtype> curses_attribute_data = {};
		//Process the style map's attributes
		std::unordered_map<std::string, chtype> style_attribute_data = curses_styleAttributes(style);

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

			//Print!
			mvwprintw(win, yMove, xMove, "%s", tokens[i].content.c_str());
			// Advance position so the next token continues from where this one ended
			getyx(win, yMove, xMove);

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
	 * 	std::unordered_map<std::string,std::string> style - The styling options that we will apply to the text that we print to the curses window.
	 * 	std::unordered_map<std::string,std::string> format - Any advanced formatting options we wish to apply while printing to the curses window.
	 * 													Valid key-value pairs are:
	 * 													{
	 * 													}
	 * 	bool textStyling - True if we are styling our text. False if we're not styling our text.
	 * 
	 * Returns:
	 * 	void
	*/
	inline void curses_wprint( WINDOW * win,
						int yMove,
						int xMove,
						std::string input,
						std::unordered_map<std::string,std::string> style,
						std::unordered_map<std::string,std::string> format,
						bool textStyling	)
	{
		//Tokenize what we're going to be printing, just to see if a user included any inline style tokens
		std::vector<PrintToken> tokens = tokenizePrintString(input);

		//Check to see if we're doing text styling
		if(textStyling)
		{
			//For any space in between tokens, we tokenize that with default styling
			tokens = tokenizeBetweenTokens(input, tokens);

			//Check to see if the style map is complete before we print
			style = PrintTokenStyling::setMissingStylesToDefault(style);

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
	 * Pure helper: splits a single text line (no embedded newlines) into the visual
	 * segments it would occupy when printed in a curses window.
	 *
	 * Parameters:
	 *   line             - text to split (no newlines)
	 *   xMove            - starting x column
	 *   width            - window width from getmaxyx()
	 *   borderAdjustment - pass 1 when the window has a right border to avoid
	 *
	 * Returns a vector of strings; each element is the text for one visual row.
	 * Every segment fits entirely within columns [xMove, width - borderAdjustment).
	 * When a space is available it is used as the break point; otherwise the line
	 * is force-broken at the available width.
	 *
	 * This function has no ncurses dependencies and can be called from unit tests.
	*/
	inline std::vector<std::string> computeWrapSegments(
							std::string line,
							int xMove,
							int width,
							int borderAdjustment)
	{
		std::vector<std::string> segments;
		int textWidth = (width - borderAdjustment) - xMove;

		if (textWidth <= 0) return segments;

		// Decode once and work in codepoints (not bytes) for the rest of this function, so multi-byte
		// characters (Cyrillic, CJK, box-drawing glyphs) are counted/cut correctly - see
		// stevensStringLib::utf8to32() doc comment.
		std::u32string u32line = stevensStringLib::utf8to32(line);

		while (true) {
			if ((int)u32line.length() <= textWidth) {
				segments.push_back(stevensStringLib::utf32to8(u32line));
				break;
			}
			// Find the last space within textWidth characters
			size_t cutPos = u32line.rfind(U' ', (size_t)(textWidth - 1));
			if (cutPos == std::u32string::npos) {
				// No space: force-break
				segments.push_back(stevensStringLib::utf32to8(u32line.substr(0, textWidth)));
				u32line = u32line.substr(textWidth);
			} else {
				segments.push_back(stevensStringLib::utf32to8(u32line.substr(0, cutPos)));
				u32line = u32line.substr(cutPos + 1);
			}
		}

		return segments;
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
	 * 	std::unordered_map<std::string,std::string> style - The style to apply to the text that we print
	 *
	 * Returns:
	*/
	inline void curses_wwrap(	WINDOW * win,
						int yMove,
						int xMove,
						std::string printString,
						size_t indent,
						std::unordered_map<std::string,std::string> style )
	{
		std::istringstream in(printString);
		std::string line;
		std::string output;
		int lineCutOffIndex = 0;
		int numberOfLines = stevensStringLib::countLines(printString);
		int currLineNum = 0;
		int width;
		int height;
		getmaxyx(win, height, width); //Curses function to get the width of the window we are printing to

		//Turn on any styles we might be using
		std::unordered_map<std::string,chtype> curses_style_data;
		if(!style.empty())
		{
			//Turn on the style attributes
			curses_style_data = curses_styleAttributes(style);
			curses_wAttrOn(win, curses_style_data);
		}

		while(getline(in,line))
		{
			// Decode once and work in codepoints (not bytes) for the rest of this line, so
			// multi-byte characters (Cyrillic, CJK, box-drawing glyphs) are counted/cut correctly
			// instead of being torn apart by a byte-offset substr/rfind - see
			// stevensStringLib::utf8to32() doc comment. Only re-encoded (via utf32to8()) right
			// before each mvwprintw() call.
			std::u32string u32line = stevensStringLib::utf8to32(line);
			lineCutOffIndex = 0;
			// Characters available for text on this line, accounting for where we start (xMove)
			int textWidth = width - xMove - (int)indent;

			while(true)
			{
				//Check to see if we need to wrap this line around
				if(textWidth > 0 && (int)u32line.length() > textWidth)
				{
					//Find the last space in the string that fits on the line
					lineCutOffIndex = u32line.rfind(U' ', (size_t)(textWidth - 1));
					//If we can't find a space in the string and the string is too long, we just cut the line and wrap to the next line
					if(lineCutOffIndex == std::u32string::npos)
					{
						//Add as much of the line as we can to the output and then add a newline
						output = std::string(indent, ' ') + stevensStringLib::utf32to8(u32line.substr(0, textWidth));
						mvwprintw(win,yMove,xMove,"%s", output.c_str());
						yMove++;
						//Continue looping until the rest of the line is added to the output
						u32line = u32line.substr(textWidth);
					}
					//If we find a space...
					else
					{
						//Add the part of the string before the cut off point to the output
						output = std::string(indent,' ') + stevensStringLib::utf32to8(u32line.substr(0, lineCutOffIndex));
						mvwprintw(win,yMove,xMove,"%s", output.c_str());
						yMove++;
						//Set the line equal to the cut off portion of the string and continue looping until the rest of the line is added to the output
						u32line = u32line.substr(lineCutOffIndex+1);
					}
				}
				else
				{
					output = stevensStringLib::utf32to8(u32line);
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
	inline void curses_wwrap_withTokens(	WINDOW * win,
									int yMove,
									int xMove,
									std::vector<PrintToken> tokens,
									std::unordered_map<std::string,std::string> style,
									std::unordered_map<std::string,std::string> format,
									bool textStyling	)
	{
		//Holds data for which attributes to use before printing text
		std::unordered_map<std::string, chtype> curses_attribute_data = {};
		//Process the style map's attributes and store them in a map that can be understood by curses
		std::unordered_map<std::string, chtype> style_attribute_data = curses_styleAttributes(style);
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

		// Text alignment within the available line width.
		// "left" (default) prints from xMove; "center" offsets each segment so it is
		// horizontally centred; "right" is reserved for future use.
		std::string textAlign = format.contains("textAlign") ? format.at("textAlign") : "left";

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

			PrintHelper::curses_wAttrOn(win, curses_attribute_data);

			//Print each line of the token
			while(getline(in,line))
			{
				// Decode once and work in codepoints (not bytes) for the rest of this line, so
				// multi-byte characters (Cyrillic, CJK, box-drawing glyphs) are counted/cut correctly
				// instead of being torn apart by a byte-offset substr/rfind - see
				// stevensStringLib::utf8to32() doc comment. Only re-encoded (via utf32to8()) right
				// before each mvwprintw() call.
				std::u32string u32line = stevensStringLib::utf8to32(line);
				lineCutOffIndex = 0;
				while(!u32line.empty())
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
					if(((xMove-borderAdjustment) + (int)u32line.length()) >= (width - borderAdjustment))
					{
						// printw("%s", line.c_str());
						if(format.contains("debug"))
						{
							std::cout << "need to wrap" << std::endl;
							getch();
						}
						size_t maxLength = ((width-borderAdjustment)-xMove);
						if(maxLength > u32line.length()) maxLength = u32line.length();
						std::u32string bitOfLineThatCanFit = u32line.substr(0, maxLength);
						//Find the last space in the string that fits on the line
						lineCutOffIndex = bitOfLineThatCanFit.rfind(U' ', (indent + width));
						if (lineCutOffIndex == std::u32string::npos)
						{
							// No space within available width: force-break at maxLength.
							// If maxLength is 0 (cursor already at right boundary) we just
							// advance to the next line without printing anything.
							if (maxLength > 0) {
								std::u32string u32output = u32line.substr(0, maxLength);
								output = stevensStringLib::utf32to8(u32output);
								{
									int availableWidth = (int)maxLength;
									int printX = xMove + (textAlign == "center" ? std::max(0, (availableWidth - (int)u32output.length()) / 2) : 0);
									mvwprintw(win, yMove, printX, "%s", output.c_str());
								}
								u32line = (maxLength < u32line.length()) ? u32line.substr(maxLength) : U"";
							}
							yMove++;
							charsPrintedToLine = 0;
							if (retainXMoveOnNewline) {
								xMove = xMoveOrigin;
							} else if (avoidBorders) {
								xMove = 1;
							} else {
								xMove = 0;
							}
							wmove(win, yMove, xMove);
						}
						else
						{
							// Space found: break at the last space within available width
							std::u32string u32output = u32line.substr(0, lineCutOffIndex);
							output = stevensStringLib::utf32to8(u32output);
							{
								int availableWidth = (int)((width - borderAdjustment) - xMove);
								int printX = xMove + (textAlign == "center" ? std::max(0, (availableWidth - (int)u32output.length()) / 2) : 0);
								mvwprintw(win, yMove, printX, "%s", output.c_str());
							}
							yMove++;
							charsPrintedToLine = 0;
							if (lineCutOffIndex + 1 < u32line.length()) {
								u32line = u32line.substr(lineCutOffIndex + 1);
							} else {
								u32line.clear();
							}
							if (retainXMoveOnNewline) {
								xMove = xMoveOrigin;
							}
							else if (avoidBorders)
							{
								xMove = 1;
							}
							else {
								xMove = 0;
							}
							wmove(win, yMove, xMove);
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
						{
							int availableWidth = (int)((width - borderAdjustment) - xMove);
							int printX = xMove + (textAlign == "center" ? std::max(0, (availableWidth - (int)output.length()) / 2) : 0);
							mvwprintw(win, yMove, printX, "%s", output.c_str());
						}
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

} // namespace PrintHelper
} // namespace stevensTerminal