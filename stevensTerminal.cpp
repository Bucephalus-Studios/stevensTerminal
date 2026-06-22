/**
 * stevensTerminal.cpp
 * Implementation file for stevensTerminal library
 * Migrated from header-only to compiled library for faster build times
 */

#include "stevensTerminal.hpp"

#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <limits>
#include <limits>

#include "classes/PrintToken.hpp"
#include "classes/PrintTokenHelper.hpp"
#include "classes/WindowManager.hpp"
#include <stevensMapLib.hpp>
#include "subnamespaces/Colors.hpp"
#include "subnamespaces/Input.hpp"
#include "subnamespaces/ParticleFX/ParticleWindowRegistry.hpp"
#include "subnamespaces/Styling.hpp"

namespace stevensTerminal
{

// ==================== GLOBAL VARIABLE DEFINITIONS ====================

std::map<std::string, DisplayMode> displayModes = {
    {"very small", DisplayMode({-1,-1},{79,23},18)},
    {"small", DisplayMode({80,24},{119,39},18)},
    {"regular", DisplayMode({120,40},{-1,-1},24)}
};

std::string currentDisplayMode = "regular";
bool textStyling = true;
bool curses_colorCompatible = false;

// ==================== CORE.HPP IMPLEMENTATIONS ====================

WindowManager& windowManager() {
    static WindowManager instance;
    return instance;
}

#if defined(_WIN32)
std::pair<int, int> get_screen_size() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return std::pair<int, int>(csbi.srWindow.Right - csbi.srWindow.Left + 1,
                               csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
}
#elif defined(__linux__)
std::pair<int, int> get_screen_size() {
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    return std::pair<int, int>(size.ws_col, size.ws_row);
}
#endif

void hideCursor() { curs_set(0); }

void showCursor() { curs_set(1); }

#ifdef curses
void curses_prepare_color()
{
    if(has_colors())
    {
        start_color();
        // Set up Curses color codes
        Colors::curses_setup_colorCodes();
        // Set up Curses colorpairs
        Colors::curses_setup_colorPairs();
        curses_colorCompatible = true;
        textStyling = true;
    }
    else
    {
        curses_colorCompatible = false;
        textStyling = false;
    }
}
#endif

void initialize(bool initWindowManager,
                const std::vector<std::string>& windowNames) {
    #ifdef curses
    // Initialize ncurses
    initscr();

    // Set up color support
    curses_prepare_color();

    // Configure ncurses behavior
    cbreak();              // Disable line buffering
    noecho();              // Don't echo input
    keypad(stdscr, TRUE);  // Enable function keys
    curs_set(0);           // Hide cursor

    // Initialize window manager if requested
    if (initWindowManager)
    {
        if (!windowNames.empty())
        {
            windowManager().initialize(windowNames);
        }
        else
        {
            windowManager().initialize();  // Use default windows
        }
    }

    // Set display mode based on screen size
    //setDisplayMode(get_screen_size());
    #endif
}

void shutdown() {
    #ifdef curses
    windowManager().shutdown();  // This handles both window cleanup and endwin()
    #endif
}

bool displayMode_GTEminSize(std::pair<int,int> minSize, std::pair<int,int> screenSize) {
    if(minSize.first == -1 && minSize.second == -1) {
        return true;
    }
    if(minSize.first == -1) {
        return screenSize.second >= minSize.second;
    }
    if(minSize.second == -1) {
        return screenSize.first >= minSize.first;
    }
    return (screenSize.first >= minSize.first && screenSize.second >= minSize.second);
}

bool displayMode_LTEmaxSize(std::pair<int,int> maxSize, std::pair<int,int> screenSize) {
    if(maxSize.first == -1 && maxSize.second == -1) {
        return true;
    }
    if(maxSize.first == -1) {
        return screenSize.second <= maxSize.second;
    }
    if(maxSize.second == -1) {
        return screenSize.first <= maxSize.first;
    }
    return (screenSize.first <= maxSize.first && screenSize.second <= maxSize.second);
}

void setDisplayMode(std::pair<int,int> screenSize) {
    for(auto const& mode : displayModes) {
        if(displayMode_GTEminSize(mode.second.minSize, screenSize) &&
           displayMode_LTEmaxSize(mode.second.maxSize, screenSize)) {
            currentDisplayMode = mode.first;
            return;
        }
    }
    // If no mode matches, default to "regular"
    currentDisplayMode = "regular";
}

// ==================== COLORS.HPP IMPLEMENTATIONS ====================

namespace Colors {

std::unordered_map<std::string, int> curses_colorPairs;
std::unordered_map<std::string, int> curses_colors;

void curses_setup_colorCodes()
{
	// First 2 colors
	if(COLORS >= 2)
	{
		curses_colors["black"] = COLOR_BLACK;
		curses_colors["white"] = COLOR_WHITE;
	}
	else
	{
		std::cout << "Error from stevensTerminal::Colors - less than 2 colors available for use detected! Closing program." << std::endl;
		exit(EXIT_FAILURE);
	}
	// 8 colors
	if(COLORS >= 8)
	{
		curses_colors["red"]     = COLOR_RED;
		curses_colors["green"]   = COLOR_GREEN;
		curses_colors["yellow"]  = COLOR_YELLOW;
		curses_colors["blue"]    = COLOR_BLUE;
		curses_colors["magenta"] = COLOR_MAGENTA;
		curses_colors["cyan"]    = COLOR_CYAN;
	}
	// 16 colors
	if(COLORS >= 16)
	{
		curses_colors["bright-black"]   = 8;
		curses_colors["bright-red"]     = 9;
		curses_colors["bright-green"]   = 10;
		curses_colors["bright-yellow"]  = 11;
		curses_colors["bright-blue"]    = 12;
		curses_colors["bright-magenta"] = 13;
		curses_colors["bright-cyan"]    = 14;
		curses_colors["bright-white"]   = 15;
	}
}

void curses_setup_colorPairs()
{
	// Error check
	if ((COLORS < 2) || (COLOR_PAIRS < 1)) {
		std::cout << "Error from stevensTerminal::Colors - insufficient color support! Closing program." << std::endl;
		exit(EXIT_FAILURE);
	}

	// Get all available colors from curses_colors map
	std::vector<int> allColors = stevensMapLib::getValueVector(curses_colors);
	std::vector<std::string> allColorNames = stevensMapLib::getKeyVector(curses_colors);

	int pairNum = 0;

	// Generate all foreground x background combinations
	for (size_t bgIdx = 0; bgIdx < allColors.size(); ++bgIdx) {
		int bgColor = allColors[bgIdx];
		const std::string& bgName = allColorNames[bgIdx];

		// Skip bright backgrounds if we don't have 16 colors
		if (bgColor >= 8 && COLORS < 16) continue;

		for (size_t fgIdx = 0; fgIdx < allColors.size(); ++fgIdx) {
			int fgColor = allColors[fgIdx];
			const std::string& fgName = allColorNames[fgIdx];

			// Skip bright foregrounds if we don't have 16 colors
			if (fgColor >= 8 && COLORS < 16) continue;

			// Skip if we've exceeded available color pairs
			if (pairNum >= COLOR_PAIRS) break;

			// Create the color pair
			std::string pairName = fgName + "_on_" + bgName;
			init_pair(pairNum, fgColor, bgColor);
			curses_colorPairs[pairName] = pairNum;

			// Special case: "default" for white on black
			if (pairNum == 0) {
				curses_colorPairs["default"] = pairNum;
			}

			pairNum++;
		}

		// Stop if we've exceeded available color pairs
		if (pairNum >= COLOR_PAIRS) break;
	}
}

std::string colorToString(int colorNum)
{
	std::string result = stevensMapLib::getKeyByValue(curses_colors, colorNum);
	// If not found (empty string), return "white" as fallback
	return result.empty() ? "white" : result;
}

int lookupColorPair(int fgColor, int bgColor) {
	std::string fgName = colorToString(fgColor);
	std::string bgName = colorToString(bgColor);
	std::string pairName = fgName + "_on_" + bgName;

	if (curses_colorPairs.contains(pairName)) {
		return curses_colorPairs[pairName];
	}

	// Fallback to pair 0 (default white on black)
	return 0;
}

int lookupColorPairByName(const std::string& fgName, const std::string& bgName) {
	std::string pairName = fgName + "_on_" + bgName;
	if (curses_colorPairs.contains(pairName))
		return curses_colorPairs.at(pairName);
	return 0;
}

short extractForegroundColor(int colorPairNum) {
	short fg, bg;
	pair_content(colorPairNum, &fg, &bg);
	return fg;
}

short extractBackgroundColor(int colorPairNum) {
	short fg, bg;
	pair_content(colorPairNum, &fg, &bg);
	return bg;
}

} // namespace Colors

// ==================== STYLING.HPP IMPLEMENTATIONS ====================

std::string addStyleToken(std::string str,
                          std::unordered_map<std::string,std::string> styleMap)
{
    // Read the contents of the style map into a std::string list of keys and values associated by '=' and delimited by ','
    std::string styleString = stevensStringLib::stringifyMap(styleMap, "=", ",");
    str = "{" + str + "}$[" + styleString + "]";
    return str;
}

std::string style(std::string str,
                  std::unordered_map<std::string,std::string> styleMap)
{
    return addStyleToken(std::move(str), std::move(styleMap));
}

void insertStyleToken(std::string & str,
                      const PrintToken & token)
{
    // First check to see if the index that the token exists at is valid
    if( token.existsAtIndex >= str.length() ) {
        // If not, just return without doing anything
        return;
    }
    // Otherwise, begin by placing the left content bound character at the token's existsAtIndex
    str.insert(token.existsAtIndex, 1, '{');
    // To find the placement of the right content bound character, step over the length of the content from the existsAtIndex
    size_t indexOfRightContentBound = token.existsAtIndex + token.content.length() + 1;
    // If this would cause us to exceed the length of the string, just place the indexOfRightContentBound at the end of the std::string
    if(indexOfRightContentBound >= str.length() ) {
        indexOfRightContentBound = str.length();
    }
    // Get the directives to place in the style std::string
    std::string styleString = token.getStyleString();
    // Insert the right bound and the style directives
    std::string endOfToken = "}$[" + styleString + "]";
    str.insert(indexOfRightContentBound, endOfToken);
}

std::string removeAllStyleTokenization(std::string str)
{
    // Find a token return its position in the std::string
    size_t tokenPosition = PrintTokenHelper::findToken(str);
    // If findToken return std::string::npos, then no token was found
    while( tokenPosition != std::string::npos ) {
        // Remove its braces and styling directives
        str = PrintTokenHelper::untokenize(str, tokenPosition);
        // Try to find the position of the next token
        tokenPosition = PrintTokenHelper::findToken(str);
    }
    return str;
}

std::string resizeStyledString(std::string str, const size_t desiredLength, const char fillChar)
{
    // First, get all of the style tokens that the std::string contains
    std::vector<PrintToken> tokens = PrintTokenHelper::getAllTokens(str);

    // If there are no style tokens, just return a call of the std::string.resize() function
    if(tokens.empty()) {
        str.resize(desiredLength, fillChar);
        return str;
    }

    // Otherwise, now that we have all of the style tokens, we take just their content and turn it into a std::string
    std::string contentString = removeAllStyleTokenization(str);
    // We resize this std::string
    contentString.resize(desiredLength, fillChar);
    std::string resizedStr = contentString;
    // We now place the style tokens back into this resized std::string
    for(int tokenIndex = 0; tokenIndex < tokens.size(); tokenIndex++) {
        // For each style token, check to see if the index which it starts relative to the content and all other tokens
        // is still in bounds of the string's length
        if(tokens.at(tokenIndex).existsAtIndex+1 < resizedStr.length()) {
            // If so, then we add the style token back at its original location
            insertStyleToken(resizedStr, tokens.at(tokenIndex));
        } else {
            // Otherwise, all other style tokens will no longer be in bounds either, as they come after the current one
            // in order, so we break and finish
            break;
        }
    }

    // Return the resized styled std::string
    return resizedStr;
}

void printHorizontalBorder(std::string borderPattern,
                           std::string color,
                           std::string bgColor,
                           std::vector<std::string> colorVector,
                           std::vector<std::string> bgColorVector)
{
    // Get the width of our current display setting
    int displayWidth = displayModes[currentDisplayMode].minSize.first;
    // If we have -1, meaning no set display boundary, just take the size of the terminal window
    if(displayWidth == -1) {
        displayWidth = get_screen_size().first-1;
    }

    print("\n");

    // Check to see if our horizontal border is going to be a single color/bgcolor or multiple colors/bgcolors
    if(colorVector.size() == 0 && bgColorVector.size() == 0) { // Single color/bgcolor
        // Gradually build up a std::string containing the full border that we will print once to the terminal
        std::string output = "";
        while(output.length() < displayWidth) {
            for(int patternIndex = 0; patternIndex < borderPattern.length(); patternIndex++) {
                if(output.length() < displayWidth) {
                    output += borderPattern[patternIndex];
                }
            }
        }
        print(output, {{"textColor",color}, {"bgColor",bgColor}});
    } else { // Multiple colors/bgcolors
        // Keep track of how many characters we have printed to the terminal for the border pattern
        int outputLength = 0;
        std::string colorToPrint = "default";
        std::string bgColorToPrint = "default";
        // Until the horizontal length of the screen is filled, we repeatedly print the pattern
        while(outputLength < displayWidth) {
            // Print each character of the border pattern out individually
            for(int patternIndex = 0; patternIndex < borderPattern.length(); patternIndex++) {
                if(outputLength < displayWidth) {
                    // Get the text color to print
                    if((colorVector.size()-1) >= patternIndex) {
                        colorToPrint = colorVector[patternIndex];
                    } else {
                        colorToPrint = "default";
                    }
                    // Get the text background color to print
                    if((bgColorVector.size()-1) >= patternIndex) {
                        bgColorToPrint = bgColorVector[patternIndex];
                    } else {
                        bgColorToPrint = "default";
                    }
                    print(std::to_string(borderPattern[patternIndex]), {{"textColor",colorToPrint}, {"bgColor",bgColorToPrint}});
                    outputLength++;
                }
            }
        }
    }

    print("\n");
}

// ==================== INPUT.HPP IMPLEMENTATIONS ====================

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string input() {
    std::string userInput; // the input from the player
    int whitespaceCount;
    bool hasValidInput = false; // bool declaring whether or not the correct input has been received
    print("> ");

    while (hasValidInput == false) {
        whitespaceCount = 0;
        std::getline(std::cin, userInput); // use getline to take in user input

        // Check to see if the std::string is just whitespace, if so, we prompt for input again
        for (int charIndex = 0; charIndex < userInput.length(); charIndex++) {
            if (userInput[charIndex] == ' ') {
                whitespaceCount++;
            } else {
                break;
            }
        }

        if (whitespaceCount == userInput.length()) {
            hasValidInput = false;
            continue;
        }

        if (userInput == "") { // we also check to see if the std::string is empty. If so, we prompt again for input.
            hasValidInput = false;
        } else { // if the std::string is not empty and the std::string is not entirely whitespace, we accept the input
            hasValidInput = true;
        }
    }
    return userInput;
}

int input(int responseRange) {
    if (responseRange < 0) {
        // Handle error case for negative response range
        void(0);
    }

    std::string userInput; // the input from the player
    int userInputAsNum; // casted form of the user input
    bool hasValidInput = false; // bool declaring whether or not the correct input has been received

    while (hasValidInput == false) {
        std::cout << "> ";
        std::cin >> userInput;

        if (stevensStringLib::isNumber(userInput)) {
            hasValidInput = true;
            userInputAsNum = stoi(userInput);

            if (responseRange >= 0) { // we request that our integer response be within a certain range
                if (!((userInputAsNum >= 0) && (userInputAsNum <= responseRange))) {
                    hasValidInput = false;
                }
            }
        } else {
            hasValidInput = false;
        }
        clearInput();
    }
    return userInputAsNum;
}

bool inputWithinResponseRange(std::string input, int responseRange) {
    if(stevensStringLib::isNumber(input)) {
        int inputAsInt = std::stoi(input);
        if((inputAsInt >= 1) && (inputAsInt <= responseRange)) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

std::string input(
    WINDOW * win,
    size_t maxLen,
    const std::unordered_map<std::string,std::string> & textStyle
)
{
    cbreak();
    noecho();
    keypad(win, true);
    showCursor();
    set_escdelay(25);

    std::unordered_map<std::string,int> styleAttrs = PrintHelper::curses_styleAttributes(textStyle);

    // Re-renders str into the window interior on each keystroke.
    // Clears all inner rows first (eliminates color bleed from previously-rendered
    // content), then prints str with textStyle, then positions the cursor at end.
    // Grows the window by one row via wresize if the cursor would hit the bottom border.
    auto renderInput = [&](const std::string & str)
    {
        int winHeight, winWidth;
        getmaxyx(win, winHeight, winWidth);
        const int innerWidth = winWidth - 2;

        int cursorRow = 1 + static_cast<int>(str.size()) / innerWidth;

        if(cursorRow >= winHeight - 1)
        {
            wresize(win, winHeight + 1, winWidth);
            getmaxyx(win, winHeight, winWidth);
        }

        wattrset(win, A_NORMAL);
        for(int r = 1; r < winHeight - 1; r++)
        {
            wmove(win, r, 1);
            for(int c = 0; c < innerWidth; c++)
                waddch(win, ' ');
        }

        PrintHelper::curses_wAttrOn(win, styleAttrs);
        for(size_t i = 0; i < str.size(); i++)
        {
            int r = 1 + static_cast<int>(i) / innerWidth;
            int c = 1 + static_cast<int>(i) % innerWidth;
            wmove(win, r, c);
            waddch(win, str[i]);
        }
        PrintHelper::curses_wAttrOff(win, styleAttrs);

        int cursorCol = 1 + static_cast<int>(str.size()) % innerWidth;
        wmove(win, cursorRow, cursorCol);
        wrefresh(win);
    };

    std::string str    = "";
    bool        escaped = false;

    renderInput(str);

    while(true)
    {
        int ch = wgetch(win);

        if(ch == KEY_ESCAPE)
        {
            escaped = true;
            break;
        }
        if(ch == '\n' || ch == '\r' || ch == KEY_ENTER)
            break;
        if(ch == KEY_BACKSPACE || ch == 127 || ch == 8)
        {
            if(!str.empty())
            {
                str.pop_back();
                renderInput(str);
            }
            continue;
        }
        if(ch >= 32 && ch < 127 && str.size() < maxLen)
        {
            str.push_back(static_cast<char>(ch));
            renderInput(str);
        }
    }

    keypad(win, false);
    hideCursor();

    if(escaped)
        return std::string(1, static_cast<char>(KEY_ESCAPE));
    return str;
}

// ==================== PARTICLEFX IMPLEMENTATIONS ====================

namespace ParticleFX {

// Static member definition for ParticleWindowRegistry
std::unordered_map<WINDOW*, ParticleWindowRegistry::WindowState> ParticleWindowRegistry::registry;

} // namespace ParticleFX

} // namespace stevensTerminal


namespace stevensTerminal
{

// ==================== FUNCTIONS MOVED FROM HEADER ====================
// (previously defined inline in stevensTerminal.hpp)

// ==================== calculateBarGraphLabel ====================
	BarGraphLabelFormat calculateBarGraphLabel(
		const std::string& labelText,
		float percentage,
		int availableWidth,
		bool showText,
		bool showPercentage)
	{
		BarGraphLabelFormat result;
		result.style = BarGraphLabelStyle::None;
		result.text = "";
		result.width = 0;
		result.indent = 0;

		if (!showText && !showPercentage) {
			return result;
		}

		std::string percentageStr = std::to_string(stevensMathLib::roundToNearest10th(percentage * 100));

		// Try text + percentage first (most informative)
		if (showText && showPercentage) {
			std::string combined = labelText + " " + percentageStr + "%";
			if (static_cast<int>(combined.length()) <= availableWidth) {
				result.text = combined;
				result.style = BarGraphLabelStyle::TextAndPercentage;
				result.width = combined.length();
				result.indent = (availableWidth - result.width) / 2;
				return result;
			}
		}

		// Try percentage only (compact)
		if (showPercentage) {
			std::string percentageLabel = percentageStr + "%";
			if (static_cast<int>(percentageLabel.length()) <= availableWidth) {
				result.text = percentageLabel;
				result.style = BarGraphLabelStyle::PercentageOnly;
				result.width = percentageLabel.length();
				result.indent = (availableWidth - result.width) / 2;
				return result;
			}
		}

		// Try text only
		if (showText && static_cast<int>(labelText.length()) <= availableWidth) {
			result.text = labelText;
			result.style = BarGraphLabelStyle::TextOnly;
			result.width = labelText.length();
			result.indent = (availableWidth - result.width) / 2;
			return result;
		}

		// Nothing fits - return empty
		return result;
	}

// ==================== horizontalStackedBarGraph ====================
	void horizontalStackedBarGraph(	std::vector<std::string> labels,
									std::vector< std::tuple<std::string,std::string> > colorCombos,
									std::vector<float> distribution,
									int width,
									bool textLabels,
									bool percentageLabels	)
	{
		// Input validation
		if(labels.size() != colorCombos.size() || labels.size() != distribution.size())
		{
			std::cerr << "Error: horizontalStackedBarGraph requires labels, colorCombos, and distribution vectors to have equal sizes" << std::endl;
			return;
		}
		if(width <= 0)
		{
			std::cerr << "Error: horizontalStackedBarGraph width must be positive" << std::endl;
			return;
		}
		if(labels.empty())
		{
			return; // Nothing to graph
		}

		if(!textStyling)
		{
			return; // Styling must be enabled for colored graphs
		}

		// Calculate total distribution
		float sumOfDistributions = 0;
		for(size_t labelIndex = 0; labelIndex < labels.size(); labelIndex++)
		{
			sumOfDistributions += distribution[labelIndex];
		}

		// Guard against division by zero
		if(sumOfDistributions == 0)
		{
			return; // Cannot create bar graph with zero total distribution
		}

		// Calculate percentages for each label
		std::vector<float> distributionPercentages;
		distributionPercentages.reserve(labels.size());
		for(size_t labelIndex = 0; labelIndex < labels.size(); labelIndex++)
		{
			distributionPercentages.push_back(distribution[labelIndex] / sumOfDistributions);
		}

		// Calculate bar width for each segment (in characters)
		std::vector<int> segmentWidths;
		segmentWidths.reserve(labels.size());
		for(size_t labelIndex = 0; labelIndex < labels.size(); labelIndex++)
		{
			int segmentWidth = static_cast<int>(floor(distributionPercentages[labelIndex] * width));
			segmentWidths.push_back(segmentWidth);
		}

		// Render each bar segment
		for(size_t labelIndex = 0; labelIndex < labels.size(); labelIndex++)
		{
			// Calculate optimal label for this segment
			BarGraphLabelFormat labelFormat = calculateBarGraphLabel(
				labels[labelIndex],
				distributionPercentages[labelIndex],
				segmentWidths[labelIndex],
				textLabels,
				percentageLabels
			);

			// Get colors for this segment
			const std::string& bgColor = std::get<0>(colorCombos[labelIndex]);
			const std::string& textColor = std::get<1>(colorCombos[labelIndex]);

			// Print the segment character by character
			int charsRemaining = segmentWidths[labelIndex];
			int indentRemaining = labelFormat.indent;
			size_t labelCharIndex = 0;

			while(charsRemaining > 0)
			{
				// // Print indent spaces first
				// if(indentRemaining > 0)
				// {
				// 	print(" ", {{"textColor", textColor}, {"bgColor", bgColor}});
				// 	indentRemaining--;
				// }
				// // Then print label characters
				// else if(labelCharIndex < labelFormat.text.length())
				// {
				// 	std::string charAsString(1, labelFormat.text[labelCharIndex]);
				// 	print(charAsString, {{"textColor", textColor}, {"bgColor", bgColor}, {"bold", "true"}});
				// 	labelCharIndex++;
				// }
				// // Fill remaining space
				// else
				// {
				// 	print(" ", {{"textColor", textColor}, {"bgColor", bgColor}});
				// }

				charsRemaining--;
			}
		}

		std::cout << "\n";
	}

// ==================== table ====================
	void table(	std::vector< std::tuple<std::string,std::string,std::string> > xLabels,
				std::vector< std::tuple<std::string,std::string,std::string> > yLabels, std::vector< std::vector<float> > tableContent, std::string tableType)
	{
		//Indent the X labels to account for the longest Y label
		int longestYLabelLength = 0;
		for(size_t yLabelIndex = 0; yLabelIndex < yLabels.size(); yLabelIndex++)
		{
			size_t currentYLabelLength = get<0>(yLabels[yLabelIndex]).length();
			if(currentYLabelLength > longestYLabelLength)
			{
				longestYLabelLength = currentYLabelLength;
			}
		}
		std::string leftIndent(longestYLabelLength + 1, ' ');
		print(leftIndent);

		//Print X labels (column headers)
		for(size_t xLabelIndex = 0; xLabelIndex < xLabels.size(); xLabelIndex++)
		{
			const std::string& labelText = get<0>(xLabels[xLabelIndex]);
			const std::string& textColor = get<1>(xLabels[xLabelIndex]);
			print(labelText, {{"textColor", textColor}});
			print("\t");
		}
		print("\n");

		// Print table rows with Y labels and cell content
		for(size_t rowIndex = 0; rowIndex < tableContent.size(); rowIndex++)
		{
			// Calculate indent for current Y label to align with longest Y label
			const std::string& currentYLabel = get<0>(yLabels[rowIndex]);
			std::string yLabelIndent(longestYLabelLength - currentYLabel.length() + 1, ' ');

			// Print each cell in the row
			for(size_t columnIndex = 0; columnIndex < tableContent[0].size(); columnIndex++)
			{
				// Get cell value and prepare entry string
				float cellValue = tableContent[rowIndex][columnIndex];
				std::string cellEntry;

				// Format cell based on table type
				if (tableType == "religionData")
				{
					if (rowIndex == 0) // Followers row: use integers
					{
						cellEntry = std::to_string(static_cast<int>(cellValue));
					}
					else if (rowIndex == 1) // Devotion row: round to tenths
					{
						cellEntry = std::to_string(stevensMathLib::roundToNearest10th(cellValue));
					}
					else
					{
						cellEntry = std::to_string(cellValue);
					}
				}
				else if(tableType == "factionData")
				{
					if(rowIndex == 0) // Political power row: round to tenths
					{
						cellEntry = std::to_string(stevensMathLib::roundToNearest10th(cellValue));
					}
					else
					{
						cellEntry = std::to_string(cellValue);
					}
				}
				else
				{
					cellEntry = std::to_string(cellValue);
				}

				print(cellEntry);

				// Add tabs for column alignment based on entry length
				const int TAB_WIDTH_THRESHOLD = 6;
				if(cellEntry.length() < TAB_WIDTH_THRESHOLD)
				{
					print("\t\t");
				}
				else
				{
					print("\t");
				}
			}
			print("\n");
		}
	}

// ==================== localizedWrap ====================
	std::string localizedWrap(	std::string input,
								int availableSpace,
								bool menuLabel)
	{
		//cout << "availableSpace:" << availableSpace;
		for(int i = 0; i < input.length(); i++)
		{
			if(availableSpace > 1)
			{
				if(menuLabel)
				{
					std::string s = std::string() + input[i];
					print(s, {{"textColor","default"}, {"bgColor", "default"}, {"bold","true"}});
				}
				else
				{
					std::cout << input[i];
				}
				availableSpace--;
			}
			else //If we have no more space, we print a hyphen and return the remainder of the std::string
			{
				std::cout << "-";
				return input.substr(i); //Return all the characters from ith index to the end of the std::string
			}
		}
		return "";
	}

// ==================== increaseOffset ====================
	bool increaseOffset(std::vector<int> & menuItemOffset,
						int j)
	{
		std::vector<int> offsetPreIncrement = menuItemOffset;
		menuItemOffset[j]++;

		//Find max of offset preincrement
		int maxPreIncrement = -1;
		for(int i = 0; i < offsetPreIncrement.size(); i++)
		{
			if(offsetPreIncrement[i] > maxPreIncrement)
			{
				maxPreIncrement = offsetPreIncrement[i];
			}
		}
		//Find max postincrement
		int maxPostIncrement = -1;
		for(int i = 0; i < menuItemOffset.size(); i++)
		{
			if(menuItemOffset[i] > maxPostIncrement)
			{
				maxPostIncrement = menuItemOffset[i];
			}
		}
		//Check the max offsets of pre and post increment with another. If they're different, we return true. If they're the same, we return false.
		if(maxPreIncrement < maxPostIncrement)
		{
			//cout << "menu length increased!";
			return true;
		}
		else
		{
			return false;
		}
	}

// ==================== verticalMenus ====================
	int verticalMenus(	std::vector< std::vector<std::string> > menus,
						std::vector<std::string> menuLabels,
						std::string borderPattern,
						bool showResponseNums,
						int maxMenuLength,
						int availableSpaceForColumns,
						bool showNone)
	{
		
		//First, we need to receive the terminal size to determine how to shape the vertical menus
		std::pair<int,int> screenSize = get_screen_size();
		int individualMenuWidth = DEFAULT_MENU_WIDTH; //This is the max length in characters for items in the menu, including the select number (6 - defenes-). DEFAULT_MENU_WIDTH is a good default value, as
									//including our borders, 4 menus and 3 borders of size 2 to separate them totals 79 characters. 
		int verticalMenusPerRow = 0; //The number of menus to display in a row
		int charactersLeftPerRow = screenSize.first; //The number of characters left in a row

		if (availableSpaceForColumns == -1) //If no size given, we'll just adjust based on window size
		{
			if(displayModes.contains(currentDisplayMode))
			{
				individualMenuWidth = displayModes[currentDisplayMode].verticalMenuWidth;
			}
			else
			{
				individualMenuWidth = DEFAULT_MENU_WIDTH;
			}
		}
		else
		{
			individualMenuWidth = availableSpaceForColumns;
		}

		//Find how many menus per row
		while(charactersLeftPerRow > individualMenuWidth)
		{
			verticalMenusPerRow++;
			charactersLeftPerRow -= individualMenuWidth;
			if(charactersLeftPerRow > individualMenuWidth+2) //If we're able to print another menu in this row, then we add a border to the right side before the next menu
			{
				charactersLeftPerRow -= 2;
			}
		}

		//Prepare the vector of selectNums based on their menu length
		int startingSelectNum = 1;
		std::vector<int> selectNumByMenu;
		selectNumByMenu.resize(menus.size(),0);
		for(size_t menuIndex = 0; menuIndex < menus.size(); menuIndex++)
		{
			if (menuIndex == 0) //
			{
				selectNumByMenu[0] = 1;
			}
			else
			{
				selectNumByMenu[menuIndex] = startingSelectNum + menus[menuIndex-1].size();
				startingSelectNum = selectNumByMenu[menuIndex];
			}
		}

		std::string formattedSelectNum = "";
		int currentBorderCharIndex = 0;
		// int	currentResponseNum = 1; // The number of the response to print in the menu (unused)
		int remainingMenusToPrint = menus.size();
		// int extraRowsForTextWrapAround = 0; // (unused)
		int menuLength = maxMenuLength;
		std::vector< int > menuItemRowOffsets;
		std::vector< std::queue<std::string> > wrappedTextQueues;
		std::vector< std::queue<std::string> > queuedMenuItems;
		std::queue<std::string> emptyQueue;
		std::vector<int> itemsPrintedPerMenu; //A std::vector that keeps track of the number of items printed from each menu
		itemsPrintedPerMenu.resize(menus.size(),0); 
		int charsRemaining = individualMenuWidth;
		while(remainingMenusToPrint > 0)
		{
			//START A NEW MENU ROW
			wrappedTextQueues.resize(menus.size(),emptyQueue);
			queuedMenuItems.resize(menus.size(),emptyQueue);
			menuItemRowOffsets.resize(menus.size(),0); //How many rows down a particular row is offset from displaying its contents on the standard indices
			if(verticalMenusPerRow > remainingMenusToPrint) //If we don't have to print a full row of vertical menus, we just print the remainder of menus
			{
				verticalMenusPerRow = remainingMenusToPrint;
			}
			for(size_t menuColumnIndex = 0; menuColumnIndex < verticalMenusPerRow; menuColumnIndex++) //Loop over the menu labels to print them
			{
				charsRemaining = individualMenuWidth;
				std::string overflowText = localizedWrap(menuLabels[menuColumnIndex],charsRemaining); //Prints the menu label. If it's too long, then we return the part of the label that wraps belong the allotted space.
				charsRemaining -= menuLabels[menuColumnIndex].length();
				if(overflowText != "")
				{
					wrappedTextQueues[menuColumnIndex].push(overflowText);
					if(increaseOffset(menuItemRowOffsets,menuColumnIndex))
					{
						menuLength++;
					}
				}
				while(charsRemaining > 0) //If we have leftover space in a place where a menu label is printed, then we print spaces until we have none left
				{
					std::cout << " ";
					charsRemaining--;
				}
				if(menuColumnIndex+1 < verticalMenusPerRow) //If we still have menus left to print, then we print a border
				{
					std::cout << " ";
					if(borderPattern.length() > 0)
					{
						std::cout << borderPattern[currentBorderCharIndex];
					}
				}
			}
			std::cout << "\n"; //Go to the next line after printing the menu labels
			//Go to the next border pattern character because we go to the next line
			currentBorderCharIndex++;
			if(currentBorderCharIndex >= borderPattern.length())
			{
				currentBorderCharIndex = 0;
			}

			/*
			if(menus[j].size() > maxMenuLength)
			{
				//Insert a "view more" option in the last row of the menu

				//This having happened, we need a way for the function to tell the place where it has been called that selecting the choice of "view more"
				//should recall this vertical menus function to show a different part of the menu content
			}
			*/

			std::vector<int> emptyMenuIndices = {};
			/// Find empty menus ///
			for(size_t emptyCheckIndex = 0; emptyCheckIndex < menus.size(); emptyCheckIndex++)
			{
				if(menus[emptyCheckIndex].size() == 0)
				{
					emptyMenuIndices.push_back(1);
				}
				else
				{
					emptyMenuIndices.push_back(0);
				}
			}

			/// PRINT ITEMS IN MENU ///
			int itemRowIndex = 0;
			while(itemRowIndex < menuLength) //itemRowIndex is the variable indicating the inner index of indexing menus: menus[menuColumnIndex][itemRowIndex], here we begin printing line by line the items in the menus
			{
				for(size_t menuColumnIndex = 0; menuColumnIndex < menus.size(); menuColumnIndex++) //menuColumnIndex is the index of the current row of menus, the outer index of menus
				{
					charsRemaining = individualMenuWidth;
					//cout << "itemRowIndex: " << itemRowIndex << " " << "menuColumnIndex: " << menuColumnIndex;
					//We need to check to see if there exists an element at index itemRowIndex in menu menuColumnIndex
					if (itemRowIndex >= menus[menuColumnIndex].size() && wrappedTextQueues[menuColumnIndex].empty() && queuedMenuItems[menuColumnIndex].empty()) //If we index past the size of a list and we have no wrapped text to print
					{
						//If a menu is empty and we want to show "None" in empty menus, we print "None" here
						if(emptyMenuIndices[menuColumnIndex] == 1 && showNone)
						{
							std::cout << "None";
							emptyMenuIndices[menuColumnIndex] = 2;
							charsRemaining -= 4;
							while (charsRemaining > 0)
							{
								std::cout << " ";
								charsRemaining--;
							}
							goto printBorder;
							continue;
						}
						//Otherwise, we print empty spaces
						//cout << "print empty space";
						charsRemaining = individualMenuWidth;
						while(charsRemaining > 0)
						{
							std::cout << " ";
							charsRemaining--;
						}
					}
					else
					{
						if (!wrappedTextQueues[menuColumnIndex].empty()) //If we have text wrapping over from the line above, then we print it
						{
							//cout << "wrap";
							std::string textToPrint = wrappedTextQueues[menuColumnIndex].front();
							std::string overflowText = localizedWrap(textToPrint,charsRemaining);
							charsRemaining -= textToPrint.length();
							wrappedTextQueues[menuColumnIndex].pop();
							if(overflowText != "")
							{
								//cout << "text wrapped again";
								wrappedTextQueues[menuColumnIndex].push(overflowText);
								if(increaseOffset(menuItemRowOffsets,menuColumnIndex))
								{
									menuLength++;
								}
							}
							else
							{
								itemsPrintedPerMenu[menuColumnIndex]++;
							}
							while (charsRemaining > 0)
							{
								std::cout << " ";
								charsRemaining--;
							}
							if(itemsPrintedPerMenu[menuColumnIndex] < menus[menuColumnIndex].size()) //If we have a menu item to print after the wraparound text, we std::queue it here
							{
								queuedMenuItems[menuColumnIndex].push(menus[menuColumnIndex][itemRowIndex+1-menuItemRowOffsets[menuColumnIndex]]);
							}
						}
						else if (!queuedMenuItems[menuColumnIndex].empty()) //If we have some menu items that have been queued to be printed...
						{
							//cout << "queuedprint";
							std::string queuedItem = queuedMenuItems[menuColumnIndex].front();
							if (showResponseNums) //Print the response number if it is requested
							{
								formattedSelectNum = std::to_string(selectNumByMenu[menuColumnIndex]) + " - ";
								selectNumByMenu[menuColumnIndex]++;
								charsRemaining -= formattedSelectNum.length();
								std::cout << formattedSelectNum;
							}
							std::string overflowText = localizedWrap(queuedItem,charsRemaining);
							charsRemaining -= queuedItem.length();
							queuedMenuItems[menuColumnIndex].pop();
							if(overflowText != "")
							{
								wrappedTextQueues[menuColumnIndex].push(overflowText);
								if(increaseOffset(menuItemRowOffsets,menuColumnIndex))
								{
									// Note: menuLength increment removed here - appears to cause issues with this specific case
									// The increaseOffset call still updates the offset, but we don't increment menuLength
									// This may need further investigation to understand why this case differs
								}
							}
							else
							{
								itemsPrintedPerMenu[menuColumnIndex]++;
								//We need to check to queue other content here that has been passed over by wrapping
								if(itemsPrintedPerMenu[menuColumnIndex] < menus[menuColumnIndex].size())
								{
									queuedMenuItems[menuColumnIndex].push(menus[menuColumnIndex][itemRowIndex+1-menuItemRowOffsets[menuColumnIndex]]);
								}
							}
							while (charsRemaining > 0)
							{
								std::cout << " ";
								charsRemaining--;
							}
						}
						else //If we have no text wrapping over from the line above and no items in the std::queue to print, then we print at the current row and column
						{
							//cout << "stdprint";
							if (showResponseNums) //Print the response number if it is requested
							{
								formattedSelectNum = std::to_string(selectNumByMenu[menuColumnIndex]) + " - ";
								selectNumByMenu[menuColumnIndex]++;
								charsRemaining -= formattedSelectNum.length();
								std::cout << formattedSelectNum;
							}
							std::string overflowText = localizedWrap(menus[menuColumnIndex][itemRowIndex-menuItemRowOffsets[menuColumnIndex]],charsRemaining);
							charsRemaining -= menus[menuColumnIndex][itemRowIndex-menuItemRowOffsets[menuColumnIndex]].length();
							if(overflowText != "")
							{
								//cout << "text wrapped";
								wrappedTextQueues[menuColumnIndex].push(overflowText);
								//extraRowsForTextWrapAround++;
								//menuLength++;
								if(increaseOffset(menuItemRowOffsets,menuColumnIndex))
								{
									//cout << "menuItemRowOffsets[" << menuColumnIndex << "]: " << menuItemRowOffsets[menuColumnIndex] << endl;
									menuLength++;
								}
								//cout << "menuOffset: " << menuItemRowOffsets[menuColumnIndex];
							}
							else
							{
								itemsPrintedPerMenu[menuColumnIndex]++;
							}
							while (charsRemaining > 0)
							{
								std::cout << " ";
								charsRemaining--;
							}

							//cout << "standard text printed";
						}
					}

					printBorder: //Jump point for printing the border
					if(menuColumnIndex+1 < menus.size()) //Print menu border
					{
						std::cout << " ";
						if(borderPattern.length() > 0)
						{
							std::cout << borderPattern[currentBorderCharIndex];
						}
					}
					//cout << "row done";
					//cout << menuLength;
				}
				//Go to the next line if we have more menus to print
				if((itemRowIndex + 1 < menuLength) && (remainingMenusToPrint-verticalMenusPerRow >= 0))
				{
					std::cout << "\n";
				}
				currentBorderCharIndex++;
				if(currentBorderCharIndex >= borderPattern.length())
				{
					currentBorderCharIndex = 0;
				}
				itemRowIndex++;
				//cout << menuLength << endl;
			}
			//cout << "menuLength:" << menuLength;
			remainingMenusToPrint -= verticalMenusPerRow;
		}

		/*
		Structure of return std::tuple:
		0 - Last selectnum used in the vertical menu
		1 - A std::vector, where each index represents a vertical from left to right. The number stored at each index indicates the selectnum
			of the menu which can be selected to view more of the vertical menu in question.
		*/
		std::vector<int> viewMoreNums;
		std::tuple<int,std::vector<int> > returnTuple(selectNumByMenu.back(),viewMoreNums);
		return selectNumByMenu.back();
	}

// ==================== promptMultipleResponses ====================
	void promptMultipleResponses(	std::vector<std::string> responses,
									int columns, int rows,
									int startingResponseNum,
									bool showResponseNums	)
	{
		// There's a seg fault in here somewhere

		std::string stringToPrint = ""; //This will be the final std::string we print after we decide where all of the responses will be ordered
		int selectNum = startingResponseNum-1;  

		if (responses.size() <=  rows) //If the given amount of rows is greater than or equal to the number of responses, we can just print out our responses vertically in a single column
		{
			for(size_t responseIndex = 0; responseIndex < responses.size(); responseIndex++)
			{
				selectNum++;
				if (showResponseNums)
				{
					print(std::to_string(selectNum) + " - " + stevensStringLib::cap1stChar(responses[responseIndex]) + "\n");
				}
				else
				{
					print(stevensStringLib::cap1stChar(responses[responseIndex]) + "\n");
				}
			}
		}
		else //The following is the main purpose of this function: We print out a series of columns and rows of responses that ascend in selectNum by vertically down columns, left to right
		{
			// int totalResponseSlots = columns * rows; //total spaces in the grid to fill (unused)
			int responsesToCount = responses.size();
			std::vector< std::vector<std::string> > responseGrid; //responseGrid[column][response in row of index]
			std::vector<std::string> emptyVec;
			emptyVec.resize(rows); //mske the columns contain the number of rows we designate in the function call
			int columnIndex = 0;
			//cout << "Just before while loop\n";
			while(responsesToCount > 0) //Go through each column and fill a column std::vector with responses
			{
				responseGrid.push_back(emptyVec); //add a column here
				//cout << "column added\n";
				for(int rowIndex = 0; rowIndex < rows; rowIndex++) //Add a response to each row of a column
				{
					selectNum++;
					if (responsesToCount > 0)
					{
						//cout << responses.front();
						if (showResponseNums)
						{
							responseGrid[columnIndex][rowIndex] = std::to_string(selectNum) + " - " + stevensStringLib::cap1stChar(responses.front());
						}
						else
						{
							responseGrid[columnIndex][rowIndex] = stevensStringLib::cap1stChar(responses.front());
						}
						//cout << "response added\n";
						responses.erase(responses.begin());
						responsesToCount--;
					}
					else
					{
						//cout << "we breakin\n";
						break;
					}
				}
				columnIndex++;
			}
			//Now we have our responses in a grid/2D vector structure. We can print them out in lines now.
			for(int rowIndex = 0; rowIndex < rows; rowIndex++)
			{
				for(int columnIndex = 0; columnIndex < columns; columnIndex++) //Print each column after each other separated by tabs
				{
					//void(0);
					if (responseGrid[columnIndex][rowIndex].length() < MIN_CELL_WIDTH)
					{
						while(true)
						{
							responseGrid[columnIndex][rowIndex] += " ";
							if (responseGrid[columnIndex][rowIndex].length() == MIN_CELL_WIDTH)
							{
								break;
							}
						}
					}
					std::cout << responseGrid[columnIndex][rowIndex] + "\t";
				}
				std::cout << "\n"; //Once we finish a row we print a newline
			}
		}
		std::cout << "\n"; //print newline when finished
	}

// ==================== curses_wprint ====================
	void curses_wprint( WINDOW * win,
						int yMove,
						int xMove,
						std::string printString,
						std::unordered_map<std::string,std::string> style,
						std::unordered_map<std::string,std::string> format )
	{
		PrintHelper::curses_wprint(	win,
												yMove,
												xMove,
												printString,
												style,
												format,
												textStyling	);
	}

// ==================== curses_wClearLine ====================
	void curses_wClearLine(WINDOW* win, int y, std::unordered_map<std::string, std::string> settings )
	{
		int height, width;
		getmaxyx(win, height, width);

		bool avoidBorders = (settings.contains("avoid borders") && settings.at("avoid borders") == "true");

		int startX = avoidBorders ? 1 : 0;
		int endX = avoidBorders ? width - 1 : width;

		// Move to the line and clear it with spaces
		wmove(win, y, startX);
		for (int x = startX; x < endX; ++x)
		{
			waddch(win, ' ');
		}
	}

// ==================== formatTableAsString ====================
	std::string formatTableAsString(	std::vector< std::vector<std::string> > table,
										const std::unordered_map<std::string, std::string> & style,
										std::unordered_map<std::string, std::string> format	)
	{
		std::string printString = "";

		//Determine how wide we should make the columns
		// unsigned int columnWidth; (unused)
		std::vector<unsigned int> columnWidths;

		// Check if explicit column widths were provided
		if(format.contains("column widths"))
		{
			// Parse comma-separated column widths like "24,12,12,10,10"
			std::vector<std::string> widthStrings = stevensStringLib::separate(format["column widths"], ',');
			for(const auto& widthStr : widthStrings)
			{
				try
				{
					columnWidths.push_back(std::stoi(widthStr));
				}
				catch(const std::invalid_argument& e)
				{
					std::cerr << "Error: Invalid column width '" << widthStr << "' - must be a valid integer" << std::endl;
					return ""; // Return empty std::string on error
				}
				catch(const std::out_of_range& e)
				{
					std::cerr << "Error: Column width '" << widthStr << "' is out of range" << std::endl;
					return ""; // Return empty std::string on error
				}
			}
		}
		else if(!format.contains("column width") || format["column width"] == "use width of largest entry")
		{
			//If we don't include a column width key, then we just use width of largest entry
			//To find the width of the largest entry for each column, we first make a copy of our 2D vector that is a vector of columns, where each element is a different row in the column
			std::vector< std::vector<std::string> > tableByColumnsOfRows = stevensVectorLib::reorient2DVector(table);
			//Remove all style tokens from the table (if desired, as they can mess up column sizing)
			for(int i = 0; i < tableByColumnsOfRows.size(); i++)
			{
				for(int n = 0; n < tableByColumnsOfRows[i].size(); n++)
				{
					tableByColumnsOfRows[i][n] = removeAllStyleTokenization(tableByColumnsOfRows[i][n]);
				}
			}
			//For each column, discover the string element with the greatest length and store it to that column's index
			for(int i = 0; i < tableByColumnsOfRows.size(); i++)
			{
				std::string longestStringElement = stevensVectorLib::getLongestStringElement(tableByColumnsOfRows[i]);
				columnWidths.push_back(longestStringElement.length());
			}
		}

		// Check if wrapping is enabled
		bool enableWrapping = format.contains("enable wrapping") && format.at("enable wrapping") == "true";

		//Concatenate each row
		for(int row = 0; row < table.size(); row++)
		{
			if(enableWrapping)
			{
				// Wrap cells and build multi-line row
				std::vector<std::vector<std::string>> wrappedCells; // Each cell becomes std::vector of lines
				std::vector<std::string> originalStyledCells; // Store original styled text
				int maxLines = 1; // Track maximum lines needed for this row

				// Wrap each cell
				for(int col = 0; col < table[row].size(); col++)
				{
					std::string originalStyled = table[row][col];
					originalStyledCells.push_back(originalStyled);
					std::string cellContent = removeAllStyleTokenization(originalStyled);

					// Extract style token from original (format: {text}$[style])
					std::string styleToken = "";
					size_t styleStart = originalStyled.find("}$[");
					if(styleStart != std::string::npos)
					{
						size_t styleEnd = originalStyled.find("]", styleStart);
						if(styleEnd != std::string::npos)
						{
							styleToken = originalStyled.substr(styleStart + 1, styleEnd - styleStart); // Include $[ and ]
						}
					}

					std::vector<std::string> lines;

					// Calculate indentation for wrapped lines in first column (response numbers like "1 - ")
					int indentSize = 0;
					if(col == 0 && row > 0) // Skip header row
					{
						// Find " - " pattern which separates response number from text
						size_t dashPos = cellContent.find(" - ");
						if(dashPos != std::string::npos)
						{
							indentSize = dashPos + 3; // Include the " - " part
						}
					}

					// Wrap the cell content at column width
					int firstLineWidth = columnWidths[col];
					int wrappedLineWidth = columnWidths[col] - indentSize;

					// First line uses full width
					if(cellContent.length() > firstLineWidth)
					{
						lines.push_back("{" + cellContent.substr(0, firstLineWidth) + "}" + styleToken);
						cellContent = cellContent.substr(firstLineWidth);

						// Subsequent lines are indented and styled
						while(cellContent.length() > wrappedLineWidth && wrappedLineWidth > 0)
						{
							lines.push_back("{" + std::string(indentSize, ' ') + cellContent.substr(0, wrappedLineWidth) + "}" + styleToken);
							cellContent = cellContent.substr(wrappedLineWidth);
						}
						// Add remaining content with indentation and styling
						if(!cellContent.empty())
						{
							lines.push_back("{" + std::string(indentSize, ' ') + cellContent + "}" + styleToken);
						}
					}
					else
					{
						// No wrapping needed - use original styled text
						lines.push_back(originalStyled);
					}

					// If cell was empty, add one empty line
					if(lines.empty())
					{
						lines.push_back("");
					}

					wrappedCells.push_back(lines);
					maxLines = std::max(maxLines, (int)lines.size());
				}

				// Print each line of the row
				for(int lineNum = 0; lineNum < maxLines; lineNum++)
				{
					for(int col = 0; col < wrappedCells.size(); col++)
					{
						std::string lineToPrint;
						if(lineNum < wrappedCells[col].size())
						{
							lineToPrint = wrappedCells[col][lineNum];
						}
						else
						{
							lineToPrint = ""; // Empty if this cell doesn't have this many lines
						}

						// Lines already have styling applied, just resize
						lineToPrint = resizeStyledString(lineToPrint, columnWidths[col]);
						printString += lineToPrint + format["column spacing std::string"];
					}
					printString += "\n";
				}
			}
			else
			{
				// Original non-wrapping behavior
				//Concatenate each column
				for(int col = 0; col < table[row].size(); col++)
				{
					std::string cell = table[row][col];
					//Resize the cell based on our given column size
					cell = resizeStyledString(cell, columnWidths[col]);
					printString += cell + format["column spacing std::string"];
				}
				printString += "\n"; //+ stevensStringLib::multiply(" ", yMove);
			}
		}

		return printString;
	}

// ==================== curses_wborder ====================
	void curses_wborder(	WINDOW * win,
							std::unordered_map<std::string,std::string> borderPatterns,
							std::unordered_map<std::string,std::string> styleMap	)
	{
		int height;
		int width;
		std::string strToPrint = "";

		//Very important - get the window size
		getmaxyx(win, height, width); //Curses function to get the width of the window we are printing to

		/*** Check for key value pairs in the borderPatterns map and draw borders from them ***/
		if(borderPatterns.contains("left"))
		{
			//TODO - Make a vertical print function to use for these
			for(int i = 0; i < height; i++)
			{
				strToPrint = stevensStringLib::circularIndex(borderPatterns["left"], i);
				curses_wprint(	win,
								i,
								0,
								strToPrint,
								styleMap,
								{}	);
			}
		}
		if(borderPatterns.contains("right"))
		{
			for(int i = 0; i < height; i++)
			{
				strToPrint = stevensStringLib::circularIndex(borderPatterns["right"], i);
				curses_wprint( 	win,
								i,
								width-1,
								strToPrint,
								styleMap,
								{} );
			}
		}
		if(borderPatterns.contains("top"))
		{
			//If the top border pattern is greater than or equal to the window width, print all the characters of
			//the pattern equal up to the width size
			if(borderPatterns["top"].length() >= width)
			{
				curses_wprint( 	win,
								0, 
								0,
								borderPatterns["top"].substr(0, width),
								styleMap,
								{}	);
			}
			//If the border pattern is less than the window size, multiply the pattern by an amount equal to the width
			//divided by the border pattern length. To get the string that we'll print.
			else
			{
				std::string strToPrint = stevensStringLib::multiply(borderPatterns["top"], (width/borderPatterns["top"].length()) );
				//If the resulting border pattern string is not equal to the width, add/subtract
				//individual characters from the border pattern until we have a string equal to the window width
				int i = 0;
				while(strToPrint.length() != width)
				{
					if(strToPrint.length() > width)
					{
						strToPrint.pop_back();
					}
					else if(strToPrint.length() < width)
					{
						strToPrint.append(std::string(stevensStringLib::circularIndex(borderPatterns["top"], i),1));
						i++;
					}
				}
				//Now we can print the string!
				curses_wprint( 	win,
								0, 
								0,
								strToPrint,
								styleMap,
								{}	);
			}
		}
		if(borderPatterns.contains("bottom"))
		{
			//If the top border pattern is greater than or equal to the window width, print all the characters of
			//the pattern equal up to the width size
			if(borderPatterns["bottom"].length() >= width)
			{
				curses_wprint( 	win,
								height-1, 
								0,
								borderPatterns["bottom"].substr(0, width),
								styleMap,
								{}	);
			}
			//If the border pattern is less than the window size, multiply the pattern by an amount equal to the width
			//divided by the border pattern length. To get the string that we'll print.
			else
			{
				std::string strToPrint = stevensStringLib::multiply(borderPatterns["bottom"], (width/borderPatterns["bottom"].length()) );
				//If the resulting border pattern string is not equal to the width, add/subtract
				//individual characters from the border pattern until we have a string equal to the window width
				int i = 0;
				while(strToPrint.length() != width)
				{
					if(strToPrint.length() > width)
					{
						strToPrint.pop_back();
					}
					else if(strToPrint.length() < width)
					{
						strToPrint.append(std::string(stevensStringLib::circularIndex(borderPatterns["bottom"], i),1));
						i++;
					}
				}
				//Now we can print the string!
				curses_wprint( 	win,
								height-1, 
								0,
								strToPrint,
								styleMap,
								{}	);
			}
		}
		if(borderPatterns.contains("top-left corner"))
		{
			curses_wprint(	win,
							0,
							0,
							borderPatterns["top-left corner"],
							styleMap,
							{}	);
		}
		if(borderPatterns.contains("top-right corner"))
		{
			curses_wprint(	win,
							0,
							width-1,
							borderPatterns["top-right corner"],
							styleMap,
							{}	);
		}
		if(borderPatterns.contains("bottom-left corner"))
		{
			curses_wprint( 	win,
							height-1,
							0,
							borderPatterns["bottom-left corner"],
							styleMap,
							{}	);
		}
		if(borderPatterns.contains("bottom-right corner"))
		{
			curses_wprint(	win,
							height-1,
							width-1,
							borderPatterns["bottom-right corner"],
							styleMap,
							{}	);
		}
	}

// ==================== curses_wEraseHLine ====================
	void curses_wEraseHLine(	WINDOW * & win,
							 	int y,
								std::unordered_map<std::string,std::string> settingsMap )
	{
		//Get the width of the window
		int windowWidth = getmaxx(win);
		//Define where we'll start erasing our horizontal line in the window
		int x = 0;
		//If we are avoiding erasing the borders, change our parameters here
		if(settingsMap.contains("avoid borders"))
		{
			if(stevensStringLib::stringToBool(settingsMap["avoid borders"]))
			{
				//Avoid erasing the lefthand border
				x = 1;
				//Avoid erasing the righthand border
				windowWidth -= 2;
			}
		}
		//Erase the horizontal line in the window
		mvwprintw(	win,
					y,
					x,
					"%s",
					stevensStringLib::multiply(" ", windowWidth).c_str()	);
	}

// ==================== printFile ====================
	void printFile(	std::string textFilePath,
					bool wrap	)
	{
		std::ifstream input_file(textFilePath);
		if (!input_file.is_open())
		{
			std::cerr << "Could not open the file - '" << textFilePath << "'" << std::endl;
			exit(EXIT_FAILURE);
		}
		std::string textFile = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
		print(textFile, {}, {{"wrap",stevensStringLib::boolToString(wrap)}});
	}

// ==================== curses_wprintFile ====================
	void curses_wprintFile(	WINDOW * win,
							std::string filePath,
							std::unordered_map<std::string,std::string> style,
							std::unordered_map<std::string,std::string> format	)
	{
		//Create an input stream from the file we are trying to print
		std::ifstream input_file(filePath);
		if (!input_file.is_open())
		{
			//Error if we cannot successfully print the file
			throw std::invalid_argument("Error, could not find file: " + filePath);
		}
		//Store the text content of the file in a string
		std::string fileContent = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());

		/*** Formatting ***/
		//A lot of textfiles contain a last newline to indicate the end of the file. We can choose to remove that here.
		if(format.contains("ignore last newline"))
		{
			if(stevensStringLib::stringToBool(format["ignore last newline"]))
			{
				//Find the last newline in the file content and remove it
				int positionOfLastNewLine = fileContent.rfind('\n');
				if(positionOfLastNewLine != std::string::npos)
				{
					fileContent.replace(positionOfLastNewLine, 1, "");
				}
			}
		}

		//COMMENTED OUT (We already do this in the wprint function below)
		//If we are printing the file to a bordered window, check to see if we're avoiding printing on the border
		int yMove, xMove = 0; 
		if(format.contains("avoid borders"))
		{
			if(stevensStringLib::stringToBool(format["avoid borders"]))
			{
				//Start printing the text within the borders
				yMove = 1;
				xMove = 1;
			} 
		}

		//Print to the window
		curses_wprint(	win,
						yMove,
						xMove,
						fileContent,
						style,
						format	);
	}

// ==================== recolorText ====================
#ifdef _WIN32
	void recolorText(std::string textColor, std::string bgColor)
	{
		HANDLE  hConsole;
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		int bgColorCode = 0;
		int textColorCode = 15;
		
		if (bgColor != "default")
		{
			if (bgColor == "red")
			{
				bgColorCode = 12;
			}
			else if (bgColor == "cyan")
			{
				bgColorCode = 11;
			}
			else if (bgColor == "yellow")
			{
				bgColorCode = 14;
			}
			else if (bgColor == "green")
			{
				bgColorCode = 10;
			}
			else if (bgColor == "blue")
			{
				bgColorCode = 9;
			}
			else if (bgColor == "purple")
			{
				bgColorCode = 13;
			}
			else if (bgColor == "white")
			{
				bgColorCode = 15;
			}
		}
		
		if (textColor != "default")
		{
			if (textColor == "red")
			{
				textColorCode = 12;
			}
			else if (textColor == "cyan")
			{
				textColorCode = 11;
			}
			else if (textColor == "yellow")
			{
				textColorCode = 14;
			}	
			else if (textColor == "green")
			{
				textColorCode = 10;
			}
			else if (textColor == "blue")
			{
				textColorCode = 9;
			}
			else if (textColor == "purple")
			{
				textColorCode = 13;
			}
			else if (textColorCode == "white")
			{
				textColorCode = 15;
			}
		}
		
		SetConsoleTextAttribute(hConsole, textColorCode + (bgColorCode*16));	
	}
#endif // _WIN32

// ==================== print_overload2 ====================
	void print(	std::string input,
				std::unordered_map<std::string, std::string> style,
				std::unordered_map<std::string, std::string> format	)
	{
		PrintHelper::print(	input,
									style,
									format,
									textStyling,
									get_screen_size(),
									displayModes,
									currentDisplayMode	);
	}

// ==================== menuLabel ====================
	void menuLabel(std::string label)
	{
		std::cout << label << "\n\n";
	}

// ==================== responseVerify ====================
	bool responseVerify(std::string response, std::vector<std::string> desiredResponse)
	{
		for (int i = 0; i < desiredResponse.size(); i++)
		{
			if (response == desiredResponse[i])
			{
				return true;
			}
		}
		return false;
	}

} // namespace stevensTerminal (moved functions)
