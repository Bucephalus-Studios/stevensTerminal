/**
 * @file Input.hpp
 * @brief Input handling and validation functions for stevensTerminal
 * 
 * This header provides functions for user input, input validation,
 * and input stream management.
 * 
 * Part of the Stevens Terminal Library
 * Originally created July 2019
 */

#ifndef STEVENS_TERMINAL_INPUT_HPP
#define STEVENS_TERMINAL_INPUT_HPP

namespace stevensTerminal {

    /**
     * @brief Clear the input stream buffer
     * 
     * Clears the cin buffer to prevent input injections. 
     * Not used with getline operations.
     */
    void clearInput() {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    /**
     * @brief Get string input from user with validation
     * 
     * User input function used throughout the application for accepting strings as input.
     * Returns the strings that the player inputs via getline, refuses strings of complete 
     * whitespace or empty strings.
     * 
     * @return std::string The validated user input string
     */
    std::string input() {
        std::string userInput; // the input from the player
        int whitespaceCount;
        bool hasValidInput = false; // bool declaring whether or not the correct input has been received
        print("> ");

        while (hasValidInput == false) {
            whitespaceCount = 0;
            std::getline(std::cin, userInput); // use getline to take in user input

            // Check to see if the string is just whitespace, if so, we prompt for input again
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

            if (userInput == "") { // we also check to see if the string is empty. If so, we prompt again for input.
                hasValidInput = false;
            } else { // if the string is not empty and the string is not entirely whitespace, we accept the input
                hasValidInput = true;
            }
        }
        return userInput;
    }

    /**
     * @brief Wait for user to press enter
     * 
     * Accepts any input, and continues the program upon pressing the enter key.
     */
    void pressEnter() {
        std::cin;
        clearInput();
    }

    /**
     * @brief Get integer input from user within a specified range
     * 
     * User input function used throughout the application for accepting integer input.
     * Keeps a loop going until the desired type of input is accepted.
     * 
     * @param responseRange The total number of responses inclusive that will exist for inputting the integer 
     *                      (passing 2 in means that there will be 2 choices: 0, 1, 2)
     * @return int The validated integer input from the user
     */
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

            if (strlib::isNumber(userInput)) {
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

    /**
     * @brief Check if input string is a valid number within response range
     * 
     * Given a string input, determine if it is a number that exists within
     * the size range of possible responses. If the string input is determined
     * to not be a number, then we return false.
     * 
     * @param input A string value that potentially exists as a number in the response range or not
     * @param responseRange The number of total responses. The range is effectively from 1 to the responseRange passed in.
     * @return bool True if the input string is a number and exists within a range of 1 to the response range inclusive.
     *              False in all other situations.
     */
    bool inputWithinResponseRange(std::string input, int responseRange) {
        if(strlib::isNumber(input)) {
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

} // namespace stevensTerminal

#endif // STEVENS_TERMINAL_INPUT_HPP