/**
 * TerminalDisplayMode.h
 *
 * Defines the TerminalDisplayMode class; an object used to contain data concerning
 * the display modes of the stevensTerminal library.
 *
 * Display modes can be defined by users to customize how exactly they want to control the
 * formatting of text when the user changes their terminal's size.
 */

#ifndef STEVENS_TERMINAL_DISPLAYMODE_H
#define STEVENS_TERMINAL_DISPLAYMODE_H

namespace stevensTerminal {

class TerminalDisplayMode
{
	public:

		// Member variables
		std::pair<int,int> minSize; // The inclusive lower bounding for the X,Y dimensions of the terminal in characters for this display mode
		std::pair<int,int> maxSize; // The inclusive upper bounding for the X,Y dimensions of the terminal in characters for this display mode
		int verticalMenuWidth; // The width of a single vertical menu when this display mode is in use.


		//Default constructor
		TerminalDisplayMode()
		{
			minSize = {0,0};
			maxSize = {80,24};
			verticalMenuWidth = 18;
		}


		//Parametric constructor
		TerminalDisplayMode(	std::pair<int,int> minSizeParam,
								std::pair<int,int> maxSizeParam,
								int verticalMenuWidthParam	)
		{
			minSize = minSizeParam;
			maxSize = maxSizeParam;
			verticalMenuWidth = verticalMenuWidthParam;
		}


	private:

};

} // namespace stevensTerminal

#endif // STEVENS_TERMINAL_DISPLAYMODE_H
