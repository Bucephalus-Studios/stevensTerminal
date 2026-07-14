/**
 * stevensTerminal.cpp
 * Implementation file for stevensTerminal library
 * Migrated from header-only to compiled library for faster build times
 */

#include "stevensTerminal.hpp"

#if defined(__linux__)
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include <limits>
#include <fstream>

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

void showCursor()
{
    #if defined(_WIN32)
        // PDCurses' wincon backend maps curs_set(1) ("normal") to the
        // console's original/default cursor size (PDC_curs_set(), SP->
        // orig_cursor), which on the standard Windows console is a thin
        // underline - unlike Linux terminals, where a "normal" cursor
        // typically fills the whole cell. curs_set(2) ("high visibility")
        // maps to a 95%-of-cell-height block on wincon, matching Linux's
        // appearance.
        curs_set(2);
    #else
        curs_set(1);
    #endif
}

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

void setTerminalTitle(const std::string & title)
{
    #if defined(_WIN32)
        PDC_set_title(title.c_str());
    #elif defined(__linux__)
        std::cout << "\033]0;" << title << "\007" << std::flush;
    #endif
}

void resizeTerminalWindow(int rows, int cols)
{
    #if defined(_WIN32)
        resize_term(rows, cols);
    #endif
    // No portable way to resize the user's terminal emulator window on
    // Linux/macOS - intentionally a no-op there.
}

void enableTrueBlink()
{
    #if defined(_WIN32)
        // PDCursesMod defaults A_BLINK to setting the background to high
        // intensity instead of actually blinking (PDC_set_blink(FALSE) is
        // the default on most platforms, per pdcsetsc.c) - without this
        // call, blink=true text just renders as a bright/light background
        // and never flashes. Real terminals (Linux/macOS) already treat
        // A_BLINK as a true blink via terminfo, so this is a no-op there.
        PDC_set_blink(TRUE);
    #endif
}

void initialize(bool initWindowManager,
                const std::vector<std::string>& windowNames) {
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
}

void shutdown() {
    windowManager().shutdown();  // This handles both window cleanup and endwin()
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
	// NOTE: computed from the base colors' own (already bit-order-correct)
	// values rather than hardcoded absolute numbers. PDCurses' COLOR_RED/
	// COLOR_BLUE/etc. resolve differently depending on whether PDC_RGB is
	// defined (RGB vs. legacy PC BGR bit ordering) - hardcoding e.g.
	// "bright-yellow = 11" silently assumed RGB ordering (yellow=3, +8=11),
	// which is wrong under PDCursesMod's Windows default (BGR, where 11 is
	// actually the bright-cyan slot) - this is exactly why yellow rendered
	// as cyan and blue rendered as red on Windows.
	if(COLORS >= 16)
	{
		curses_colors["bright-black"]   = curses_colors["black"]   + 8;
		curses_colors["bright-red"]     = curses_colors["red"]     + 8;
		curses_colors["bright-green"]   = curses_colors["green"]   + 8;
		curses_colors["bright-yellow"]  = curses_colors["yellow"]  + 8;
		curses_colors["bright-blue"]    = curses_colors["blue"]    + 8;
		curses_colors["bright-magenta"] = curses_colors["magenta"] + 8;
		curses_colors["bright-cyan"]    = curses_colors["cyan"]    + 8;
		curses_colors["bright-white"]   = curses_colors["white"]   + 8;
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
        str = stevensStringLib::utf8Resize(str, desiredLength, fillChar);
        return str;
    }

    // Otherwise, now that we have all of the style tokens, we take just their content and turn it into a std::string
    std::string contentString = removeAllStyleTokenization(str);

    // getAllTokens() reports each token's existsAtIndex as a byte position in the ORIGINAL
    // (fully tokenized) string, but insertStyleToken() below needs each token's position within
    // contentString (the token-stripped string) to reinsert it correctly. Adjust for the wrapper
    // overhead ("{" + "}$[" + style + "]") of every earlier token, which contentString doesn't
    // have - without this, only a single leading token's position is ever correct by coincidence;
    // a second (or later) token's existsAtIndex is too large by the combined wrapper overhead of
    // every token before it, so it gets reinserted at the wrong spot. Byte-based on purpose (see
    // insertStyleToken()'s std::string::insert() calls below) - safe for multi-byte content because
    // rawToken.length() - content.length() always cancels content's own length out, leaving just
    // the (always-ASCII) markup's byte count, regardless of what content actually contains.
    size_t wrapperOverhead = 0;
    for(PrintToken & token : tokens)
    {
        token.existsAtIndex -= wrapperOverhead;
        wrapperOverhead += token.rawToken.length() - token.content.length();
    }

    // We resize this std::string by codepoint, not byte, so multi-byte content (Cyrillic, CJK, etc.)
    // isn't torn in half by truncation or padded to the wrong displayed width
    contentString = stevensStringLib::utf8Resize(contentString, desiredLength, fillChar);
    std::string resizedStr = contentString;
    // We now place the style tokens back into this resized std::string. insertStyleToken() mutates
    // resizedStr in place, growing it by each token's wrapper overhead - so every later token's
    // existsAtIndex (already adjusted above to be correct against the original content-only string)
    // also needs to account for the growth every earlier-in-this-loop insertion just added, or it
    // lands wherever the string happened to be *before* those insertions instead of where the
    // content actually ended up.
    size_t reinsertionGrowth = 0;
    for(int tokenIndex = 0; tokenIndex < tokens.size(); tokenIndex++) {
        tokens.at(tokenIndex).existsAtIndex += reinsertionGrowth;
        // For each style token, check to see if the index which it starts relative to the content and all other tokens
        // is still in bounds of the string's length
        if(tokens.at(tokenIndex).existsAtIndex+1 < resizedStr.length()) {
            // If so, then we add the style token back at its original location
            insertStyleToken(resizedStr, tokens.at(tokenIndex));
            reinsertionGrowth += tokens.at(tokenIndex).rawToken.length() - tokens.at(tokenIndex).content.length();
        } else {
            // Otherwise, all other style tokens will no longer be in bounds either, as they come after the current one
            // in order, so we break and finish
            break;
        }
    }

    // Return the resized styled std::string
    return resizedStr;
}

// NOTE: printHorizontalBorder() (plain std::cout ANSI-styling-era border
// printer) was removed — unreachable from anywhere in cultgame or elsewhere
// in this library. Use curses_wprint()-based rendering instead.

// ==================== INPUT.HPP IMPLEMENTATIONS ====================

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// NOTE: the plain no-arg input() overload (std::cin/std::getline-based, from
// before the WINDOW*-based input() existed) was removed — unreachable from
// anywhere in cultgame or elsewhere in this library.

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
#if defined(__linux__)
    // set_escdelay() is an ncurses extension (delay before treating a lone ESC
    // as a keypress rather than the start of a function-key sequence). PDCurses
    // has no equivalent and doesn't need one: the Win32 console API reports
    // key events individually rather than as ambiguous escape sequences.
    set_escdelay(25);
#endif

    std::unordered_map<std::string,chtype> styleAttrs = PrintHelper::curses_styleAttributes(textStyle);

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

// NOTE: table() (plain std::cout ANSI-styling-era table printer) was removed
// — unreachable from anywhere in cultgame or elsewhere in this library.

// NOTE: localizedWrap(), increaseOffset(), verticalMenus(), and
// promptMultipleResponses() (plain std::cout ANSI-styling-era menu/response
// printers) were removed — unreachable from anywhere in cultgame or elsewhere
// in this library (verticalMenus is only ever referenced in README examples).

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

// ==================== curses_wprintDirect ====================
	void curses_wprintDirect(	WINDOW * win,
								int yMove,
								int xMove,
								const std::string & printString,
								const std::unordered_map<std::string,std::string> & style )
	{
		std::unordered_map<std::string, chtype> attrs = PrintHelper::curses_styleAttributes(style);

		PrintHelper::curses_wAttrOn(win, attrs);
		mvwaddstr(win, yMove, xMove, printString.c_str());
		PrintHelper::curses_wAttrOff(win, attrs);
	}

// ==================== curses_wGetAttrs ====================
	chtype curses_wGetAttrs( WINDOW * win )
	{
		return getattrs(win);
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
			//For each column, discover the string element with the greatest display width and store it to that column's index
			for(int i = 0; i < tableByColumnsOfRows.size(); i++)
			{
				std::string widestStringElement = stevensVectorLib::getStringWithMaxDisplayWidth(tableByColumnsOfRows[i]);
				columnWidths.push_back(stevensStringLib::lineDisplayWidth(widestStringElement));
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

					// Extract the style map from the original cell content, if any, using the shared
					// token parser (PrintTokenHelper::getAllTokens() + PrintToken::getStyleMap())
					// instead of ad-hoc substring searching, so this matches the same nesting-aware
					// parsing used everywhere else tokens are read.
					std::vector<PrintToken> originalTokens = PrintTokenHelper::getAllTokens(originalStyled);
					std::unordered_map<std::string,std::string> cellStyle =
						originalTokens.empty() ? std::unordered_map<std::string,std::string>{} : originalTokens[0].getStyleMap();

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

					// Decode once and work in codepoints (not bytes) for this cell's wrapping, so
					// multi-byte content (Cyrillic, CJK, etc.) is measured/cut correctly instead of
					// being torn apart by a byte-offset substr - see stevensStringLib::utf8to32()
					// doc comment. Only re-encoded (via utf32to8()) when a line is finished.
					std::u32string u32cellContent = stevensStringLib::utf8to32(cellContent);

					// First line uses full width
					if((int)u32cellContent.length() > firstLineWidth)
					{
						lines.push_back(stevensTerminal::style(stevensStringLib::utf32to8(u32cellContent.substr(0, firstLineWidth)), cellStyle));
						u32cellContent = u32cellContent.substr(firstLineWidth);

						// Subsequent lines are indented and styled
						while((int)u32cellContent.length() > wrappedLineWidth && wrappedLineWidth > 0)
						{
							lines.push_back(stevensTerminal::style(std::string(indentSize, ' ') + stevensStringLib::utf32to8(u32cellContent.substr(0, wrappedLineWidth)), cellStyle));
							u32cellContent = u32cellContent.substr(wrappedLineWidth);
						}
						// Add remaining content with indentation and styling
						if(!u32cellContent.empty())
						{
							lines.push_back(stevensTerminal::style(std::string(indentSize, ' ') + stevensStringLib::utf32to8(u32cellContent), cellStyle));
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
			if(stevensStringLib::charCount(borderPatterns["top"]) >= width)
			{
				curses_wprint( 	win,
								0,
								0,
								stevensStringLib::utf32to8(stevensStringLib::utf8to32(borderPatterns["top"]).substr(0, width)),
								styleMap,
								{}	);
			}
			//If the border pattern is less than the window size, multiply the pattern by an amount equal to the width
			//divided by the border pattern length. To get the string that we'll print.
			else
			{
				std::string strToPrint = stevensStringLib::multiply(borderPatterns["top"], (width/stevensStringLib::charCount(borderPatterns["top"])) );
				//If the resulting border pattern string is not equal to the width, add/subtract
				//individual characters from the border pattern until we have a string equal to the window width.
				//Work in codepoints (not bytes) so multi-byte border glyphs aren't torn apart - see
				//stevensStringLib::utf8to32() doc comment.
				std::u32string u32strToPrint = stevensStringLib::utf8to32(strToPrint);
				int i = 0;
				while((int)u32strToPrint.length() != width)
				{
					if((int)u32strToPrint.length() > width)
					{
						u32strToPrint.pop_back();
					}
					else if((int)u32strToPrint.length() < width)
					{
						u32strToPrint += stevensStringLib::utf8to32(stevensStringLib::circularIndex(borderPatterns["top"], i));
						i++;
					}
				}
				//Now we can print the string!
				curses_wprint( 	win,
								0,
								0,
								stevensStringLib::utf32to8(u32strToPrint),
								styleMap,
								{}	);
			}
		}
		if(borderPatterns.contains("bottom"))
		{
			//If the top border pattern is greater than or equal to the window width, print all the characters of
			//the pattern equal up to the width size
			if(stevensStringLib::charCount(borderPatterns["bottom"]) >= width)
			{
				curses_wprint( 	win,
								height-1,
								0,
								stevensStringLib::utf32to8(stevensStringLib::utf8to32(borderPatterns["bottom"]).substr(0, width)),
								styleMap,
								{}	);
			}
			//If the border pattern is less than the window size, multiply the pattern by an amount equal to the width
			//divided by the border pattern length. To get the string that we'll print.
			else
			{
				std::string strToPrint = stevensStringLib::multiply(borderPatterns["bottom"], (width/stevensStringLib::charCount(borderPatterns["bottom"])) );
				//If the resulting border pattern string is not equal to the width, add/subtract
				//individual characters from the border pattern until we have a string equal to the window width.
				//Work in codepoints (not bytes) so multi-byte border glyphs aren't torn apart - see
				//stevensStringLib::utf8to32() doc comment.
				std::u32string u32strToPrint = stevensStringLib::utf8to32(strToPrint);
				int i = 0;
				while((int)u32strToPrint.length() != width)
				{
					if((int)u32strToPrint.length() > width)
					{
						u32strToPrint.pop_back();
					}
					else if((int)u32strToPrint.length() < width)
					{
						u32strToPrint += stevensStringLib::utf8to32(stevensStringLib::circularIndex(borderPatterns["bottom"], i));
						i++;
					}
				}
				//Now we can print the string!
				curses_wprint( 	win,
								height-1,
								0,
								stevensStringLib::utf32to8(u32strToPrint),
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

// NOTE: printFile() (plain std::cout ANSI-styling-era file printer) was
// removed — unreachable from anywhere in cultgame or elsewhere in this
// library. Use curses_wprintFile() instead.

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

// NOTE: recolorText() (Windows SetConsoleTextAttribute-based coloring, never
// called) and print()/print_overload2 (ANSI-styling wrapper around the now-
// removed PrintHelper::print()) were removed. Text coloring is handled
// uniformly on every platform via curses color pairs (see curses_wprint()
// and PrintHelper::curses_styleToken()).

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
