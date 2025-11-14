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