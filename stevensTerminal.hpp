/*
 * Stevens Terminal Library
 * July 2019
 * These are just some terminal functions I made to make my development easier.
 */
#ifndef STEVENSTERMINAL
#define STEVENSTERMINAL

// Include the core foundation
#include "Core.hpp"
#include "Styling.hpp"
#include "Input.hpp"


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

	/************* Methods *************/
	void horizontalStackedBarGraph(	std::vector<std::string> labels,
									std::vector< std::tuple<std::string,std::string> > colorCombos,
									std::vector<float> distribution,
									int width,
									bool textLabels,
									bool percentageLabels	)
	/*
	Creates a bar of a fixed length that is split into colors and percentages to show how much of each dimension makes up the whole, almost like a flat pie graph.
	Example:
	|===(red)==20%========|===(blue)======36%=================|=====(yellow)=========44%==========================|

	This graph will use ANSI color codes or system calls, so it must be platform specific. I believe we can dodge making this function itself platform specific by
	passing the coloring off to cgout();

	input:
		vector<string> labels - The names of each section of the graph. Note that all data in this function is parallel. 
										Meaning that index 0 of labels is attached to index 0 of colorCombos and distribution. And so on.
		vector< tuple<string,string> > colorCombos - textColor [0] and bgColor [1] respectively of a graph label
		vector<float> distribution - The graph value of a label
		int width - how wide the graph should be across the screen;

	output:
		prints a horizontalStackedBarGraph in the console
	*/
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
				// Print indent spaces first
				if(indentRemaining > 0)
				{
					print(" ", {{"textColor", textColor}, {"bgColor", bgColor}});
					indentRemaining--;
				}
				// Then print label characters
				else if(labelCharIndex < labelFormat.text.length())
				{
					std::string charAsString(1, labelFormat.text[labelCharIndex]);
					print(charAsString, {{"textColor", textColor}, {"bgColor", bgColor}, {"bold", "true"}});
					labelCharIndex++;
				}
				// Fill remaining space
				else
				{
					print(" ", {{"textColor", textColor}, {"bgColor", bgColor}});
				}

				charsRemaining--;
			}
		}

		std::cout << "\n";
	}
	

	void table(	std::vector< std::tuple<std::string,std::string,std::string> > xLabels,
				std::vector< std::tuple<std::string,std::string,std::string> > yLabels, std::vector< std::vector<float> > tableContent, std::string tableType)
	//Make a 2D table for elements of type T. If we want to have multiple types in a table,
	//stackoverflow suggests wrapping our types in classes: https://stackoverflow.com/questions/26208918/vector-that-can-have-3-different-data-types-c
	/*
	Input:
		xLabels - list of tuples: (xlabel text, text color, background color)
		yLabels - list of tuples: (ylabel text, text color, background color)
		tableContent - 2d vector of type T

	Output:
		a 2d table printed in the console of floats
	*/
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


	std::string localizedWrap(	std::string input,
								int availableSpace,
								bool menuLabel = false)
	/*
	** Takes a string and returns a string of characters that go beyond the available space given after printing
	** as much of the input as possible.
	*/
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
			else //If we have no more space, we print a hyphen and return the remainder of the string
			{
				std::cout << "-";
				return input.substr(i); //Return all the characters from ith index to the end of the string
			}
		}
		return "";
	}


	bool increaseOffset(std::vector<int> & menuItemOffset,
						int j)
	/*
	Helper function for vecticalMenus().
	Takes a vector all of the offsets for each column in the menu
	and after incrementing index j, returns true if the maximum offset was
	changed and false if the maximum offset was not affected.
	*/
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


	int verticalMenus(	std::vector< std::vector<std::string> > menus,
						std::vector<std::string> menuLabels,
						std::string borderPattern,
						bool showResponseNums,
						int maxMenuLength,
						int availableSpaceForColumns,
						bool showNone)
	/*
	** Lists multiple menus vertically, from left to right. Example:
		
		vecticalMenus({{snoo,kar,snookar},{eggs,dangson,defenestration,billybob,cats,dogs},{alice,balice}}, {foobar,spam,bob}, "|", true, 4)

		foobar	    |spam         |bob  
		1 - snoo     |4 - eggs     |9 - alice
		2 - kar      |5 - dangson  |10 - balice
		3 - snookar  |6 - defenes- |
							|    tration  |
							|7 - billybob |
							|8 - More...  |

		Returns the last selectnum in the menus. For the above example, it would return 10.
	**
	**
	*/
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
		int	currentResponseNum = 1; //The number of the response to print in the menu
		int remainingMenusToPrint = menus.size();
		int extraRowsForTextWrapAround = 0;
		int menuLength = maxMenuLength;
		std::vector< int > menuItemRowOffsets;
		std::vector< std::queue<std::string> > wrappedTextQueues;
		std::vector< std::queue<std::string> > queuedMenuItems;
		std::queue<std::string> emptyQueue;
		std::vector<int> itemsPrintedPerMenu; //A vector that keeps track of the number of items printed from each menu
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
							if(itemsPrintedPerMenu[menuColumnIndex] < menus[menuColumnIndex].size()) //If we have a menu item to print after the wraparound text, we queue it here
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
						else //If we have no text wrapping over from the line above and no items in the queue to print, then we print at the current row and column
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
		Structure of return tuple:
		0 - Last selectnum used in the vertical menu
		1 - A vector, where each index represents a vertical from left to right. The number stored at each index indicates the selectnum
			of the menu which can be selected to view more of the vertical menu in question.
		*/
		std::vector<int> viewMoreNums;
		std::tuple<int,std::vector<int> > returnTuple(selectNumByMenu.back(),viewMoreNums);
		return selectNumByMenu.back();
	}


	void promptMultipleResponses(	std::vector<std::string> responses,
									int columns, int rows,
									int startingResponseNum,
									bool showResponseNums	)
	/*
	Lists multiple responses to the player in a colum and row format. Useful if there is a lot of responses available to the player.
	input:
		responses - the possible responses that the player is able to select, the index of the response+1 is the select number associated wit the response
		columns - the number of columns of responses that will be created
		rows - the number of rows of responses that will be created
		startingResponseNum - the response num that the first response will be numbered with
		showResponseNums - show the dashes and numbers associated with each response when it is printed
	output:
		Responses printed from 0 to the final index. So a function call like this: promptMultipleResponses([a,b,c,d,e,f,g,h,i,j,k,l],4,3) looks like the following:
		1 - a     4 - d     7 - g     10 - j
		2 - b     5 - e     8 - h     11 - k
		3 - c     6 - f     9 - i     12 - l
	*/
	{
		//There's a seg fault in here somewhere :\

		std::string stringToPrint = ""; //This will be the final string we print after we decide where all of the responses will be ordered
		int selectNum = startingResponseNum-1;  

		if (responses.size() <=  rows) //If the given amount of rows is greater than or equal to the number of responses, we can just print out our responses vertically in a single column
		{
			for(size_t responseIndex = 0; responseIndex < responses.size(); responseIndex++)
			{
				selectNum++;
				if (showResponseNums)
				{
					print(std::to_string(selectNum) + " - " + strlib::cap1stChar(responses[responseIndex]) + "\n");
				}
				else
				{
					print(strlib::cap1stChar(responses[responseIndex]) + "\n");
				}
			}
		}
		else //The following is the main purpose of this function: We print out a series of columns and rows of responses that ascend in selectNum by vertically down columns, left to right
		{
			int totalResponseSlots = columns * rows; //total spaces in the grid to fill
			int responsesToCount = responses.size();
			std::vector< std::vector<std::string> > responseGrid; //responseGrid[column][response in row of index]
			std::vector<std::string> emptyVec;
			emptyVec.resize(rows); //mske the columns contain the number of rows we designate in the function call
			int columnIndex = 0;
			//cout << "Just before while loop\n";
			while(responsesToCount > 0) //Go through each column and fill a column vector with responses
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
							responseGrid[columnIndex][rowIndex] = std::to_string(selectNum) + " - " + strlib::cap1stChar(responses.front());
						}
						else
						{
							responseGrid[columnIndex][rowIndex] = strlib::cap1stChar(responses.front());
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


	/*
	* Outputs a vector of strings to terminal window with options for advanced formatting.
	* 
	* Parameters:
	*		vector<T> vec - The vector of objects we wish to output to the terminal.
	*		unordered_map<std::string,std::string> format - An unordered map containing various custom formatting options for printing the vector.
	*														Below are the possible keys that can be defined to format your vectors when printed:
	*			"columns" : (int as string) The number of columns to organize the printed objects in.
	*			"rows" - (int as string) The number of rows to organize the printed object in.
	*			"listType" - Adds special formatting when printing the vector. Possible values are:
	*				"numbered" - Appends a number in front of every object printed representing the index of the object printed (starting at 1)
	*			"prependString" - A string value to be prepended to the output before every object is printed.
	*			"appendString" - A string value to be appended to the output after every object is printed.
	*	Returns:
	*		Void, but outputs text to the terminal window:
	*		Vector elements printed from 0 to the final index. So a function call like this: printVector([a,b,c,d,e,f,g,h,i,j,k,l],{{"rows","3"},{"columns","4"}}) looks like the following:
	*		1 - a     4 - d     7 - g     10 - j
	*		2 - b     5 - e     8 - h     11 - k
	*		3 - c     6 - f     9 - i     12 - l
	*/
	template<typename T>
	void printVector(	std::vector<T> vec,
						std::unordered_map<std::string,std::string> format = {}	)
	{
		//flesh this out
		std::string stringToPrint = ""; //This will be the final string we print after we decide where all of the responses will be ordered

		//If no formatting is applied, then we just print all of the objects out on their own lines
		if(format.empty())
		{
			for(size_t elementIndex = 0; elementIndex < vec.size(); elementIndex++)
			{
				print(vec[elementIndex]);
			}
			return;
		}

		//We know that the user wants some specific type of formatting applied, we begin to set that up here.
		int columns = -1;
		int rows = -1;
		std::string listType = "";
		std::string prependString = "";
		std::string originalPrependString = "";
		std::string appendString = "";
		/*** Get all of the applicable information from the format map  ***/
		if(format.contains("columns"))
		{
			//Check to see how many columns the user wants to output their vector elements into
			if(strlib::isNumber(format["columns"]))
			{
				columns = stoi(format["columns"]);
			}
		}
		if(format.contains("rows"))
		{
			//Check to see how many rows the user wants to output their vector elements into
			if(strlib::isNumber(format["rows"]))
			{
				rows = stoi(format["columns"]);
			}
		}
		if(format.contains("listType"))
		{
			//Check to see what type of list formatting the user wants
			if(format["listType"] == "numbered")
			{
				listType = "numbered";
			}
		}
		if(format.contains("prependString"))
		{
			prependString = format["prependString"];
			originalPrependString = format["prependString"];
		}
		if(format.contains("appendString"))
		{
			appendString = format["appendString"];
		}

		//Begin printing the formatted vector. We do this storing all of the objects in a 2D vector.
		std::vector< std::vector<T> > elementGrid; //responseGrid[column][response in row of index]
		std::vector<T> emptyTVec;
		std::vector< std::vector<std::string> > prependTextGrid; 
		std::vector< std::vector<std::string> > appendTextGrid;
		std::vector< std::string > emptyStringVec;
		int numberLabel = 1;
		if(columns <= 0)
		{
			columns = 1; //We cannot have a columns value of LTE 0
		}
		if(rows > 0)
		{
			emptyTVec.resize(rows); //make the columns contain the number of rows we designate in the function call
			emptyStringVec.resize(rows);
		}
		else
		{
			//If the number of rows was not specified, we go with the size of the input vector we are printing
			emptyTVec.resize(vec.size());
			emptyStringVec.resize(vec.size());
			rows = vec.size();
		}
		int currentColumnIndex = 0;
		//cout << "Just before while loop\n";
		while(vec.size() > 0) //Go through each column and fill a column vector with responses
		{
			elementGrid.push_back(emptyTVec); //add a column here
			prependTextGrid.push_back(emptyStringVec);
			appendTextGrid.push_back(emptyStringVec);
			//cout << "vec.size() == " << vec.size() << "\n";
			//cout << "column added\n";
			for(int rowIndex = 0; rowIndex < rows; rowIndex++) //Add an element to each row of a column
			{
				if (vec.size() > 0)
				{
					if(listType == "numbered")
					{
						prependString = std::to_string(numberLabel) + " - " + prependString;
					}
					prependTextGrid[currentColumnIndex][rowIndex] = prependString;
					appendTextGrid[currentColumnIndex][rowIndex] = appendString;
					elementGrid[currentColumnIndex][rowIndex] = vec.front();
					//cout << "response added\n";
					//We've added the vector object to the grid we will print. We can erase it from its source vector.
					vec.erase(vec.begin());
					//Reset the prepend string
					prependString = originalPrependString;
					//Increase our number label
					numberLabel++;
				}
				else
				{
					//If we have no more items in our source vector to add to the grid we will print, then we break out of the loop here<<
					//cout << "we breakin\n";
					break;
				}
			}
			currentColumnIndex++;
		}
		//Now we have our responses in a grid/2D vector structure. We can print them out in lines now.
		for(int rowIndex = 0; rowIndex < rows; rowIndex++)
		{
			for(int columnIndex = 0; columnIndex < columns; columnIndex++) //Print each column after each other separated by tabs
			{
				//void(0);
				if (elementGrid[columnIndex][rowIndex].length() < MIN_CELL_WIDTH)
				{
					while(true)
					{
						elementGrid[columnIndex][rowIndex] += " ";
						if (elementGrid[columnIndex][rowIndex].length() == MIN_CELL_WIDTH)
						{
							break;
						}
					}
				}
				//cout << "printing a row!" << endl;
				print(prependTextGrid[columnIndex][rowIndex]);
				print(elementGrid[columnIndex][rowIndex]);
				print(appendTextGrid[columnIndex][rowIndex] + "\t");
			}
			std::cout << "\n"; //Once we finish a row we print a newline
		}
		std::cout << "\n"; //print newline when finished
		
		//cout << "finished printVector()!\n";

		return;
	}


	/**
	 * Given a vector, return a print-friendly std::string that lists all of the elements in the vector with
	 * advanced formatting options. Similar to printVector, but does not directly output to the cout stream and 
	 * just returns the string it would have output to cout instead. 
	 * 
	 * Originally created to work with the ncurses library.
	 * 
	 * Parameters:
	 * 	vector<T> vec - The vector of objects we wish to return a print-friendly string of.
	 *	unordered_map<std::string,std::string> format - An unordered map containing various custom formatting options for printing the vector.
	 *													Below are the possible keys that can be defined to format your vectors when printed:
	 *		"columns" : (int as string) The number of columns to organize the printed objects in.
	 *		"rows" - (int as string) The number of rows to organize the printed object in.
	 *		"listType" - Adds special formatting when printing the vector. Possible values are:
	 *		"numbered" - Appends a number in front of every object printed representing the index of the object printed (starting at 1)
	 *		"prependString" - A string value to be prepended to the output before every object is printed.
	 *		"appendString" - A string value to be appended to the output after every object is printed.
     *		
	 *
	 * @param style A map of styling options for how the vector should be printed. Valid styling options are:
	 * 				"textColor" - Controls the foreground text color of the whole returned string
	 * 			    "bgColor" - Controls the background text color of the whole returned string
	 * 				"@index{indexOfVectorElementHere}:{aboveStylingOptionsHere}" - Applies a certain style to only the string representing a specified element
	 * 
     * @param columnWidths A map where you can assign the horizontal width of columns in characters. Each key represents
	 					 the number of a column from left to right, starting with 0. Each value associated with a column
						 number represents how the column should be sized. The following values are valid.
						 Positive integers as strings "1", "2", "3", etc.: Assigns the width of the column to be the specified integer width
						 "auto" : Assigns the width of the column to be the width of the longest cell within it. By default, all columns
						 		  are set to "auto" unless specified otherwise.
	 *
	 * Returns:
	 * 	std::string - The print-friendly string that contains the elements of the vector vec.
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
		// 	std::unordered_map<std::string,std::string> wholeStringStyle = stevensMapLib::erasePairsWhereKeysStartWith(style,std::string("@"));
		// 	stringToPrint = stevensTerminal::addStyleToken(stringToPrint,wholeStringStyle);
		// 	//Apply styling to individual indices of the vector
		// 	//Check to see the specific styling directives given for each index
		// 	std::vector<std::string> separatedVectorOfStrings = stevensStringLib::separate(stringToPrint, "\n", false);
		// 	for(int i = 0; i < separatedVectorOfStrings.size(); i++)
		// 	{
		// 		std::unordered_map<std::string,std::string> indexStyle = stevensMapLib::getPairsWhereKeysStartWith(style,"@" + std::std::to_string(i));
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
	 * @brief Takes a maplike object as input and converts its contents to a printable string.
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


	#ifdef curses
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
		s_TerminalPrintHelper::curses_wprint(	win,
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
						std::unordered_map<std::string,std::string> format )
	{
		s_TerminalPrintHelper::curses_wprint(	win,
												yMove,
												xMove,
												printString,
												style,
												format,
												textStyling	);
	}
	#endif


	/**
	 * @brief Clears a line in a curses window
	 * @param win The window to clear the line in
	 * @param y The y coordinate of the line to clear
	 * @param settings Optional settings map. If "avoid borders" == "true", skips first and last column
	 */
	void curses_wClearLine(WINDOW* win, int y, std::unordered_map<std::string, std::string> settings = {})
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
	 * 					"column width" : -A string written as an integer describing the maximum number of character of a string that we'll
	 * 									 write to a particular column space
	 * 									- "use width of largest entry" makes the width of each column equal to the longest entry out of all rows for that column
	 * 					"column spacing string" : -A string that will be used to separate all columns
	 */
	//template<typename S>
	std::string formatTableAsString(	std::vector< std::vector<std::string> > table,
										const std::unordered_map<std::string, std::string> & style = {},
										std::unordered_map<std::string, std::string> format = {	{"column width", "use width of largest entry"},
																								{"column spacing string", "\t"}	}	)
	{
		std::string printString = "";

		//Determine how wide we should make the columns
		unsigned int columnWidth;
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
					return ""; // Return empty string on error
				}
				catch(const std::out_of_range& e)
				{
					std::cerr << "Error: Column width '" << widthStr << "' is out of range" << std::endl;
					return ""; // Return empty string on error
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
					tableByColumnsOfRows[i][n] = stevensTerminal::removeAllStyleTokenization(tableByColumnsOfRows[i][n]);
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
				std::vector<std::vector<std::string>> wrappedCells; // Each cell becomes vector of lines
				int maxLines = 1; // Track maximum lines needed for this row

				// Wrap each cell
				for(int col = 0; col < table[row].size(); col++)
				{
					std::string cellContent = stevensTerminal::removeAllStyleTokenization(table[row][col]);
					std::vector<std::string> lines;

					// Wrap the cell content at column width
					while(cellContent.length() > columnWidths[col])
					{
						lines.push_back(cellContent.substr(0, columnWidths[col]));
						cellContent = cellContent.substr(columnWidths[col]);
					}
					// Add remaining content
					if(!cellContent.empty())
					{
						lines.push_back(cellContent);
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

						// Pad or truncate to column width
						lineToPrint = stevensTerminal::resizeStyledString(lineToPrint, columnWidths[col]);
						printString += lineToPrint + format["column spacing string"];
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
					cell = stevensTerminal::resizeStyledString(cell, columnWidths[col]);
					printString += cell + format["column spacing string"];
				}
				printString += "\n"; //+ stevensStringLib::multiply(" ", yMove);
			}
		}

		return printString;
	}


	#ifdef curses
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
																						{"bgColor",		"black"}	}	)
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
								std::unordered_map<std::string,std::string> settingsMap )
	{
		//Get the width of the window
		int windowWidth = getmaxx(win);
		//Define where we'll start erasing our horizontal line in the window
		int x = 0;
		//If we are avoiding erasing the borders, change our parameters here
		if(settingsMap.contains("avoid borders"))
		{
			if(strlib::stringToBool(settingsMap["avoid borders"]))
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
					strlib::multiply(" ", windowWidth).c_str()	);
	}
	#endif


	void printFile(	std::string textFilePath,
					bool wrap = true	)
	//Copied from: https://www.delftstack.com/howto/cpp/read-file-into-string-cpp/
	{
		std::ifstream input_file(textFilePath);
		if (!input_file.is_open())
		{
			std::cerr << "Could not open the file - '" << textFilePath << "'" << std::endl;
			exit(EXIT_FAILURE);
		}
		std::string textFile = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
		print(textFile, {}, {{"wrap",strlib::boolToString(wrap)}});
	}


	#ifdef curses
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
							}	)
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
	#endif


	// inputWithinResponseRange() is now in Input.hpp


	#if defined(_WIN32)
	void recolorText(std::string textColor, std::string bgColor)
	/* Uses the windows API to color text.
	**
	**
	**
	*/
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
	
	
	void print(		std::string input,
						bool wrapped = true,
						int indent 	= 0,
						std::string textColor = "default",
						std::string bgColor = "default",
						bool bold = false,
						bool flash = false	)
	/*
	Handles any output in the terminal to make it look good and fit within a certain character range
	
	Input:
		string input: the string to print
		bool wrapped: indicates if we should print the text in such a way that we limit the amount of text
						possible on each line
	Output:
		Prints text with cout. It is wrapped around the screen by default, but the player can specify not to
		by calling cgout in the following way: cgout("foobar",0);
	
	***NB***: This function uses a global variable "textStyling" to control whether or not we add styles to our text
	*/
	{
		recolorText(textColor,bgColor);
	
		if(wrapped)
		{
			//Based on the screen size, we adjust how the text is wrapped.
			std::pair<int,int> screenSize = get_screen_size();
			switch(displayMode(screenSize))
			{
				case -1: //Weird terminal shapes
				{
					if(screenSize.first >= 80)
					{
						wrap(input, 79, std::cout, 0,true);
					}
					else
					{
						wrap(input, screenSize.first-1, std::cout, 0,true);
					}
					break;
				}
				case 0: //Teminal shapes smaller than 80x24
				{
					if (screenSize.first != 1)
					{
						wrap(input, screenSize.first-1, std::cout, 0,true);
					}
					else //We have a minimum width of 1
					{
						wrap(input, 1, std::cout, 0,true);
					}
					break;
				}
				case 1: //80x24
				{
					wrap(input, 79, std::cout, 0,true);
					//cout << "Low detail mode";
					break;
				}
				case 2: //120x40 or greater
				{
					wrap(input, 79, std:: cout, 0,true);
					//cout << "High detail mode";
					break;
				}
			}
		}
		else
		{
			cout << input;
		}
		recolorText("default","default"); //We call recolor text again here at the end to reset the text coloring back to normal
	}
	#endif
	
	
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
		Prints text to std out with cout.
	*/
	void print(	std::string input,
				std::unordered_map<std::string, std::string> style,
				std::unordered_map<std::string, std::string> format	)
	{
		term_printhelper::print(	input,
									style,
									format,
									textStyling,
									get_screen_size(),
									displayModes,
									currentDisplayMode	);
	}


	void menuLabel(std::string label)
	//Returns a gui menu label with the given string
	{
		std::cout << label << "\n\n";
	}


	bool responseVerify(std::string response, std::vector<std::string> desiredResponse)
	/* Used to check and see if at least one string from a vector (desired response) is included in another string (response)
		* The process is that we compare the response string to our strings of desired responses. If we find a single
		* match, then we return true. If we exhaust all comparisons and find no match, we return false.
		*
		* This function is mainly used for the riddler event, where we need to compare user input to the desired answers of our riddles.
		*/
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


	// displayMode_GTEminSize() and displayMode_LTEmaxSize() are now in Core.hpp
}

#endif