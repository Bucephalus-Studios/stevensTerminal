/*
 * Stevens Terminal Library
 * July 2019
 * These are just some terminal functions I made to make my development easier.
 */
#pragma once

// Include ncurses based on platform
#if defined(__linux__)
	#include <ncurses.h>
#elif defined(_WIN32)
	#include <curses.h>   // PDCurses/PDCursesMod: main header is curses.h, not pdcurses.h
#endif

// Include all stevensTerminal components
// Each file has its own namespace stevensTerminal {} wrapper
#include "subnamespaces/Core.hpp"
#include "subnamespaces/Styling.hpp"
#include "subnamespaces/Input.hpp"
#include "subnamespaces/ParticleFX/ParticleFX.hpp"
#include "subnamespaces/Bar.hpp"
#include "subnamespaces/Spinner.hpp"

namespace stevensTerminal
{

	/////// FUNCTION PROTOTYPES //////
	// Core functions are now in Core.hpp
	// Additional function prototypes for this module:
	//////////////////////////////////

	/************* Variables *************/
	// Core variables are now in Core.hpp
	// Additional variables for this module:

	/************* Types and Constants *************/

	// UI Layout Constants
	constexpr int DEFAULT_MENU_WIDTH = 18;  // Default width for menu items
	constexpr int MIN_CELL_WIDTH = 8;       // Minimum width for grid cells

	// Label style options for bar graphs
	enum class BarGraphLabelStyle {
		None,
		TextOnly,
		PercentageOnly,
		TextAndPercentage
	};

	// Result of label formatting calculation
	struct BarGraphLabelFormat {
		std::string text;
		BarGraphLabelStyle style;
		int width;
		int indent;
	};

	/************* Helper Functions *************/

	/**
	 * @brief Calculates the optimal label format based on available space
	 * @param labelText The text label to display
	 * @param percentage The percentage value (0.0 to 1.0)
	 * @param availableWidth How much space is available in the bar
	 * @param showText Whether text labels are enabled
	 * @param showPercentage Whether percentage labels are enabled
	 * @return Formatted label with calculated width and indent
	 */
	inline BarGraphLabelFormat calculateBarGraphLabel(
		const std::string& labelText,
		float percentage,
		int availableWidth,
		bool showText,
		bool showPercentage);

	/************* Methods *************/
	void horizontalStackedBarGraph(	std::vector<std::string> labels,
									std::vector< std::tuple<std::string,std::string> > colorCombos,
									std::vector<float> distribution,
									int width,
									bool textLabels,
									bool percentageLabels	);
	

	// NOTE: table(), localizedWrap(), increaseOffset(), verticalMenus(), and
	// promptMultipleResponses() were removed — unreachable dead code from
	// before curses_wprint()/WINDOW* existed. See stevensTerminal.cpp.

	// NOTE: printVector() (plain std::cout ANSI-styling-era vector printer) was
	// removed — unreachable dead code; cultgame uses curses_mvw_printVector()
	// instead. See stevensTerminal.cpp for the string-returning variant kept below.


	/**
	 * Given a vector, return a print-friendly std::string that lists all of the elements in the vector with
	 * advanced formatting options. Similar to printVector, but does not directly output to the cout stream and 
	 * just returns the std::string it would have output to cout instead. 
	 * 
	 * Originally created to work with the ncurses library.
	 * 
	 * Parameters:
	 * 	vector<T> vec - The vector of objects we wish to return a print-friendly std::string of.
	 *	unordered_map<std::string,std::string> format - An unordered map containing various custom formatting options for printing the vector.
	 *													Below are the possible keys that can be defined to format your vectors when printed:
	 *		"columns" : (int as std::string) The number of columns to organize the printed objects in.
	 *		"rows" - (int as std::string) The number of rows to organize the printed object in.
	 *		"listType" - Adds special formatting when printing the vector. Possible values are:
	 *		"numbered" - Appends a number in front of every object printed representing the index of the object printed (starting at 1)
	 *		"prependString" - A std::string value to be prepended to the output before every object is printed.
	 *		"appendString" - A std::string value to be appended to the output after every object is printed.
     *		
	 *
	 * @param style A map of styling options for how the vector should be printed. Valid styling options are:
	 * 				"textColor" - Controls the foreground text color of the whole returned std::string
	 * 			    "bgColor" - Controls the background text color of the whole returned std::string
	 * 				"@index{indexOfVectorElementHere}:{aboveStylingOptionsHere}" - Applies a certain style to only the std::string representing a specified element
	 * 
     * @param columnWidths A map where you can assign the horizontal width of columns in characters. Each key represents
	 					 the number of a column from left to right, starting with 0. Each value associated with a column
						 number represents how the column should be sized. The following values are valid.
						 Positive integers as strings "1", "2", "3", etc.: Assigns the width of the column to be the specified integer width
						 "auto" : Assigns the width of the column to be the width of the longest cell within it. By default, all columns
						 		  are set to "auto" unless specified otherwise.
	 *
	 * Returns:
	 * 	std::string - The print-friendly std::string that contains the elements of the vector vec.
	*/
	template<typename T>
	std::string printVector_str(	std::vector<T> vec,
									const std::unordered_map<std::string,std::string> & format = {},
									const std::unordered_map<std::string,std::string> & style = {},
									const std::unordered_map<int, std::string> columnWidths = {} )
	{
		//This will be the final string we print after we decide where all of the responses will be ordered
		std::string stringToPrint = "";

		//If no formatting is applied, then we just print all of the objects out sequentially in a single column
		if(format.empty())
		{
			for(size_t elementIndex = 0; elementIndex < vec.size(); elementIndex++)
			{
				stringToPrint += vec[elementIndex] + "\n";
			}
			return stringToPrint;
		}

		//We know that the user wants some specific type of formatting applied, we begin to set that up here.
		int columns = -1;
		int rows = -1;
		std::string sequence = "column first";
		bool allowOverflow = false;
		std::string listType = "";
		std::string prependString = "";
		std::string originalPrependString = "";
		std::string appendString = "";
		/*** Get all of the applicable information from the format map  ***/
		if(format.contains("columns"))
		{
			//Check to see how many columns the user wants to output their vector elements into
			if( stevensStringLib::isNumber(format.at("columns")) )
			{
				columns = std::stoi(format.at("columns"));
			}
		}
		if(format.contains("rows"))
		{
			//Check to see how many rows the user wants to output their vector elements into
			if(stevensStringLib::isNumber(format.at("rows")))
			{
				rows = std::stoi(format.at("rows"));
			}
		}
		//In what order should we add the items from a vector into a string
		if(format.contains("sequence"))
		{
			if(format.at("sequence") == "column first")
			{
				sequence = "column first";
			}
			else if(format.at("sequence") == "row first")
			{
				sequence = "row first"; //TODO
			}
		}
		//Are we allowing overflow?
		if(format.contains("allowOverflow"))
		{
			if(format.at("allowOverflow") == "true")
			{
				//If yes, then if the input vector exceeds the column and row dimensions we pass in,
				//we still output the excess elements in the returned string by adding additional columns for
				//sequence == "column first" or additional rows for sequence == "row first"
				allowOverflow = true;
			}
			else
			{
				//If no (which is default), any elements in vec at indices greater than the product of the row and columns
				//are not added to the output string
				allowOverflow = false;
			}
		}
		if(format.contains("listType"))
		{
			//Check to see what type of list formatting the user wants
			if(format.at("listType") == "numbered")
			{
				listType = "numbered";
			}
		}
		if(format.contains("prependString"))
		{
			prependString = format.at("prependString");
			originalPrependString = format.at("prependString");
		}
		if(format.contains("appendString"))
		{
			appendString = format.at("appendString");
		}
		//Indicates what the default width of columns should be. By default, there is no set default width.
		std::string defaultColumnWidth = "none";
		if( format.contains("defaultColumnWidth") )
		{
			//"auto" makes the default column width equal to the length of the longest cell 
			if( format.at("defaultColumnWidth") == "auto" )
			{
				defaultColumnWidth = "auto";
			}
		}
		//A setting for the string we use to separate columns. \t by default.
		std::string horizontalSeparator = "\t";
		if(format.contains("horizontal separator"))
		{
			horizontalSeparator = format.at("horizontal separator");
		}

		/*** FORMAT THE VECTOR ***/
		std::vector< std::vector<T> > elementGrid; //The 2D vector we will fill up to print out. elementGrid[column][row]
		std::vector<T> emptyTVec; //An empty row we will push to element grid as a row to fill up
		std::vector< std::vector<std::string> > prependTextGrid; //A 2D vector containing strings we will prepend to the corresponding elements in elementGrid when printing
		std::vector< std::vector<std::string> > appendTextGrid; //A 2D vector containing strings we will append to the corresponding elements in elementGrid when printing
		std::vector< std::string > emptyStringVec;
		std::unordered_map<int, size_t> greatestCellSizePerColumn; //Map that tracks the longest cell per each column (ignoring style token characters)
		int numberLabel = 1;
		//Assign default column and row sizes if unspecified
		if(columns <= 0)
		{
			columns = 1; //We cannot have a columns value of LTE 0
		}
		//If the number of rows was not specified, we go with the size of the input vector we are printing
		if(rows <= 0)
		{
			rows = vec.size();
		}
		
		//Construct the elementGrid by iterating through vec and pushing its elements back into elementGrid
		//Begin with choosing a starting index
		if(sequence == "column first")
		{
			//Declare the starting index
			int startingIndex = 0;
			int workingIndex = startingIndex;

			//Construct and insert each row into elementGrid
			for(int rowIndex = 0; rowIndex < rows; rowIndex++ )
			{
				//Initialize vectors that will make up the row
				std::vector<T> workingElementRow = {};
				std::vector<std::string> workingPrependStringRow = {};
				std::vector<std::string> workingAppendStringRow = {};

				//Put elements from each column into the row if possible
				while(true)
				{
					//Make sure the working index is within the bounds of vec
					if(vec.size() <= workingIndex)
					{
						//Row is now done being filled up, add the row to the element grid
						elementGrid.push_back(workingElementRow);
						prependTextGrid.push_back(workingPrependStringRow);
						appendTextGrid.push_back(workingAppendStringRow);
						//Increment the starting index to restart the process for the next row
						startingIndex++;
						workingIndex = startingIndex;
						break;
					}

					//Add the element at the working index to the row
					workingElementRow.push_back(vec.at(workingIndex));

					//Add the correct prepend and append strings
					if(listType == "numbered")
					{
						prependString = std::to_string(workingIndex + 1) + " - " + prependString;
					}
					workingPrependStringRow.push_back(prependString);
					workingAppendStringRow.push_back(appendString);

					if(defaultColumnWidth == "auto")
					{
						//Get the length of the cell (less the style token characters)
						int cellLength = stevensTerminal::removeAllStyleTokenization(prependString + vec.at(workingIndex) + appendString).length();
						//Do we have a recorded greatest cell size for this column?
						if( greatestCellSizePerColumn.contains(workingElementRow.size() - 1) )
						{
							//Yes, so we compare the length of this cell to the recorded greatest cell size
							if( greatestCellSizePerColumn.at(workingElementRow.size() - 1) < cellLength )
							{
								//If the cell length is greater than our recorded greatest, it becomes the new recorded greatest
								greatestCellSizePerColumn[workingElementRow.size() - 1] = cellLength;
							}
						}
						else
						{
							//No, so we set the greatest length for this column to be this cell's length
							greatestCellSizePerColumn[workingElementRow.size() - 1] = cellLength;
						}
					}

					//Step over all the rows in vec to see if we have a next element to place in the row of elementGrid
					workingIndex += rows;
				}
			}

			// //Add an element to each row of the new column
			// for(int currRow = 0; currRow < rows; currRow++)
			// {
			// 	if(vec.size() > 0)
			// 	{
			// 		if(listType == "numbered")
			// 		{
			// 			prependString = std::to_string(numberLabel) + " - " + prependString;
			// 		}
			// 		prependTextGrid[currCol][currRow] = prependString;
			// 		appendTextGrid[currCol][currRow] = appendString;
			// 		elementGrid[currCol][currRow] = vec.front();
			// 		//cout << "response added\n";
			// 		//We've added the vector object to the grid we will print. We can erase it from its source vector.
			// 		vec.erase(vec.begin());
			// 		//Reset the prepend string
			// 		prependString = originalPrependString;
			// 		//Increase our number label
			// 		numberLabel++;
			// 	}
			// 	else
			// 	{
			// 		//If we have no more items in our source vector to add to the grid we will print, then we break out of the loop here<<
			// 		//cout << "we breakin\n";
			// 		break;
			// 	}
			// }
			// // if(format.contains("debug"))
			// // {
			// // 	std::cout << "added a column with " << rows << " elements" << std::endl;
			// // 	getchar();
			// // }
			// currCol++;
		}
		// if(format.contains("debug"))
		// {
			// std::cout << "columns of elementgrid: " <<  elementGrid.size() << std::endl;
			// std::cout << "elements left in vec: " << vec.size() << std::endl;
			// getchar();
		// }

		/*** WRITE THE FORMATTED VECTOR TO A STRING ***/
		// std::cout << "elementGrid dimensions: " << elementGrid.size();
		// getch();
		//Now we have our responses in a grid/2D vector structure. We can them write them to the stringToPrint now.
		for(int rowIndex = 0; rowIndex < rows; rowIndex++)
		{
			//If we don't have elements to print at this row, stop finish creating the stringToPrint
			if( rowIndex >= elementGrid.size())
			{
				break;
			}
			//If we're allowing overflow, make it so we can print all of the elements in the row
			int columnsToPrint;
			if(allowOverflow)
			{
				columnsToPrint = elementGrid.at(rowIndex).size();
			}
			else
			{
				columnsToPrint = columns;
			}

			for(int columnIndex = 0; columnIndex < columnsToPrint; columnIndex++) //Print each column after each other separated by tabs
			{
				//If we don't have an item to print in the current column, skip it
				if( columnIndex >= elementGrid.at(rowIndex).size() )
				{
					continue;
				}

				//Get what should be printed in the cell
				std::string cell =	prependTextGrid.at(rowIndex).at(columnIndex) +
									elementGrid.at(rowIndex).at(columnIndex) +
									appendTextGrid.at(rowIndex).at(columnIndex);

				//Before we add the cell to the print string, check to see if we have a specific width for this column
				//or if we are using a default column width
				if( (columnWidths.contains(columnIndex)) ||
					(defaultColumnWidth != "none") )
				{
					//Get the width of the column that we can print to
					size_t columnWidth;
					if(columnWidths.contains(columnIndex))
					{
						//If we specified an integer column size for this column, use it
						if( stevensStringLib::isInteger( columnWidths.at(columnIndex)) )
						{
							columnWidth = std::stoi( columnWidths.at(columnIndex) );
						}
						//If we didn't use the "auto" setting
						else
						{
							columnWidth = greatestCellSizePerColumn.at(columnIndex);
						}
					}
					//If we didn't supply the width of the column, use the "auto" column sizing
					else
					{
						columnWidth = greatestCellSizePerColumn.at(columnIndex);
					}
					//Resize the cell based on our given column size
					cell = stevensTerminal::resizeStyledString(cell, columnWidth);
				}

				cell += horizontalSeparator;

				stringToPrint += cell;
			}
			stringToPrint += "\n";
		}

		// //Lastly, we have our string to print. Let's see if we need to apply any styles
		// if(!style.empty())
		// {
		// 	//Add a styling token to the stringToPrint
		// 	std::unordered_map<string,std::string> wholeStringStyle = stevensMapLib::erasePairsWhereKeysStartWith(style,std::string("@"));
		// 	stringToPrint = stevensTerminal::addStyleToken(stringToPrint,wholeStringStyle);
		// 	//Apply styling to individual indices of the vector
		// 	//Check to see the specific styling directives given for each index
		// 	std::vector<string> separatedVectorOfStrings = stevensStringLib::separate(stringToPrint, "\n", false);
		// 	for(int i = 0; i < separatedVectorOfStrings.size(); i++)
		// 	{
		// 		std::unordered_map<string,std::string> indexStyle = stevensMapLib::getPairsWhereKeysStartWith(style,"@" + std::std::to_string(i));
		// 		if(!indexStyle.empty())
		// 		{
		// 			std::string indexingString = "@" + std::std::to_string(i) + ":";
		// 			indexStyle = stevensMapLib::eraseStringFromKeys(indexStyle, indexingString);
		// 			separatedVectorOfStrings[i] = stevensTerminal::addStyleToken(	separatedVectorOfStrings.at(i),
		// 																			indexStyle	);
		// 		}
		// 	}
		// 	stringToPrint = stevensStringLib::join(separatedVectorOfStrings, "\n", false);
		// }
		// std::cout << "rows: " << rows << ", columns: " << columns << std::endl;
		// getch();

		return stringToPrint;
	}

	/**
	 * @brief Takes a maplike object as input and converts its contents to a printable std::string.
	 */
	template <template <typename, typename, typename...> class M, typename K, typename V, typename... Args>
	std::string printMap_str(	const M<K,V> & map	)
	{
		std::string stringToPrint;
		
		stringToPrint += "{\n";
		//Turn each key-value pair into a string
		for(const auto & [key, value] : map)
		{
			stringToPrint += "\t{";
			stringToPrint += key + ",";
			//Convert the value to a string
			std::string valueAsStr;
			if( std::is_same<V, float>::value )
			{
				valueAsStr = std::to_string(value);
			}
			stringToPrint += valueAsStr += "}\n";
		}
		stringToPrint += "}";

		return stringToPrint;
	}

	/**
	 * Prints a vector to a window and location using the n/pd curses library.
	 *
	 * Parameters:
	 * 	vector<T> vec - The vector we'd like to print a window of our curses app.
	 * 	unordered_map<std::string,std::string> style -
	 * 	unordered_map<std::string,std::string> format - The specfic formatting instructions that we're supplying to printVector_str.
	 * 													Valid key-value pairs:
	 * 													"printOverBorders" - "true"/"false" - Contains all of the text we print to be within
	 * 																						  a window's border of size 1
	 * 	WINDOW * win - The n/pd curses library window object that we're printing to.
	 * 	int yMove - The y coordinate in the window we're moving towards to start printing the vector
	 * 	int xMove - The x coordinate in the window we're moving towards to start printing the vector
	 *
	*/
	template<typename T>
	void curses_mvw_printVector(	std::vector<T> vec,
									WINDOW * win,
									std::unordered_map<std::string,std::string> style = {},
									std::unordered_map<std::string,std::string> format = {},
									int yMove = 0,
									int xMove = 0	)
	{
		//Get the vector we would like to print
		std::string printString = stevensTerminal::printVector_str(vec, format);

		//For each line (using getline), print the line (with option for wrapping) to the window
		PrintHelper::curses_wprint(	win,
												yMove,
												xMove,
												printString,
												style,
												format,
												textStyling	);
	}

	/**
	 * @brief Prints to a curses window
	 */
	void curses_wprint( WINDOW * win,
						int yMove,
						int xMove,
						std::string printString,
						std::unordered_map<std::string,std::string> style,
						std::unordered_map<std::string,std::string> format );

	/**
	 * @brief Clears a line in a curses window
	 * @param win The window to clear the line in
	 * @param y The y coordinate of the line to clear
	 * @param settings Optional settings map. If "avoid borders" == "true", skips first and last column
	 */
	void curses_wClearLine(WINDOW* win, int y, std::unordered_map<std::string, std::string> settings = {});

	/**
	 * @brief Prints a 2D vector of stringlike values as a table to the curses window.
	 * 
	 * Example:
	 * This code...
	 * curses_mvw_printTable(	{	{"Demographic", "Population", "Happiness", "Income"},
	 * 								{"Herdazian Peasants", "1000", "50/100", "30 gp"},
	 * 								{"Herdazian Noblemen", "143", "70/100", "100 gp"},
	 * 								{"Canuck Tribesman", "400", "20/100", "0 gp"}	},
	 * 							),
	 * 							0,
	 * 							0,
	 * 							{},
	 * 							{}	);
	 * 
	 * Prints...
	 * Demographic			Population	Happiness	Income				
     * Herdazian Peasants	1000		50/100		30 gp
     * Herdazian Noblemen	143			70/100		100 gp
     * Canuck Tribesmen		400			20/100		0 gp
	 * 
	 * @param table A 2D vector that we are printing to the curses window win.
	 * @param style A map of strings that can contain directives regarding how you would like to style the whole table
	 * @param format A map of strings that can contain directives regarding how you would like to format the printing of the table.
	 * 				 Valid key-value pairs aare:
	 * 					"column width" : -A std::string written as an integer describing the maximum number of character of a std::string that we'll
	 * 									 write to a particular column space
	 * 									- "use width of largest entry" makes the width of each column equal to the longest entry out of all rows for that column
	 * 					"column spacing std::string" : -A std::string that will be used to separate all columns
	 */
	//template<typename S>
	std::string formatTableAsString(	std::vector< std::vector<std::string> > table,
										const std::unordered_map<std::string, std::string> & style = {},
										std::unordered_map<std::string, std::string> format = {	{"column width", "use width of largest entry"},
																								{"column spacing std::string", "\t"}	}	);

	/**
	 * Given a curses window, print border patterns to it from a map of given border patterns.
	 *
	 * Parameters:
	 * 	WINDOW * win - The window we are printing a border to
	 * 	unordered_map<std::string,std::string> borderPatterns - The location and patterns which to repeat on the borders of the window.
	 * 															Can contain the following key-value pairs:
	 * 															"left" 	- Pattern to repeat on the left side of the window
	 * 															"right" - Pattern to repeat on the right side of the window
	 * 															"top"	- Pattern to repeat on the top side of the window
	 * 															"bottom"- Pattern to repeat on the bottom side of the window
	 * 															"top-left corner" - Pattern to print on the top left corner of the window
	 * 															"top-right corner" - Pattern to print on the top right corner of the window
	 * 															"bottom-left corner" - Pattern to print on the bottom left corner of the window
	 * 															"bottom-right corner" - Pattern to print on the bottom right corner of the window
	 *
	 * Returns:
	 * 	void, but prints fancy borders to a window
	 */
	void curses_wborder(	WINDOW * win,
							std::unordered_map<std::string,std::string> borderPatterns = {},
							std::unordered_map<std::string,std::string> styleMap = {	{"textColor",	"white"},
																						{"bgColor",		"black"}	}	);

	/**
	 * @brief Erases a single horizontal line of text on a curses window at a given Y coordinate.
	 *
	 * @param win The window where we are erasing a horizontal line of text
	 * @param y The coordinate of the window where we are erasing a horizontal line.
	 * @param settingsMap A map of settings we can use to further specialize this function call. Possible keys are:
	 * 						"avoid borders" : Avoids erasing the horizontal borders of the line if set to "true"
	 */
	void curses_wEraseHLine(	WINDOW * & win,
							 	int y,
								std::unordered_map<std::string,std::string> settingsMap );

	// NOTE: printFile() was removed — unreachable dead code. Use
	// curses_wprintFile() instead. See stevensTerminal.cpp.

	/**
	 *	Given a curses window and a path for file, print the content of the file to the curses window.
		*
		*	Parameters:
		*		WINDOW * win - The curses window we desire to print the file content to.
		*		std::string filePath - The filepath for the file we wish to print the content of.
		*		unordered_map<std::string,std::string> style -  Style options for printing the file
															The following key value pairs are valid:
															{

															}
		*		unordered_map<std::string,std::string> format - Formatting options for printing the file to the window.
															The following key value pairs are valid:
															{
																{"wrap" : "true"/"false"}
																	Wraps the text by whitespace around the window if true
																{"avoid borders" : "true"/"false"}
																	Avoid printing over the 1 character thick borders on the left
																	and right sides of the window if true
															}
		*
		* 	Returns:
		* 		void, but prints to the console using curses
		*/
	void curses_wprintFile(	WINDOW * win,
							std::string filePath,
							std::unordered_map<std::string,std::string> style = {},
							std::unordered_map<std::string,std::string> format = {
								{"wrap","true"},
								{"avoid borders","true"}
							}	);

	// inputWithinResponseRange() is now in Input.hpp

	// NOTE: recolorText() and print()/print_overload2 were removed — see
	// stevensTerminal.cpp for details. Use curses_wprint() for colored output.

	void menuLabel(std::string label);

	bool responseVerify(std::string response, std::vector<std::string> desiredResponse);

	// displayMode_GTEminSize() and displayMode_LTEmaxSize() are now in Core.hpp
} // namespace stevensTerminal
