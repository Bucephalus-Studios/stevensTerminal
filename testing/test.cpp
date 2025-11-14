/**
 * @brief test.cpp
 * 
 * This file contains all of the tests for the stevensTerminal library.
 */

#include "../stevensTerminal.hpp"
#include <iostream>
#include <fstream>
#include <gtest/gtest.h>


/***** TESTS *****/
/*** s_TerminalPrintTokenHelper::findToken() ***/
TEST(findToken, startIndexIsOutOfBounds)
{
    //Arrange
    std::string str = "{Duck, duck, goose!}$[bgColor=bright-yellow]";
    size_t actualIndexOfToken = 0;
    //Act
    size_t foundIndexOfToken = s_TerminalPrintTokenHelper::findToken(str, 9001);
    //Assert
    ASSERT_EQ(std::string::npos, foundIndexOfToken);
}


/*** s_TerminalPrintTokenHelper::parseToken() ***/
// TEST(parseToken, emptyString)
// {
//     //Arrange
//     std::string str = "";
//     //Act
//     s_TerminalPrintToken token = s_TerminalPrintTokenHelper::parseRawToken(str);
//     //Assert
//
//}


/*** Nested Token Depth Testing ***/
TEST(NestedTokenDepth, twoLevelsDeep)
{
    // Test 2 levels: parent -> child
    std::string input = "{Outer {inner}$[textColor=red]}$[bgColor=blue]";
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(input);
    
    EXPECT_GT(tokens.size(), 0) << "Should tokenize 2-level nested structure";
    
    // Debug output
    std::cout << "\n=== 2-Level Test ===" << std::endl;
    std::cout << "Input: " << input << std::endl;
    std::cout << "Tokens found: " << tokens.size() << std::endl;
    for(size_t i = 0; i < tokens.size(); i++) {
        std::cout << "  Token " << i << ": '" << tokens[i].content << "' [bg=" << tokens[i].bgColor << ", text=" << tokens[i].textColor << "]" << std::endl;
    }
}

TEST(NestedTokenDepth, threeLevelsDeep)
{
    // Test 3 levels: parent -> child -> grandchild
    std::string input = "{Level1 {Level2 {Level3}$[textColor=green]}$[textColor=red]}$[bgColor=blue]";
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(input);
    
    EXPECT_GT(tokens.size(), 0) << "Should tokenize 3-level nested structure";
    
    // Debug output
    std::cout << "\n=== 3-Level Test ===" << std::endl;
    std::cout << "Input: " << input << std::endl;
    std::cout << "Tokens found: " << tokens.size() << std::endl;
    for(size_t i = 0; i < tokens.size(); i++) {
        std::cout << "  Token " << i << ": '" << tokens[i].content << "' [bg=" << tokens[i].bgColor << ", text=" << tokens[i].textColor << "]" << std::endl;
    }
    
    // Verify inheritance - Level3 should inherit bgColor=blue from parent
    bool foundLevel3 = false;
    for(const auto& token : tokens) {
        if(token.content == "Level3") {
            EXPECT_STREQ(token.textColor.c_str(), "green") << "Level3 should keep its own textColor";
            EXPECT_STREQ(token.bgColor.c_str(), "blue") << "Level3 should inherit bgColor from root parent";
            foundLevel3 = true;
        }
    }
    EXPECT_TRUE(foundLevel3) << "Should find Level3 token";
}

TEST(NestedTokenDepth, fiveLevelsDeep)
{
    // Test 5 levels deep
    std::string input = "{L1 {L2 {L3 {L4 {L5}$[textColor=white]}$[textColor=yellow]}$[textColor=green]}$[textColor=red]}$[bgColor=blue]";
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(input);
    
    EXPECT_GT(tokens.size(), 0) << "Should tokenize 5-level nested structure";
    
    // Debug output
    std::cout << "\n=== 5-Level Test ===" << std::endl;
    std::cout << "Input: " << input << std::endl;
    std::cout << "Tokens found: " << tokens.size() << std::endl;
    for(size_t i = 0; i < tokens.size(); i++) {
        std::cout << "  Token " << i << ": '" << tokens[i].content << "' [bg=" << tokens[i].bgColor << ", text=" << tokens[i].textColor << "]" << std::endl;
    }
    
    // Verify deepest level inherits properly
    bool foundL5 = false;
    for(const auto& token : tokens) {
        if(token.content == "L5") {
            EXPECT_STREQ(token.textColor.c_str(), "white") << "L5 should keep its own textColor";
            EXPECT_STREQ(token.bgColor.c_str(), "blue") << "L5 should inherit bgColor from root";
            foundL5 = true;
        }
    }
    EXPECT_TRUE(foundL5) << "Should find L5 token at deepest level";
}

TEST(NestedTokenDepth, tenLevelsDeep)
{
    // Test 10 levels deep - stress test
    std::string input = "{L1 {L2 {L3 {L4 {L5 {L6 {L7 {L8 {L9 {L10}$[textColor=white]}$[textColor=cyan]}$[textColor=magenta]}$[textColor=yellow]}$[textColor=green]}$[textColor=red]}$[textColor=blue]}$[textColor=black]}$[textColor=bright-white]}$[bgColor=blue]";
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(input);
    
    EXPECT_GT(tokens.size(), 0) << "Should handle 10-level deep nesting without crashing";
    
    // Debug output
    std::cout << "\n=== 10-Level Test ===" << std::endl;
    std::cout << "Input: " << input << std::endl;
    std::cout << "Tokens found: " << tokens.size() << std::endl;
    for(size_t i = 0; i < tokens.size(); i++) {
        std::cout << "  Token " << i << ": '" << tokens[i].content << "' [bg=" << tokens[i].bgColor << ", text=" << tokens[i].textColor << "]" << std::endl;
    }
    
    // Check that deepest level works
    bool foundL10 = false;
    for(const auto& token : tokens) {
        if(token.content == "L10") {
            EXPECT_STREQ(token.textColor.c_str(), "white") << "L10 should keep its textColor";
            EXPECT_STREQ(token.bgColor.c_str(), "blue") << "L10 should inherit bgColor from root";
            foundL10 = true;
        }
    }
    EXPECT_TRUE(foundL10) << "Should find L10 token at 10th level deep";
}

TEST(NestedTokenDepth, twentyLevelsDeep_StressTest)
{
    // Test extremely deep nesting - 20 levels
    std::string input = "{L1 {L2 {L3 {L4 {L5 {L6 {L7 {L8 {L9 {L10 {L11 {L12 {L13 {L14 {L15 {L16 {L17 {L18 {L19 {L20}$[textColor=bright-red]}$[textColor=bright-green]}$[textColor=bright-blue]}$[textColor=bright-yellow]}$[textColor=bright-magenta]}$[textColor=bright-cyan]}$[textColor=bright-white]}$[textColor=red]}$[textColor=green]}$[textColor=blue]}$[textColor=yellow]}$[textColor=magenta]}$[textColor=cyan]}$[textColor=white]}$[textColor=black]}$[textColor=gray]}$[textColor=bright-gray]}$[textColor=dark-red]}$[textColor=dark-green]}$[bgColor=black,bold=true]";
    
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(input);
    
    EXPECT_GT(tokens.size(), 0) << "Should handle 20-level deep nesting without crashing";
    
    // Debug output (limited to avoid spam)
    std::cout << "\n=== 20-Level Stress Test ===" << std::endl;
    std::cout << "Input length: " << input.length() << " characters" << std::endl;
    std::cout << "Tokens found: " << tokens.size() << std::endl;
    
    // Check that the deepest token exists and has proper inheritance
    bool foundL20 = false;
    for(const auto& token : tokens) {
        if(token.content == "L20") {
            EXPECT_STREQ(token.textColor.c_str(), "bright-red") << "L20 should keep its textColor";
            EXPECT_STREQ(token.bgColor.c_str(), "black") << "L20 should inherit bgColor from root";
            EXPECT_TRUE(token.bold) << "L20 should inherit bold=true from root";
            foundL20 = true;
            std::cout << "Found L20: textColor=" << token.textColor << ", bgColor=" << token.bgColor << ", bold=" << token.bold << std::endl;
        }
    }
    EXPECT_TRUE(foundL20) << "Should find L20 token at 20th level deep";
}

/*** Legacy Nested Token Tests (preserved for compatibility) ***/
TEST(tokenizePrintString, simpleNestedTokens)
{
    // Arrange
    std::string input = "{outer {inner}$[textColor=red] content}$[textColor=blue]";
    
    // Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(input);
    
    // Assert - Should parse as nested tokens properly
    EXPECT_GT(tokens.size(), 0) << "Should find at least one token";
    // Debug output
    for(size_t i = 0; i < tokens.size(); i++) {
        std::cout << "Token " << i << ": " << tokens[i].rawToken << std::endl;
    }
}

TEST(tokenizePrintString, muggedManScenario)
{
    // Arrange - The exact problematic string from the Mugged Man event  
    std::string input = "{1 - {+5 local opinion}$[textColor=bright-green], {-5 gold}$[textColor=red] Take the man to an inn to care for his wounds}$[textColor=bright-yellow,bgColor=black]";
    std::string originalInput = input; // Keep original for later
    
    // Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(input);
    
    // Assert
    EXPECT_GT(tokens.size(), 0) << "Should find at least one token";
    
    // Debug output to understand what's being parsed
    std::cout << "Original: " << originalInput << std::endl;
    std::cout << "Modified: " << input << std::endl;
    std::cout << "Found " << tokens.size() << " tokens:" << std::endl;
    for(size_t i = 0; i < tokens.size(); i++) {
        std::cout << "Token " << i << ": '" << tokens[i].rawToken << "'" << std::endl;
        std::cout << "  - Position: " << tokens[i].existsAtIndex << std::endl;
        std::cout << "  - Content: '" << tokens[i].content << "'" << std::endl;
        std::cout << "  - TextColor: " << tokens[i].textColor << std::endl;
    }
    
    // Test the full styling pipeline
    std::string styledOutput = s_TerminalPrintHelper::styleTokens(input, tokens);
    std::cout << "Final styled output: " << styledOutput << std::endl;
}

TEST(tokenizePrintString, doubleNestedTokens)
{
    // Arrange - Test with double nesting
    std::string input = "{outer {first inner}$[textColor=red] and {second inner}$[textColor=green] content}$[textColor=blue]";
    
    // Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(input);
    
    // Assert
    EXPECT_GT(tokens.size(), 0) << "Should find tokens";
    
    // Debug output
    for(size_t i = 0; i < tokens.size(); i++) {
        std::cout << "Token " << i << ": " << tokens[i].rawToken << std::endl;
    }
}


/*** ***/


/*** s_TerminalPrintTokenHelper::getToken() ***/
// TEST(getToken, startIndexIsOutOfBounds)
// {
//     //Arrange
//     std::string str = "{The most distasteful thing to a man is the path that leads him to himself.}$[textColor=black]";
//     size_t tokenPosition = 9001;
//     //Act
//     s_TerminalPrintToken token = s_TerminalPrintTokenHelper::getToken(str, tokenPosition);
//     //Assert
//     ASSERT_EQ(token.existsAtIndex, 0);
// }

/*** s_TerminalPrintHelper::tokenizeString ***/
TEST(tokenizePrintString, emptyString)
{
    //Arrange
    std::string str = "";
    //Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(str);
    //Assert
    ASSERT_EQ( tokens.size(), 0 );
}

TEST(tokenizePrintString, singleTokenWholeString)
{
    //Arrange
    std::string str = "{This whole string is tokenized!}$[textColor=pink,bgColor=bright-green]";
    //Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(str);
    //Assert
    ASSERT_EQ( tokens.size(), 1 );
}

TEST(tokenizePrintString, multipleTokens)
{
    //Arrange
    std::string str = "{This bit of string is tokenized!}$[textColor=pink,bgColor=bright-green] and {this is another token}$[textColor=red,bgColor=blue]";
    //Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(str);
    //Assert
    ASSERT_EQ( tokens.size(), 2 );
}

TEST(tokenizePrintString, nestedTokens)
{
    //Arrange
    std::string str = "{The quick brown {fox}$[textColor=red] jumps over the lazy dog}$[textColor=green,bgColor=blue]";
    
    // Debug getAllTokens first
    std::vector<s_TerminalPrintToken> allTokens = s_TerminalPrintTokenHelper::getAllTokens(str);
    std::cout << "getAllTokens found " << allTokens.size() << " tokens:" << std::endl;
    for(size_t i = 0; i < allTokens.size(); i++) {
        std::cout << "  Raw Token " << i << ": '" << allTokens[i].rawToken << "' (pos: " << allTokens[i].existsAtIndex << ")" << std::endl;
    }
    
    //Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(str);
    
    // Debug output
    std::cout << "Input: " << str << std::endl;
    std::cout << "tokenizePrintString found " << tokens.size() << " tokens:" << std::endl;
    for(size_t i = 0; i < tokens.size(); i++) {
        std::cout << "Token " << i << ": '" << tokens[i].content << "' (pos: " << tokens[i].existsAtIndex << ", textColor: " << tokens[i].textColor << ", bgColor: " << tokens[i].bgColor << ")" << std::endl;
    }
    
    //Assert
    ASSERT_EQ(tokens.size(), 3) << "Should have exactly 3 tokens";
    if(tokens.size() >= 3) {
        ASSERT_STREQ( tokens[0].content.c_str(), "The quick brown " );
        ASSERT_STREQ( tokens[0].bgColor.c_str(), "blue" );
        ASSERT_STREQ( tokens[0].textColor.c_str(), "green" );
        ASSERT_STREQ( tokens[1].content.c_str(), "fox");
        ASSERT_STREQ( tokens[1].bgColor.c_str(), "blue" );
        ASSERT_STREQ( tokens[1].textColor.c_str(), "red" );
        ASSERT_STREQ( tokens[2].content.c_str(), " jumps over the lazy dog" );
        ASSERT_STREQ( tokens[2].bgColor.c_str(), "blue" );
        ASSERT_STREQ( tokens[2].textColor.c_str(), "green" );
    }
}

TEST(tokenizePrintString, nestedTokensV2)
{   
    //Arrange
    std::string str = "{Toggle text styling ({On}$[textColor=bright-green])}$[textColor=bright-yellow,bgColor=black]";
    //Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(str);
    //Assert
    ASSERT_STREQ( tokens[0].content.c_str(), "Toggle text styling (" );
    ASSERT_STREQ( tokens[0].bgColor.c_str(), "black" );
    ASSERT_STREQ( tokens[0].textColor.c_str(), "bright-yellow" );
    ASSERT_STREQ( tokens[0].getStyleString().c_str(), "textColor=bright-yellow,bgColor=black,blink=false,bold=false" );
    //ASSERT_STREQ( tokens[0].rawToken.c_str(), "{Toggle text styling (}$[textColor=bright-yellow,bgColor=black,blink=false,bold=false]" );

    ASSERT_STREQ( tokens[1].content.c_str(), "On" );
    ASSERT_STREQ( tokens[1].bgColor.c_str(), "black" );
    ASSERT_STREQ( tokens[1].textColor.c_str(), "bright-green" );
    ASSERT_STREQ( tokens[1].getStyleString().c_str(), "textColor=bright-green,bgColor=black,blink=false,bold=false" );
    //ASSERT_STREQ( tokens[1].rawToken.c_str(), "{On}$[textColor=bright-green,bgColor=black,blink=false,bold=false]" );

    ASSERT_STREQ( tokens[2].content.c_str(), ")" );
    ASSERT_STREQ( tokens[2].bgColor.c_str(), "black" );
    ASSERT_STREQ( tokens[2].textColor.c_str(), "bright-yellow" );
    ASSERT_STREQ( tokens[2].getStyleString().c_str(), "textColor=bright-yellow,bgColor=black,blink=false,bold=false" );
    //ASSERT_STREQ( tokens[2].rawToken.c_str(), "{)}$[textColor=bright-yellow,bgColor=black,blink=false,bold=false]" );
}

/**
 * When we tokenize a string with parent token of nested tokens, and there's no content between the nested tokens,
 * no between tokens are tokenized. However, the parent token styling is applied to the nested tokens if the nested
 * tokens have not specified their own styling for an attribute. In this case, blinking is styled by the parent token
 * and not by the nested tokens, so the nested tokens should adopt the parent token's blinking style.
 */
TEST(tokenizePrintString, twoNestedTokens_noBetween)
{
    //Arrange
    std::string str = "{{First nested token}$[textColor=bright-yellow,bgColor=black]{ and the second nested token with no between}$[textColor=bright-green,bgColor=black]}$[textColor=white,bgColor=aqua,blink=true]";
    
    // Debug output
    std::cout << "Input: " << str << std::endl;
    
    //Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(str);
    
    // Debug output
    std::cout << "Found " << tokens.size() << " tokens:" << std::endl;
    for(size_t i = 0; i < tokens.size(); i++) {
        std::cout << "Token " << i << ": '" << tokens[i].content << "' (textColor: " << tokens[i].textColor << ", blink: " << tokens[i].blink << ")" << std::endl;
    }
    
    //Assert - Optimized version eliminates empty tokens
    ASSERT_EQ( tokens.size(), 2 );

    if(tokens.size() >= 2) {
        ASSERT_STREQ( tokens[0].content.c_str(), "First nested token" );
        ASSERT_STREQ( tokens[0].bgColor.c_str(), "black" );
        ASSERT_STREQ( tokens[0].textColor.c_str(), "bright-yellow" );
        ASSERT_STREQ( tokens[0].getStyleString().c_str(), "textColor=bright-yellow,bgColor=black,blink=true,bold=false" );
        //ASSERT_STREQ( tokens[0].rawToken.c_str(), "{First nested token}$[textColor=bright-yellow,bgColor=black,blink=true,bold=false]" );

        ASSERT_STREQ( tokens[1].content.c_str(), " and the second nested token with no between" );
        ASSERT_STREQ( tokens[1].bgColor.c_str(), "black" );
        ASSERT_STREQ( tokens[1].textColor.c_str(), "bright-green" );
        ASSERT_STREQ( tokens[1].getStyleString().c_str(), "textColor=bright-green,bgColor=black,blink=true,bold=false" );
        //ASSERT_STREQ( tokens[1].rawToken.c_str(), "{ and the second nested token with no between}$[textColor=bright-green,bgColor=black,blink=true,bold=false]" );
    }
}


/*** s_TerminalPrintHelper::tokenizeBetweenTokens() ***/
TEST(tokenizeBetweenTokens, emptyString)
{
    //Arrange
    std::string str = "";
    std::vector<s_TerminalPrintToken> tokensBefore;
    //Act
    std::vector<s_TerminalPrintToken> tokensAfter = s_TerminalPrintHelper::tokenizeBetweenTokens(str, tokensBefore);
    //Assert
    ASSERT_EQ( tokensAfter.size(), 0 );
}

TEST(tokenizeBetweenTokens, noTokens)
{
    //Arrange
    std::string str = "No tokens in here!";
    std::vector<s_TerminalPrintToken> tokens;
    //Act
    tokens = s_TerminalPrintHelper::tokenizeBetweenTokens(str, tokens);
    //Assert
    ASSERT_EQ( tokens.size(), 1 );
    ASSERT_STREQ( tokens[0].content.c_str(), "No tokens in here!" );
}

TEST(tokenizeBetweenTokens, oneToken)
{
    //Arrange
    std::string str = "First part of the string is not a token. {This is a token.}$[textColor=red]";
    std::vector<s_TerminalPrintToken> styleTokens = s_TerminalPrintHelper::tokenizePrintString(str);
    //Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizeBetweenTokens(str, styleTokens);

    ASSERT_EQ( tokens.size(), 2 );
    ASSERT_STREQ( tokens[0].content.c_str(), "First part of the string is not a token. " );
    ASSERT_STREQ( tokens[1].content.c_str(), "This is a token." );
}

TEST(tokenizeBetweenTokens, twoTokens)
{
    //Arrange
    std::string str = "First part of the string is not a token. {This is a token}$[textColor=red] and {this is another token.}$[textColor=blue]";
    std::vector<s_TerminalPrintToken> styleTokens = s_TerminalPrintHelper::tokenizePrintString(str);
    //Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizeBetweenTokens(str, styleTokens);

    std::cout << str << std::endl;
    for(int i = 0; i < tokens.size(); i++)
    {
        std::cout << "Token " << i << ": " << tokens[i].rawToken
                  << ", existsAtIndex: " << tokens[i].existsAtIndex << std::endl;
    }

    //Assert
    ASSERT_EQ( tokens.size(), 4 );
    ASSERT_STREQ( tokens[0].content.c_str(), "First part of the string is not a token. " );
    ASSERT_STREQ( tokens[1].content.c_str(), "This is a token" );
    ASSERT_STREQ( tokens[2].content.c_str(), " and " );
    ASSERT_STREQ( tokens[3].content.c_str(), "this is another token." );
}

TEST(tokenizeBetweenTokens, oneNestedToken)
{
    //Arrange
    std::string str = "{This is a parent token {with a nested token}$[textColor=bright-green] dropped right in the middle.}$[textColor=red,bgColor=blue]";
    //Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(str);
    //Assert
    ASSERT_EQ( tokens.size(), 3 );
    ASSERT_STREQ( tokens[0].content.c_str(), "This is a parent token " );
    ASSERT_STREQ( tokens[1].content.c_str(), "with a nested token" );
    ASSERT_STREQ( tokens[2].content.c_str(), " dropped right in the middle." );
}


/*** s_TerminaLPrintTokenHelper::getAllTokens() ***/
TEST(getAllTokens, noneInString)
{
    //Arrange
    std::string str = "No style tokens in here!";
    //Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintTokenHelper::getAllTokens(str);
    //Assert
    ASSERT_EQ( tokens.size(), 0 );
}

TEST(getAllTokens, wholeStringIsAToken)
{
    //Arrange
    std::string str = "{This whole string is tokenized!}$[textColor=pink,bgColor=bright-green]";
    //Act
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintTokenHelper::getAllTokens(str);
    //Assert
    //TODO, make it so we check for token equality
    ASSERT_EQ( tokens.size(), 1);
}


/*** printVector_str() ***/
TEST(printVector_str,  defaultRows_singleColumnSpecified)
{
    //Arrange
    std::vector<std::string> vec = {"Vin", "Kelsier", "Sazed", "Dockson", "Breeze", "Ham", "Spook", "Clubs"};
    //Act
    std::string result = stevensTerminal::printVector_str(   vec,
                                                            {   {"columns", "1"}    }   );
    //Assert
    ASSERT_STREQ( result.c_str(), "Vin\t\nKelsier\t\nSazed\t\nDockson\t\nBreeze\t\nHam\t\nSpook\t\nClubs\t\n");
}

TEST(printVector_str, fourRows_twoColumns)
{
    //Arrange
    std::vector<std::string> vec = {"Vin", "Kelsier", "Sazed", "Dockson", "Breeze", "Ham", "Spook", "Clubs"};
    //Act
    std::string result = stevensTerminal::printVector_str(   vec,
                                                            {   {"columns",     "2"},
                                                                {"rows",        "4"}    }   );
    //Assert
    ASSERT_STREQ( result.c_str(), "Vin\tBreeze\t\nKelsier\tHam\t\nSazed\tSpook\t\nDockson\tClubs\t\n");
}

TEST(printVector_str, threeRows_threeColumns)
{
    //Arrange
    std::vector<std::string> vec = { "Lawful Good", "Lawful Neutral", "Lawful Evil", "Neutral Good", "True Neutral",
                                     "Neutral Evil", "Chaotic Good", "Chaotic Neutral", "Chaotic Evil"};
    //Act
    std::string result = stevensTerminal::printVector_str(  vec,
                                                            {   {"columns", "3"},
                                                                {"rows",    "3"}    }   );
    //Assert
    ASSERT_STREQ( result.c_str(),   "Lawful Good\tNeutral Good\tChaotic Good\t\n"
                                    "Lawful Neutral\tTrue Neutral\tChaotic Neutral\t\n"
                                    "Lawful Evil\tNeutral Evil\tChaotic Evil\t\n");
}

TEST(printVector_str, columnOverflow)
{
    //Arrange
    std::vector<std::string> vec = {"Jack", "Sally", "Mayor", "Lock", "Shock", "Barrel", "Oogie Boogie"};
    //Act
    std::string result = stevensTerminal::printVector_str( vec,
                                                            {   {"columns", "2"},
                                                                {"rows",    "3"},
                                                                {"allowOverflow",    "true"}    }   );
    //Assert
    ASSERT_STREQ(   result.c_str(), "Jack\tLock\tOogie Boogie\t\n"
                                    "Sally\tShock\t\n"
                                    "Mayor\tBarrel\t\n");
}

TEST(printVector_str, autoColumnWidth)
{
    //Arrange
    std::vector<std::string> vec = {"Actions", "World Map", "Theology", "Infrastructure", "Monthly Report", "Settings", "End Month", "Quit"};
    //Act
    std::string result = stevensTerminal::printVector_str(  vec,
                                                            {   {"columns", "2"},
                                                                {"rows",    "4"},
                                                                {"defaultColumnWidth", "auto"}    }   );
    //Assert
    ASSERT_STREQ(   result.c_str(), "Actions       \tMonthly Report\t\n"
                                    "World Map     \tSettings      \t\n"
                                    "Theology      \tEnd Month     \t\n"
                                    "Infrastructure\tQuit          \t\n");
}


/*** s_TerminalPrintHelper::ignoreTokenStyling() ***/
TEST(ignoreTokenStyling, emptyStringWithNoTokens)
{
    //Arrange
    std::vector<s_TerminalPrintToken> tokens = {};
    //Act
    std::string result = s_TerminalPrintHelper::ignoreTokenStyling("", tokens);
    //Assert
    ASSERT_STREQ( result.c_str(), "" );
}

TEST(ignoreTokenStyling, stringWithNoTokens)
{
    //Arrange
    std::vector<s_TerminalPrintToken> tokens = {};
    //Act
    std::string result = s_TerminalPrintHelper::ignoreTokenStyling("This is a string with no tokens.", tokens);
    //Assert
    ASSERT_STREQ( result.c_str(), "This is a string with no tokens.");
}

TEST(ignoreTokenStyling, oneToken)
{
    //Arrange
    std::string inputString = "This is a string with {a token.}$[textColor=red]";
    std::vector<s_TerminalPrintToken> tokens = s_TerminalPrintHelper::tokenizePrintString(inputString);
    //Act
    std::string result = s_TerminalPrintHelper::ignoreTokenStyling(inputString, tokens);
    //Assert
    ASSERT_STREQ( result.c_str(), "This is a string with a token.");
}



/*** curses_wwrap with tokens ***/
// TEST(curses_wwrap_withTokens, emptyString)
// {
//     //Arrange
//     std::string str = "";
//     //Act
//     stevensTerminalcurses_wwrap
//     //Assert
//     ASSERT_EQ( tokens.size(), 0 );
// }


/***** MODULAR ARCHITECTURE TESTS *****/

/*** Core.hpp Module Tests ***/
TEST(CoreModule, get_screen_size_accessible)
{
    // Test that get_screen_size is accessible through modular structure
    auto screenSize = stevensTerminal::get_screen_size();
    ASSERT_GT(screenSize.first, 0);
    ASSERT_GT(screenSize.second, 0);
}

TEST(CoreModule, display_modes_accessible)
{
    // Test that displayModes map is accessible and populated
    ASSERT_FALSE(stevensTerminal::displayModes.empty());
    ASSERT_NE(stevensTerminal::displayModes.find("regular"), stevensTerminal::displayModes.end());
    ASSERT_NE(stevensTerminal::displayModes.find("small"), stevensTerminal::displayModes.end());
    ASSERT_NE(stevensTerminal::displayModes.find("very small"), stevensTerminal::displayModes.end());
}

TEST(CoreModule, setDisplayMode_functionality)
{
    // Test setDisplayMode function works
    std::string originalMode = stevensTerminal::currentDisplayMode;
    stevensTerminal::setDisplayMode({120, 40});
    ASSERT_FALSE(stevensTerminal::currentDisplayMode.empty());
    // Mode should be set to something valid
    ASSERT_NE(stevensTerminal::displayModes.find(stevensTerminal::currentDisplayMode), 
              stevensTerminal::displayModes.end());
}

TEST(CoreModule, displayMode_validation_functions)
{
    // Test displayMode_GTEminSize
    ASSERT_TRUE(stevensTerminal::displayMode_GTEminSize({100, 30}, {120, 40}));
    ASSERT_FALSE(stevensTerminal::displayMode_GTEminSize({150, 50}, {120, 40}));
    ASSERT_TRUE(stevensTerminal::displayMode_GTEminSize({-1, -1}, {50, 30})); // unlimited bounds
    
    // Test displayMode_LTEmaxSize  
    ASSERT_TRUE(stevensTerminal::displayMode_LTEmaxSize({150, 50}, {120, 40}));
    ASSERT_FALSE(stevensTerminal::displayMode_LTEmaxSize({100, 30}, {120, 40}));
    ASSERT_TRUE(stevensTerminal::displayMode_LTEmaxSize({-1, -1}, {120, 40})); // unlimited bounds
}

/*** Styling.hpp Module Tests ***/
TEST(StylingModule, addStyleToken_basic_functionality)
{
    // Test basic style token creation
    std::string text = "Hello World";
    std::unordered_map<std::string, std::string> styles = {{"textColor", "red"}, {"bold", "true"}};
    std::string styledText = stevensTerminal::addStyleToken(text, styles);
    
    ASSERT_NE(styledText.find("{"), std::string::npos);
    ASSERT_NE(styledText.find("}$["), std::string::npos);
    ASSERT_NE(styledText.find("textColor=red"), std::string::npos);
    ASSERT_NE(styledText.find("bold=true"), std::string::npos);
}

TEST(StylingModule, addStyleToken_empty_styles)
{
    // Test with empty style map
    std::string text = "Test";
    std::unordered_map<std::string, std::string> emptyStyles;
    std::string result = stevensTerminal::addStyleToken(text, emptyStyles);
    
    ASSERT_NE(result.find("{Test}$["), std::string::npos);
    ASSERT_FALSE(result.empty());
}

TEST(StylingModule, removeAllStyleTokenization_basic)
{
    // Test removing style tokens
    std::string text = "Hello World";
    std::unordered_map<std::string, std::string> styles = {{"textColor", "red"}};
    std::string styledText = stevensTerminal::addStyleToken(text, styles);
    std::string cleanText = stevensTerminal::removeAllStyleTokenization(styledText);
    
    ASSERT_STREQ(cleanText.c_str(), text.c_str());
}

TEST(StylingModule, removeAllStyleTokenization_multiple_tokens)
{
    // Test removing multiple style tokens
    std::string originalText = "Hello World";
    std::string styledText = "{Hello}$[textColor=red] {World}$[textColor=blue]";
    std::string cleanText = stevensTerminal::removeAllStyleTokenization(styledText);
    
    ASSERT_STREQ(cleanText.c_str(), originalText.c_str());
}

TEST(StylingModule, resizeStyledString_basic)
{
    // Test resizing styled strings
    std::string text = "Hello World";
    std::unordered_map<std::string, std::string> styles = {{"textColor", "red"}};
    std::string styledText = stevensTerminal::addStyleToken(text, styles);
    std::string resized = stevensTerminal::resizeStyledString(styledText, 5);
    std::string resizedClean = stevensTerminal::removeAllStyleTokenization(resized);
    
    ASSERT_EQ(resizedClean.length(), 5);
    ASSERT_STREQ(resizedClean.c_str(), "Hello");
}

TEST(StylingModule, resizeStyledString_zero_length)
{
    // Test resize to zero length
    std::string text = "Test";
    std::unordered_map<std::string, std::string> styles = {{"textColor", "blue"}};
    std::string styledText = stevensTerminal::addStyleToken(text, styles);
    std::string resized = stevensTerminal::resizeStyledString(styledText, 0);
    std::string resizedClean = stevensTerminal::removeAllStyleTokenization(resized);
    
    ASSERT_TRUE(resizedClean.empty());
}

TEST(StylingModule, resizeStyledString_expand_with_fill)
{
    // Test expanding string with fill character
    std::string text = "Hi";
    std::unordered_map<std::string, std::string> styles = {{"textColor", "green"}};
    std::string styledText = stevensTerminal::addStyleToken(text, styles);
    std::string resized = stevensTerminal::resizeStyledString(styledText, 5, '*');
    std::string resizedClean = stevensTerminal::removeAllStyleTokenization(resized);
    
    ASSERT_EQ(resizedClean.length(), 5);
    ASSERT_STREQ(resizedClean.c_str(), "Hi***");
}

/*** Input.hpp Module Tests ***/
TEST(InputModule, inputWithinResponseRange_valid_input)
{
    // Test valid numeric input within range
    ASSERT_TRUE(stevensTerminal::inputWithinResponseRange("3", 5));
    ASSERT_TRUE(stevensTerminal::inputWithinResponseRange("1", 5));
    ASSERT_TRUE(stevensTerminal::inputWithinResponseRange("5", 5));
}

TEST(InputModule, inputWithinResponseRange_invalid_input)
{
    // Test invalid inputs
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("6", 5)); // out of range
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("0", 5)); // below range (expects 1-based)
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("abc", 5)); // non-numeric
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("", 5)); // empty string
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("-1", 5)); // negative
}

TEST(InputModule, inputWithinResponseRange_edge_cases)
{
    // Test edge cases
    ASSERT_TRUE(stevensTerminal::inputWithinResponseRange("1", 1)); // single response range
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("2", 1)); // exceeds single response
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("1", 0)); // zero response range
}

/*** Module Integration Tests ***/
TEST(ModularIntegration, all_modules_accessible_through_main_header)
{
    // Test that all modules are accessible through stevensTerminal.hpp
    
    // Core functionality
    auto screenSize = stevensTerminal::get_screen_size();
    ASSERT_GT(screenSize.first, 0);
    
    // Styling functionality
    std::string styledText = stevensTerminal::addStyleToken("test", {{"textColor", "red"}});
    ASSERT_FALSE(styledText.empty());
    
    // Input validation functionality
    bool validInput = stevensTerminal::inputWithinResponseRange("2", 3);
    ASSERT_TRUE(validInput);
}

TEST(ModularIntegration, namespace_consistency)
{
    // Test that all functions are properly in stevensTerminal namespace
    // This is verified by compilation - if namespace is wrong, tests won't compile
    ASSERT_TRUE(true); // If we get here, namespace is working correctly
}

TEST(ModularIntegration, cross_module_functionality)
{
    // Test functionality that spans multiple modules
    auto screenSize = stevensTerminal::get_screen_size();
    stevensTerminal::setDisplayMode(screenSize);
    
    std::string testText = "Integration Test";
    std::unordered_map<std::string, std::string> styles = {{"textColor", "green"}};
    std::string styledText = stevensTerminal::addStyleToken(testText, styles);
    std::string cleanText = stevensTerminal::removeAllStyleTokenization(styledText);
    
    ASSERT_STREQ(cleanText.c_str(), testText.c_str());
    ASSERT_TRUE(stevensTerminal::inputWithinResponseRange("1", 5));
}

/*** Error Handling and Edge Cases ***/
TEST(ErrorHandling, styling_with_empty_strings)
{
    // Test styling functions with empty strings
    std::string result = stevensTerminal::addStyleToken("", {{"textColor", "red"}});
    ASSERT_FALSE(result.empty());
    
    std::string cleaned = stevensTerminal::removeAllStyleTokenization(result);
    ASSERT_TRUE(cleaned.empty());
}

TEST(ErrorHandling, input_validation_boundary_conditions)
{
    // Test input validation at boundaries
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("0", 0)); // zero range
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("999999", 5)); // very large number
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange(" 3 ", 5)); // whitespace (should fail)
}

/***** MAIN (runs all the tests) *****/
int main(   int argc,
            char * argv[]   )
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
// Additional tests for bug fixes and error handling improvements

TEST(BugFixes, division_by_zero_in_bar_graph)
{
    // Test that horizontalStackedBarGraph doesn't crash with zero distribution
    std::vector<std::string> labels = {"A", "B"};
    std::vector<std::tuple<std::string, std::string>> colors = {std::make_tuple("red", "default"), std::make_tuple("blue", "default")};
    std::vector<float> distribution = {0.0f, 0.0f}; // Zero sum

    // Should not crash or cause undefined behavior
    stevensTerminal::horizontalStackedBarGraph(labels, colors, distribution, 100, true, true);
    SUCCEED(); // If we get here without crashing, test passes
}

TEST(BugFixes, mismatched_vector_sizes_in_bar_graph)
{
    // Test validation of mismatched input vector sizes
    std::vector<std::string> labels = {"A", "B", "C"};
    std::vector<std::tuple<std::string, std::string>> colors = {std::make_tuple("red", "default"), std::make_tuple("blue", "default")}; // Size mismatch
    std::vector<float> distribution = {1.0f, 2.0f, 3.0f};

    // Should handle gracefully without crashing
    stevensTerminal::horizontalStackedBarGraph(labels, colors, distribution, 100, true, true);
    SUCCEED();
}

TEST(BugFixes, negative_width_in_bar_graph)
{
    // Test validation of negative width
    std::vector<std::string> labels = {"A"};
    std::vector<std::tuple<std::string, std::string>> colors = {std::make_tuple("red", "default")};
    std::vector<float> distribution = {1.0f};

    // Should handle gracefully
    stevensTerminal::horizontalStackedBarGraph(labels, colors, distribution, -10, true, true);
    SUCCEED();
}

TEST(Utilities, resize_styled_string_preserves_styling)
{
    std::string styled = "{Hello}$[textColor=red]";
    std::string resized = stevensTerminal::resizeStyledString(styled, 10, ' ');

    // Should preserve styling tokens
    ASSERT_NE(resized.find("$["), std::string::npos);
    ASSERT_NE(resized.find("textColor"), std::string::npos);
}


/***** UTILITY LIBRARY TESTS *****/

/*** stevensMathLib Tests ***/
TEST(StevensMathLib, roundToNearest10th_integer)
{
    // Test rounding an integer (should return as integer)
    ASSERT_EQ(stevensMathLib::roundToNearest10th(5.0f), 5.0f);
    ASSERT_EQ(stevensMathLib::roundToNearest10th(10.0f), 10.0f);
}

TEST(StevensMathLib, roundToNearest10th_decimal)
{
    // Test rounding numbers with decimals
    ASSERT_FLOAT_EQ(stevensMathLib::roundToNearest10th(3.14159f), 3.1f);
    ASSERT_FLOAT_EQ(stevensMathLib::roundToNearest10th(2.78f), 2.8f);
    ASSERT_FLOAT_EQ(stevensMathLib::roundToNearest10th(1.05f), 1.1f);
}

TEST(StevensMathLib, roundToNearest10th_rounds_to_integer)
{
    // Test case where rounding brings us back to an integer
    ASSERT_EQ(stevensMathLib::roundToNearest10th(4.95f), 5.0f);
}

TEST(StevensMathLib, round_with_precision)
{
    // Test rounding to various precisions
    ASSERT_FLOAT_EQ(stevensMathLib::round(3.14159f, 2), 3.14f);
    ASSERT_FLOAT_EQ(stevensMathLib::round(3.14159f, 3), 3.142f);
    ASSERT_FLOAT_EQ(stevensMathLib::round(2.5f, 0), 3.0f);
}

TEST(StevensMathLib, randomInt_within_bounds)
{
    // Seed RNG for reproducibility
    stevensMathLib::seedRNG();

    // Test that random int is within bounds
    for(int i = 0; i < 100; i++) {
        int result = stevensMathLib::randomInt(0, 10);
        ASSERT_GE(result, 0);
        ASSERT_LT(result, 10);
    }
}

TEST(StevensMathLib, randomInt_edge_case_equal_bounds)
{
    // When upperBound <= lowerBound, should return lowerBound
    ASSERT_EQ(stevensMathLib::randomInt(5, 5), 5);
    ASSERT_EQ(stevensMathLib::randomInt(10, 5), 10);
}

TEST(StevensMathLib, randomFloat_within_bounds)
{
    stevensMathLib::seedRNG();

    // Test that random float is within bounds
    for(int i = 0; i < 100; i++) {
        float result = stevensMathLib::randomFloat(0.0f, 1.0f);
        ASSERT_GE(result, 0.0f);
        ASSERT_LE(result, 1.0f);
    }
}

TEST(StevensMathLib, randomFloat_custom_range)
{
    stevensMathLib::seedRNG();

    for(int i = 0; i < 100; i++) {
        float result = stevensMathLib::randomFloat(10.0f, 20.0f);
        ASSERT_GE(result, 10.0f);
        ASSERT_LE(result, 20.0f);
    }
}

TEST(StevensMathLib, randomIntNotInVec_excludes_blacklist)
{
    stevensMathLib::seedRNG();

    std::vector<int> blacklist = {1, 3, 5, 7, 9};

    // Generate multiple random ints and verify none are in blacklist
    for(int i = 0; i < 50; i++) {
        int result = stevensMathLib::randomIntNotInVec(blacklist, 0, 10);
        ASSERT_FALSE(stevensVectorLib::contains(blacklist, result));
        ASSERT_GE(result, 0);
        ASSERT_LT(result, 10);
    }
}

TEST(StevensMathLib, randomIntNotInVec_throws_when_no_valid_integers)
{
    // Create a blacklist that excludes all possible values
    std::vector<int> blacklist = {0, 1, 2, 3, 4};

    // Should throw when no valid integers remain
    ASSERT_THROW(stevensMathLib::randomIntNotInVec(blacklist, 0, 5), std::invalid_argument);
}

TEST(StevensMathLib, floatToInt_safe_conversion)
{
    // Test safe conversion of floats to ints
    ASSERT_EQ(stevensMathLib::floatToInt(3.7f), 3);
    ASSERT_EQ(stevensMathLib::floatToInt(10.2f), 10);
    ASSERT_EQ(stevensMathLib::floatToInt(-5.8f), -5);
}

TEST(StevensMathLib, floatToInt_large_values)
{
    // Test handling of very large float values
    float largeValue = 1e15f;
    int result = stevensMathLib::floatToInt(largeValue);
    // Should return max int for values that don't fit
    ASSERT_TRUE(result == std::numeric_limits<int>::max() || result == static_cast<int>(largeValue));
}

TEST(StevensMathLib, in_range_inclusive)
{
    // Test inclusive range checking
    ASSERT_TRUE(stevensMathLib::in_range(5, 1, 10, "inclusive"));
    ASSERT_TRUE(stevensMathLib::in_range(1, 1, 10, "inclusive"));  // Lower bound
    ASSERT_TRUE(stevensMathLib::in_range(10, 1, 10, "inclusive")); // Upper bound
    ASSERT_FALSE(stevensMathLib::in_range(0, 1, 10, "inclusive"));
    ASSERT_FALSE(stevensMathLib::in_range(11, 1, 10, "inclusive"));
}

TEST(StevensMathLib, in_range_exclusive)
{
    // Test exclusive range checking
    ASSERT_TRUE(stevensMathLib::in_range(5, 1, 10, "exclusive"));
    ASSERT_FALSE(stevensMathLib::in_range(1, 1, 10, "exclusive"));  // Lower bound excluded
    ASSERT_FALSE(stevensMathLib::in_range(10, 1, 10, "exclusive")); // Upper bound excluded
    ASSERT_FALSE(stevensMathLib::in_range(0, 1, 10, "exclusive"));
}

TEST(StevensMathLib, in_range_float_values)
{
    // Test range checking with floats
    ASSERT_TRUE(stevensMathLib::in_range(2.5f, 1.0f, 5.0f, "inclusive"));
    ASSERT_FALSE(stevensMathLib::in_range(0.5f, 1.0f, 5.0f, "inclusive"));
}


/*** stevensVectorLib Tests ***/
TEST(StevensVectorLib, contains_element_present)
{
    std::vector<int> vec = {1, 2, 3, 4, 5};
    ASSERT_TRUE(stevensVectorLib::contains(vec, 3));
    ASSERT_TRUE(stevensVectorLib::contains(vec, 1));
    ASSERT_TRUE(stevensVectorLib::contains(vec, 5));
}

TEST(StevensVectorLib, contains_element_absent)
{
    std::vector<int> vec = {1, 2, 3, 4, 5};
    ASSERT_FALSE(stevensVectorLib::contains(vec, 10));
    ASSERT_FALSE(stevensVectorLib::contains(vec, 0));
}

TEST(StevensVectorLib, contains_string_vector)
{
    std::vector<std::string> vec = {"apple", "banana", "cherry"};
    ASSERT_TRUE(stevensVectorLib::contains(vec, std::string("banana")));
    ASSERT_FALSE(stevensVectorLib::contains(vec, std::string("grape")));
}

TEST(StevensVectorLib, eraseAllOf_removes_all_instances)
{
    std::vector<int> vec = {1, 2, 3, 2, 4, 2, 5};
    std::vector<int> result = stevensVectorLib::eraseAllOf(vec, 2);

    ASSERT_EQ(result.size(), 4);
    ASSERT_FALSE(stevensVectorLib::contains(result, 2));
    ASSERT_TRUE(stevensVectorLib::contains(result, 1));
    ASSERT_TRUE(stevensVectorLib::contains(result, 3));
}

TEST(StevensVectorLib, eraseAllOf_element_not_present)
{
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::vector<int> result = stevensVectorLib::eraseAllOf(vec, 10);
    ASSERT_EQ(result.size(), 5);
}

TEST(StevensVectorLib, sumAll_integers)
{
    std::vector<int> vec = {1, 2, 3, 4, 5};
    ASSERT_EQ(stevensVectorLib::sumAll(vec, 0), 15);
    ASSERT_EQ(stevensVectorLib::sumAll(vec, 10), 25); // With initial value
}

TEST(StevensVectorLib, sumAll_floats)
{
    std::vector<float> vec = {1.5f, 2.5f, 3.0f};
    ASSERT_FLOAT_EQ(stevensVectorLib::sumAll(vec, 0.0f), 7.0f);
}

TEST(StevensVectorLib, multiplyAll_integers)
{
    std::vector<int> vec = {2, 3, 4};
    ASSERT_EQ(stevensVectorLib::multiplyAll(vec, 1), 24);
    ASSERT_EQ(stevensVectorLib::multiplyAll(vec, 2), 48);
}

TEST(StevensVectorLib, multiplyAll_floats)
{
    std::vector<float> vec = {2.0f, 3.0f, 1.5f};
    ASSERT_FLOAT_EQ(stevensVectorLib::multiplyAll(vec, 1.0f), 9.0f);
}

TEST(StevensVectorLib, vecOfStrings_to_vecOfInts)
{
    std::vector<std::string> strVec = {"1", "2", "3", "42", "-5"};
    std::vector<int> intVec = stevensVectorLib::vecOfStrings_to_vecOfInts(strVec);

    ASSERT_EQ(intVec.size(), 5);
    ASSERT_EQ(intVec[0], 1);
    ASSERT_EQ(intVec[3], 42);
    ASSERT_EQ(intVec[4], -5);
}

TEST(StevensVectorLib, vecOfStrings_to_vecOfLongLongInts)
{
    std::vector<std::string> strVec = {"1000000000", "2000000000"};
    std::vector<long long int> llVec = stevensVectorLib::vecOfStrings_to_vecOfLongLongInts(strVec);

    ASSERT_EQ(llVec.size(), 2);
    ASSERT_EQ(llVec[0], 1000000000LL);
    ASSERT_EQ(llVec[1], 2000000000LL);
}

TEST(StevensVectorLib, findElementIndex_found)
{
    std::vector<int> vec = {10, 20, 30, 40, 50};
    ASSERT_EQ(stevensVectorLib::findElementIndex(vec, 30), 2);
    ASSERT_EQ(stevensVectorLib::findElementIndex(vec, 10), 0);
    ASSERT_EQ(stevensVectorLib::findElementIndex(vec, 50), 4);
}

TEST(StevensVectorLib, findElementIndex_not_found)
{
    std::vector<int> vec = {10, 20, 30};
    ASSERT_EQ(stevensVectorLib::findElementIndex(vec, 100), std::numeric_limits<size_t>::max());
}

TEST(StevensVectorLib, findMin_integers)
{
    std::vector<int> vec = {5, 2, 8, 1, 9};
    ASSERT_EQ(stevensVectorLib::findMin(vec), 1);
}

TEST(StevensVectorLib, findMin_floats)
{
    std::vector<float> vec = {5.5f, 2.2f, 8.8f, 1.1f};
    ASSERT_FLOAT_EQ(stevensVectorLib::findMin(vec), 1.1f);
}

TEST(StevensVectorLib, getRandomElement)
{
    stevensMathLib::seedRNG();
    std::vector<int> vec = {1, 2, 3, 4, 5};

    // Test that random element is always in the vector
    for(int i = 0; i < 20; i++) {
        int result = stevensVectorLib::getRandomElement(vec);
        ASSERT_TRUE(stevensVectorLib::contains(vec, result));
    }
}

TEST(StevensVectorLib, getRandomElement_throws_on_empty)
{
    std::vector<int> emptyVec;
    ASSERT_THROW(stevensVectorLib::getRandomElement(emptyVec), std::invalid_argument);
}

TEST(StevensVectorLib, concat_two_vectors)
{
    std::vector<int> vec_a = {1, 2, 3};
    std::vector<int> vec_b = {4, 5, 6};
    std::vector<int> result = stevensVectorLib::concat(vec_a, vec_b);

    ASSERT_EQ(result.size(), 6);
    ASSERT_EQ(result[0], 1);
    ASSERT_EQ(result[3], 4);
    ASSERT_EQ(result[5], 6);
}

TEST(StevensVectorLib, concat_empty_vectors)
{
    std::vector<int> vec_a;
    std::vector<int> vec_b = {1, 2, 3};
    std::vector<int> result = stevensVectorLib::concat(vec_a, vec_b);

    ASSERT_EQ(result.size(), 3);
}

TEST(StevensVectorLib, getUncommonElements)
{
    std::vector<int> vec_a = {1, 2, 3, 4};
    std::vector<int> vec_b = {3, 4, 5, 6};
    std::vector<int> result = stevensVectorLib::getUncommonElements(vec_a, vec_b);

    // Should contain elements in a OR b but not both: 1, 2, 5, 6
    ASSERT_TRUE(stevensVectorLib::contains(result, 1));
    ASSERT_TRUE(stevensVectorLib::contains(result, 2));
    ASSERT_TRUE(stevensVectorLib::contains(result, 5));
    ASSERT_TRUE(stevensVectorLib::contains(result, 6));
    ASSERT_FALSE(stevensVectorLib::contains(result, 3));
    ASSERT_FALSE(stevensVectorLib::contains(result, 4));
}

TEST(StevensVectorLib, getLargestVectorElement)
{
    std::vector< std::vector<int> > vecOfVecs = {
        {1, 2},
        {3, 4, 5, 6, 7},
        {8, 9, 10}
    };

    std::vector<int> largest = stevensVectorLib::getLargestVectorElement(vecOfVecs);
    ASSERT_EQ(largest.size(), 5);
}

TEST(StevensVectorLib, getLongestStringElement)
{
    std::vector<std::string> vec = {"hi", "hello", "hey", "greetings"};
    std::string longest = stevensVectorLib::getLongestStringElement(vec);
    ASSERT_EQ(longest, "greetings");
}

TEST(StevensVectorLib, reorient2DVector_transpose)
{
    std::vector< std::vector<int> > inputVec = {
        {1, 2, 3},
        {4, 5, 6}
    };

    std::vector< std::vector<int> > result = stevensVectorLib::reorient2DVector(inputVec);

    // Should transpose: [[1,4], [2,5], [3,6]]
    ASSERT_EQ(result.size(), 3);
    ASSERT_EQ(result[0].size(), 2);
    ASSERT_EQ(result[0][0], 1);
    ASSERT_EQ(result[0][1], 4);
    ASSERT_EQ(result[1][0], 2);
    ASSERT_EQ(result[2][0], 3);
}

TEST(StevensVectorLib, eraseDuplicateElements)
{
    std::vector<int> vec = {1, 2, 2, 3, 3, 3, 4, 5, 5};
    std::vector<int> result = stevensVectorLib::eraseDuplicateElements(vec);

    ASSERT_EQ(result.size(), 5);
    ASSERT_TRUE(stevensVectorLib::contains(result, 1));
    ASSERT_TRUE(stevensVectorLib::contains(result, 2));
    ASSERT_TRUE(stevensVectorLib::contains(result, 3));
    ASSERT_TRUE(stevensVectorLib::contains(result, 4));
    ASSERT_TRUE(stevensVectorLib::contains(result, 5));
}

TEST(StevensVectorLib, difference)
{
    std::vector<int> vec_a = {1, 2, 3, 4, 5};
    std::vector<int> vec_b = {3, 4, 6};
    std::vector<int> result = stevensVectorLib::difference(vec_a, vec_b);

    // Should have 1, 2, 5 (elements in a but not in b)
    ASSERT_EQ(result.size(), 3);
    ASSERT_TRUE(stevensVectorLib::contains(result, 1));
    ASSERT_TRUE(stevensVectorLib::contains(result, 2));
    ASSERT_TRUE(stevensVectorLib::contains(result, 5));
    ASSERT_FALSE(stevensVectorLib::contains(result, 3));
}

TEST(StevensVectorLib, popFront)
{
    std::vector<int> vec = {10, 20, 30, 40};
    int first = stevensVectorLib::popFront(vec);

    ASSERT_EQ(first, 10);
    ASSERT_EQ(vec.size(), 3);
    ASSERT_EQ(vec[0], 20);
}


/*** stevensMapLib Tests ***/
TEST(StevensMapLib, addMaps_values_only)
{
    std::map<std::string, int> mapA = {{"a", 1}, {"b", 2}, {"c", 3}};
    std::map<std::string, int> mapB = {{"b", 10}, {"c", 20}, {"d", 30}};

    auto result = stevensMapLib::addMaps(mapA, mapB, "values", false);

    // Shared keys should be added
    ASSERT_EQ(result["b"], 12);  // 2 + 10
    ASSERT_EQ(result["c"], 23);  // 3 + 20
    // Non-shared keys should be included
    ASSERT_EQ(result["a"], 1);
    ASSERT_EQ(result["d"], 30);
}

TEST(StevensMapLib, addMaps_omit_non_shared)
{
    std::map<std::string, int> mapA = {{"a", 1}, {"b", 2}};
    std::map<std::string, int> mapB = {{"b", 10}, {"c", 20}};

    auto result = stevensMapLib::addMaps(mapA, mapB, "values", true);

    // Only shared keys should appear
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result["b"], 12);
}

TEST(StevensMapLib, combineMaps_no_overwrite)
{
    std::map<std::string, int> source = {{"a", 1}, {"b", 2}};
    std::map<std::string, int> dest = {{"b", 10}, {"c", 20}};

    auto result = stevensMapLib::combineMaps(source, dest, false);

    // Destination value should be kept for shared keys
    ASSERT_EQ(result["b"], 10);
    ASSERT_EQ(result["a"], 1);
    ASSERT_EQ(result["c"], 20);
}

TEST(StevensMapLib, combineMaps_with_overwrite)
{
    std::map<std::string, int> source = {{"a", 1}, {"b", 2}};
    std::map<std::string, int> dest = {{"b", 10}, {"c", 20}};

    auto result = stevensMapLib::combineMaps(source, dest, true);

    // Source value should overwrite for shared keys
    ASSERT_EQ(result["b"], 2);
    ASSERT_EQ(result["a"], 1);
    ASSERT_EQ(result["c"], 20);
}

TEST(StevensMapLib, containsOnly)
{
    std::map<std::string, int> map1 = {{"key1", 100}};
    std::map<std::string, int> map2 = {{"key1", 100}, {"key2", 200}};

    ASSERT_TRUE(stevensMapLib::containsOnly(map1, std::string("key1")));
    ASSERT_FALSE(stevensMapLib::containsOnly(map2, std::string("key1")));
}

TEST(StevensMapLib, multiplyWithValues)
{
    std::map<std::string, int> map = {{"a", 10}, {"b", 20}, {"c", 30}};
    auto result = stevensMapLib::multiplyWithValues(map, 2.5);

    ASSERT_EQ(result["a"], 25);
    ASSERT_EQ(result["b"], 50);
    ASSERT_EQ(result["c"], 75);
}

TEST(StevensMapLib, sumAllValues)
{
    std::map<std::string, int> map = {{"a", 10}, {"b", 20}, {"c", 30}};
    ASSERT_EQ(stevensMapLib::sumAllValues(map), 60);
    ASSERT_EQ(stevensMapLib::sumAllValues(map, 10), 70); // With initial value
}

TEST(StevensMapLib, mapToVector)
{
    std::map<std::string, int> map = {{"a", 1}, {"b", 2}, {"c", 3}};
    auto vec = stevensMapLib::mapToVector(map);

    ASSERT_EQ(vec.size(), 3);
    // Verify first pair (maps are sorted, so "a" should be first)
    ASSERT_EQ(vec[0].first, "a");
    ASSERT_EQ(vec[0].second, 1);
}

TEST(StevensMapLib, getKeyVector)
{
    std::map<std::string, int> map = {{"apple", 1}, {"banana", 2}, {"cherry", 3}};
    auto keys = stevensMapLib::getKeyVector(map);

    ASSERT_EQ(keys.size(), 3);
    ASSERT_TRUE(stevensVectorLib::contains(keys, std::string("apple")));
    ASSERT_TRUE(stevensVectorLib::contains(keys, std::string("banana")));
    ASSERT_TRUE(stevensVectorLib::contains(keys, std::string("cherry")));
}

TEST(StevensMapLib, getValueVector)
{
    std::map<std::string, int> map = {{"a", 10}, {"b", 20}, {"c", 30}};
    auto values = stevensMapLib::getValueVector(map);

    ASSERT_EQ(values.size(), 3);
    ASSERT_TRUE(stevensVectorLib::contains(values, 10));
    ASSERT_TRUE(stevensVectorLib::contains(values, 20));
    ASSERT_TRUE(stevensVectorLib::contains(values, 30));
}

TEST(StevensMapLib, erase_multiple_keys)
{
    std::map<std::string, int> map = {{"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}};
    std::vector<std::string> keysToErase = {"b", "d"};

    auto result = stevensMapLib::erase(map, keysToErase);

    ASSERT_EQ(result.size(), 2);
    ASSERT_TRUE(result.contains("a"));
    ASSERT_TRUE(result.contains("c"));
    ASSERT_FALSE(result.contains("b"));
    ASSERT_FALSE(result.contains("d"));
}

TEST(StevensMapLib, getFirstKey)
{
    std::map<std::string, int> map = {{"zebra", 1}, {"apple", 2}, {"mango", 3}};
    // For std::map, elements are sorted, so first should be "apple"
    ASSERT_EQ(stevensMapLib::getFirstKey(map), "apple");
}

TEST(StevensMapLib, getFirstValue)
{
    std::map<std::string, int> map = {{"zebra", 100}, {"apple", 200}};
    // First in sorted order is "apple", so value should be 200
    ASSERT_EQ(stevensMapLib::getFirstValue(map), 200);
}

TEST(StevensMapLib, getPairWithMaxValue)
{
    std::map<std::string, int> map = {{"a", 10}, {"b", 50}, {"c", 30}};
    auto maxPair = stevensMapLib::getPairWithMaxValue(map);

    ASSERT_EQ(maxPair.first, "b");
    ASSERT_EQ(maxPair.second, 50);
}

TEST(StevensMapLib, getPairWithMinValue)
{
    std::map<std::string, int> map = {{"a", 10}, {"b", 50}, {"c", 30}};
    auto minPair = stevensMapLib::getPairWithMinValue(map);

    ASSERT_EQ(minPair.first, "a");
    ASSERT_EQ(minPair.second, 10);
}

TEST(StevensMapLib, mapToSortedVector_by_value_ascending)
{
    std::unordered_map<std::string, int> map = {{"a", 30}, {"b", 10}, {"c", 20}};
    auto sorted = stevensMapLib::mapToSortedVector(map, "value", "ascending");

    ASSERT_EQ(sorted.size(), 3);
    ASSERT_EQ(sorted[0].second, 10);  // "b"
    ASSERT_EQ(sorted[1].second, 20);  // "c"
    ASSERT_EQ(sorted[2].second, 30);  // "a"
}

TEST(StevensMapLib, mapToSortedVector_by_value_descending)
{
    std::unordered_map<std::string, int> map = {{"a", 30}, {"b", 10}, {"c", 20}};
    auto sorted = stevensMapLib::mapToSortedVector(map, "value", "descending");

    ASSERT_EQ(sorted.size(), 3);
    ASSERT_EQ(sorted[0].second, 30);  // "a"
    ASSERT_EQ(sorted[1].second, 20);  // "c"
    ASSERT_EQ(sorted[2].second, 10);  // "b"
}


/*** stevensStringLib Tests ***/
TEST(StevensStringLib, contains_substring)
{
    ASSERT_TRUE(stevensStringLib::contains("Hello World", "World"));
    ASSERT_TRUE(stevensStringLib::contains("Hello World", "Hello"));
    ASSERT_FALSE(stevensStringLib::contains("Hello World", "Goodbye"));
}

TEST(StevensStringLib, contains_char)
{
    ASSERT_TRUE(stevensStringLib::contains("Hello", 'e'));
    ASSERT_FALSE(stevensStringLib::contains("Hello", 'z'));
}

TEST(StevensStringLib, containsOnly)
{
    ASSERT_TRUE(stevensStringLib::containsOnly("111222111", "12"));
    ASSERT_FALSE(stevensStringLib::containsOnly("111222333", "12"));
    ASSERT_TRUE(stevensStringLib::containsOnly("", ""));
    ASSERT_FALSE(stevensStringLib::containsOnly("abc", ""));
}

TEST(StevensStringLib, eraseCharsFromEnd)
{
    ASSERT_STREQ(stevensStringLib::eraseCharsFromEnd("Hello World", 6).c_str(), "Hello");
    ASSERT_STREQ(stevensStringLib::eraseCharsFromEnd("Test", 2).c_str(), "Te");
    ASSERT_STREQ(stevensStringLib::eraseCharsFromEnd("Test", 10).c_str(), ""); // More than length
}

TEST(StevensStringLib, eraseCharsFromStart)
{
    ASSERT_STREQ(stevensStringLib::eraseCharsFromStart("Hello World", 6).c_str(), "World");
    ASSERT_STREQ(stevensStringLib::eraseCharsFromStart("Test", 2).c_str(), "st");
    ASSERT_STREQ(stevensStringLib::eraseCharsFromStart("Test", 10).c_str(), ""); // More than length
}

TEST(StevensStringLib, startsWith)
{
    ASSERT_TRUE(stevensStringLib::startsWith("Hello World", "Hello"));
    ASSERT_FALSE(stevensStringLib::startsWith("Hello World", "World"));
    ASSERT_FALSE(stevensStringLib::startsWith("Hi", "Hello"));
}

TEST(StevensStringLib, endsWith)
{
    ASSERT_TRUE(stevensStringLib::endsWith("Hello World", "World"));
    ASSERT_FALSE(stevensStringLib::endsWith("Hello World", "Hello"));
    ASSERT_FALSE(stevensStringLib::endsWith("Hi", "World"));
}

TEST(StevensStringLib, findAll_substring)
{
    std::string str = "abcabcabc";
    auto indices = stevensStringLib::findAll(str, "abc");
    ASSERT_EQ(indices.size(), 3);
    ASSERT_EQ(indices[0], 0);
    ASSERT_EQ(indices[1], 3);
    ASSERT_EQ(indices[2], 6);
}

TEST(StevensStringLib, findAll_char)
{
    std::string str = "Hello World";
    auto indices = stevensStringLib::findAll(str, 'l');
    ASSERT_EQ(indices.size(), 3);
    ASSERT_EQ(indices[0], 2);
    ASSERT_EQ(indices[1], 3);
    ASSERT_EQ(indices[2], 9);
}

TEST(StevensStringLib, separate_by_char)
{
    auto result = stevensStringLib::separate("apple,banana,cherry", ',');
    ASSERT_EQ(result.size(), 3);
    ASSERT_EQ(result[0], "apple");
    ASSERT_EQ(result[1], "banana");
    ASSERT_EQ(result[2], "cherry");
}

TEST(StevensStringLib, separate_by_string)
{
    auto result = stevensStringLib::separate("one::two::three", "::");
    ASSERT_EQ(result.size(), 3);
    ASSERT_EQ(result[0], "one");
    ASSERT_EQ(result[1], "two");
    ASSERT_EQ(result[2], "three");
}

TEST(StevensStringLib, separate_omit_empty)
{
    auto result = stevensStringLib::separate("a,,b,,c", ',', true);
    ASSERT_EQ(result.size(), 3);

    auto result2 = stevensStringLib::separate("a,,b,,c", ',', false);
    ASSERT_EQ(result2.size(), 5);
}

TEST(StevensStringLib, join)
{
    std::vector<std::string> vec = {"apple", "banana", "cherry"};
    ASSERT_EQ(stevensStringLib::join(vec, ", "), "apple, banana, cherry");
    ASSERT_EQ(stevensStringLib::join(vec, "::"), "apple::banana::cherry");
}

TEST(StevensStringLib, cap1stChar)
{
    ASSERT_EQ(stevensStringLib::cap1stChar("hello"), "Hello");
    ASSERT_EQ(stevensStringLib::cap1stChar("world"), "World");
    ASSERT_EQ(stevensStringLib::cap1stChar(""), "");
}

TEST(StevensStringLib, toUpper)
{
    ASSERT_EQ(stevensStringLib::toUpper("hello"), "HELLO");
    ASSERT_EQ(stevensStringLib::toUpper("Hello World"), "HELLO WORLD");
}

TEST(StevensStringLib, toLower)
{
    ASSERT_EQ(stevensStringLib::toLower("HELLO"), "hello");
    ASSERT_EQ(stevensStringLib::toLower("Hello World"), "hello world");
}

TEST(StevensStringLib, isInteger)
{
    ASSERT_TRUE(stevensStringLib::isInteger("123"));
    ASSERT_TRUE(stevensStringLib::isInteger("-456"));
    ASSERT_FALSE(stevensStringLib::isInteger("12.5"));
    ASSERT_FALSE(stevensStringLib::isInteger("abc"));
    ASSERT_FALSE(stevensStringLib::isInteger(""));
}

TEST(StevensStringLib, isFloat)
{
    ASSERT_TRUE(stevensStringLib::isFloat("3.14"));
    ASSERT_TRUE(stevensStringLib::isFloat("-2.5"));
    ASSERT_TRUE(stevensStringLib::isFloat("123"));
    ASSERT_FALSE(stevensStringLib::isFloat("abc"));
}

TEST(StevensStringLib, isNumber)
{
    ASSERT_TRUE(stevensStringLib::isNumber("123"));
    ASSERT_TRUE(stevensStringLib::isNumber("3.14"));
    ASSERT_TRUE(stevensStringLib::isNumber("-5.5"));
    ASSERT_FALSE(stevensStringLib::isNumber("abc"));
}

TEST(StevensStringLib, stringToBool)
{
    ASSERT_TRUE(stevensStringLib::stringToBool("true"));
    ASSERT_TRUE(stevensStringLib::stringToBool("True"));
    ASSERT_TRUE(stevensStringLib::stringToBool("1"));
    ASSERT_FALSE(stevensStringLib::stringToBool("false"));
    ASSERT_FALSE(stevensStringLib::stringToBool("0"));
}

TEST(StevensStringLib, boolToString)
{
    ASSERT_EQ(stevensStringLib::boolToString(true), "true");
    ASSERT_EQ(stevensStringLib::boolToString(false), "false");
}

TEST(StevensStringLib, removeWhitespace)
{
    ASSERT_EQ(stevensStringLib::removeWhitespace("Hello World"), "HelloWorld");
    ASSERT_EQ(stevensStringLib::removeWhitespace("  a  b  c  "), "abc");
}

TEST(StevensStringLib, reverse)
{
    ASSERT_EQ(stevensStringLib::reverse("hello"), "olleh");
    ASSERT_EQ(stevensStringLib::reverse("12345"), "54321");
}

TEST(StevensStringLib, isPalindrome)
{
    ASSERT_TRUE(stevensStringLib::isPalindrome("racecar"));
    ASSERT_TRUE(stevensStringLib::isPalindrome("noon"));
    ASSERT_FALSE(stevensStringLib::isPalindrome("hello"));
}

TEST(StevensStringLib, multiply)
{
    ASSERT_EQ(stevensStringLib::multiply("ab", 3), "ababab");
    ASSERT_EQ(stevensStringLib::multiply("x", 5), "xxxxx");
    ASSERT_EQ(stevensStringLib::multiply("test", 0), "");
}

TEST(StevensStringLib, replaceSubstr)
{
    ASSERT_EQ(stevensStringLib::replaceSubstr("hello world", "world", "there"), "hello there");
    ASSERT_EQ(stevensStringLib::replaceSubstr("aaa", "a", "b"), "bbb");
}

TEST(StevensStringLib, replaceSubstr_quantity)
{
    ASSERT_EQ(stevensStringLib::replaceSubstr("aaa", "a", "b", 2), "bba");
}

TEST(StevensStringLib, scramble)
{
    stevensMathLib::seedRNG();
    std::string original = "abcdef";
    std::string scrambled = stevensStringLib::scramble(original);

    // Should have same length
    ASSERT_EQ(scrambled.length(), original.length());
    // Should contain same characters (different order likely, but not guaranteed)
    for(char c : original) {
        ASSERT_TRUE(stevensStringLib::contains(scrambled, c));
    }
}


/*** stevensFileLib Tests ***/
TEST(StevensFileLib, appendToFile_creates_new_file)
{
    std::string testFilePath = "/tmp/test_stevens_file_lib_new.txt";

    // Remove file if it exists
    std::remove(testFilePath.c_str());

    // Append to non-existent file (should create it)
    stevensFileLib::appendToFile(testFilePath, "Test content\n", true);

    // Verify file exists and has content
    std::ifstream file(testFilePath);
    ASSERT_TRUE(file.is_open());
    std::string line;
    std::getline(file, line);
    ASSERT_EQ(line, "Test content");
    file.close();

    // Cleanup
    std::remove(testFilePath.c_str());
}

TEST(StevensFileLib, appendToFile_appends_to_existing)
{
    std::string testFilePath = "/tmp/test_stevens_file_lib_append.txt";

    // Create initial file
    std::ofstream initialFile(testFilePath);
    initialFile << "Line 1\n";
    initialFile.close();

    // Append to it
    stevensFileLib::appendToFile(testFilePath, "Line 2\n");

    // Verify both lines exist
    auto lines = stevensFileLib::loadFileIntoVector(testFilePath);
    ASSERT_EQ(lines.size(), 2);
    ASSERT_EQ(lines[0], "Line 1");
    ASSERT_EQ(lines[1], "Line 2");

    // Cleanup
    std::remove(testFilePath.c_str());
}

TEST(StevensFileLib, loadFileIntoVector)
{
    std::string testFilePath = "/tmp/test_stevens_file_lib_load.txt";

    // Create test file
    std::ofstream testFile(testFilePath);
    testFile << "Line 1\n";
    testFile << "Line 2\n";
    testFile << "Line 3\n";
    testFile.close();

    // Load into vector
    auto lines = stevensFileLib::loadFileIntoVector(testFilePath);

    ASSERT_EQ(lines.size(), 3);
    ASSERT_EQ(lines[0], "Line 1");
    ASSERT_EQ(lines[1], "Line 2");
    ASSERT_EQ(lines[2], "Line 3");

    // Cleanup
    std::remove(testFilePath.c_str());
}

TEST(StevensFileLib, loadFileIntoVector_skip_empty_lines)
{
    std::string testFilePath = "/tmp/test_stevens_file_lib_skip.txt";

    // Create test file with empty lines
    std::ofstream testFile(testFilePath);
    testFile << "Line 1\n";
    testFile << "\n";
    testFile << "Line 2\n";
    testFile << "\n";
    testFile.close();

    // Load with skip empty lines (default)
    auto linesSkip = stevensFileLib::loadFileIntoVector(testFilePath, {}, '\n', true);
    ASSERT_EQ(linesSkip.size(), 2);

    // Load without skip
    auto linesNoSkip = stevensFileLib::loadFileIntoVector(testFilePath, {}, '\n', false);
    ASSERT_EQ(linesNoSkip.size(), 4);

    // Cleanup
    std::remove(testFilePath.c_str());
}

TEST(StevensFileLib, loadFileIntoVector_skip_if_starts_with)
{
    std::string testFilePath = "/tmp/test_stevens_file_lib_starts.txt";

    // Create test file
    std::ofstream testFile(testFilePath);
    testFile << "# Comment 1\n";
    testFile << "Data line 1\n";
    testFile << "# Comment 2\n";
    testFile << "Data line 2\n";
    testFile.close();

    // Load skipping lines starting with #
    std::unordered_map<std::string, std::vector<std::string>> settings;
    settings["skip if starts with"] = {"#"};
    auto lines = stevensFileLib::loadFileIntoVector(testFilePath, settings);

    ASSERT_EQ(lines.size(), 2);
    ASSERT_EQ(lines[0], "Data line 1");
    ASSERT_EQ(lines[1], "Data line 2");

    // Cleanup
    std::remove(testFilePath.c_str());
}

TEST(StevensFileLib, loadFileIntoVectorOfInts)
{
    std::string testFilePath = "/tmp/test_stevens_file_lib_ints.txt";

    // Create test file with integers
    std::ofstream testFile(testFilePath);
    testFile << "10\n";
    testFile << "20\n";
    testFile << "30\n";
    testFile.close();

    // Load into vector of ints
    auto ints = stevensFileLib::loadFileIntoVectorOfInts(testFilePath);

    ASSERT_EQ(ints.size(), 3);
    ASSERT_EQ(ints[0], 10);
    ASSERT_EQ(ints[1], 20);
    ASSERT_EQ(ints[2], 30);

    // Cleanup
    std::remove(testFilePath.c_str());
}

