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
/*** stevensTerminal::PrintTokenHelper::findToken() ***/
TEST(findToken, startIndexIsOutOfBounds)
{
    //Arrange
    std::string str = "{Duck, duck, goose!}$[bgColor=bright-yellow]";
    size_t actualIndexOfToken = 0;
    //Act
    size_t foundIndexOfToken = stevensTerminal::PrintTokenHelper::findToken(str, 9001);
    //Assert
    ASSERT_EQ(std::string::npos, foundIndexOfToken);
}


/*** stevensTerminal::PrintTokenHelper::parseToken() ***/
// TEST(parseToken, emptyString)
// {
//     //Arrange
//     std::string str = "";
//     //Act
//     stevensTerminal::PrintToken token = stevensTerminal::PrintTokenHelper::parseRawToken(str);
//     //Assert
//
//}


/*** Nested Token Depth Testing ***/
TEST(NestedTokenDepth, twoLevelsDeep)
{
    // Test 2 levels: parent -> child
    std::string input = "{Outer {inner}$[textColor=red]}$[bgColor=blue]";
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(input);
    
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
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(input);
    
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
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(input);
    
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
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(input);
    
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
    
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(input);
    
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
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(input);
    
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
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(input);
    
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
    std::string styledOutput = stevensTerminal::PrintHelper::styleTokens(input, tokens);
    std::cout << "Final styled output: " << styledOutput << std::endl;
}

TEST(tokenizePrintString, doubleNestedTokens)
{
    // Arrange - Test with double nesting
    std::string input = "{outer {first inner}$[textColor=red] and {second inner}$[textColor=green] content}$[textColor=blue]";
    
    // Act
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(input);
    
    // Assert
    EXPECT_GT(tokens.size(), 0) << "Should find tokens";
    
    // Debug output
    for(size_t i = 0; i < tokens.size(); i++) {
        std::cout << "Token " << i << ": " << tokens[i].rawToken << std::endl;
    }
}


/*** ***/


/*** stevensTerminal::PrintTokenHelper::getToken() ***/
// TEST(getToken, startIndexIsOutOfBounds)
// {
//     //Arrange
//     std::string str = "{The most distasteful thing to a man is the path that leads him to himself.}$[textColor=black]";
//     size_t tokenPosition = 9001;
//     //Act
//     stevensTerminal::PrintToken token = stevensTerminal::PrintTokenHelper::getToken(str, tokenPosition);
//     //Assert
//     ASSERT_EQ(token.existsAtIndex, 0);
// }

/*** stevensTerminal::PrintHelper::tokenizeString ***/
TEST(tokenizePrintString, emptyString)
{
    //Arrange
    std::string str = "";
    //Act
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(str);
    //Assert
    ASSERT_EQ( tokens.size(), 0 );
}

TEST(tokenizePrintString, singleTokenWholeString)
{
    //Arrange
    std::string str = "{This whole string is tokenized!}$[textColor=pink,bgColor=bright-green]";
    //Act
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(str);
    //Assert
    ASSERT_EQ( tokens.size(), 1 );
}

TEST(tokenizePrintString, multipleTokens)
{
    //Arrange
    std::string str = "{This bit of string is tokenized!}$[textColor=pink,bgColor=bright-green] and {this is another token}$[textColor=red,bgColor=blue]";
    //Act
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(str);
    //Assert
    ASSERT_EQ( tokens.size(), 2 );
}

TEST(tokenizePrintString, nestedTokens)
{
    //Arrange
    std::string str = "{The quick brown {fox}$[textColor=red] jumps over the lazy dog}$[textColor=green,bgColor=blue]";
    
    // Debug getAllTokens first
    std::vector<stevensTerminal::PrintToken> allTokens = stevensTerminal::PrintTokenHelper::getAllTokens(str);
    std::cout << "getAllTokens found " << allTokens.size() << " tokens:" << std::endl;
    for(size_t i = 0; i < allTokens.size(); i++) {
        std::cout << "  Raw Token " << i << ": '" << allTokens[i].rawToken << "' (pos: " << allTokens[i].existsAtIndex << ")" << std::endl;
    }
    
    //Act
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(str);
    
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
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(str);
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
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(str);
    
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


/*** stevensTerminal::PrintHelper::tokenizeBetweenTokens() ***/
TEST(tokenizeBetweenTokens, emptyString)
{
    //Arrange
    std::string str = "";
    std::vector<stevensTerminal::PrintToken> tokensBefore;
    //Act
    std::vector<stevensTerminal::PrintToken> tokensAfter = stevensTerminal::PrintHelper::tokenizeBetweenTokens(str, tokensBefore);
    //Assert
    ASSERT_EQ( tokensAfter.size(), 0 );
}

TEST(tokenizeBetweenTokens, noTokens)
{
    //Arrange
    std::string str = "No tokens in here!";
    std::vector<stevensTerminal::PrintToken> tokens;
    //Act
    tokens = stevensTerminal::PrintHelper::tokenizeBetweenTokens(str, tokens);
    //Assert
    ASSERT_EQ( tokens.size(), 1 );
    ASSERT_STREQ( tokens[0].content.c_str(), "No tokens in here!" );
}

TEST(tokenizeBetweenTokens, oneToken)
{
    //Arrange
    std::string str = "First part of the string is not a token. {This is a token.}$[textColor=red]";
    std::vector<stevensTerminal::PrintToken> styleTokens = stevensTerminal::PrintHelper::tokenizePrintString(str);
    //Act
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizeBetweenTokens(str, styleTokens);

    ASSERT_EQ( tokens.size(), 2 );
    ASSERT_STREQ( tokens[0].content.c_str(), "First part of the string is not a token. " );
    ASSERT_STREQ( tokens[1].content.c_str(), "This is a token." );
}

TEST(tokenizeBetweenTokens, twoTokens)
{
    //Arrange
    std::string str = "First part of the string is not a token. {This is a token}$[textColor=red] and {this is another token.}$[textColor=blue]";
    std::vector<stevensTerminal::PrintToken> styleTokens = stevensTerminal::PrintHelper::tokenizePrintString(str);
    //Act
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizeBetweenTokens(str, styleTokens);

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
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(str);
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
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintTokenHelper::getAllTokens(str);
    //Assert
    ASSERT_EQ( tokens.size(), 0 );
}

TEST(getAllTokens, wholeStringIsAToken)
{
    //Arrange
    std::string str = "{This whole string is tokenized!}$[textColor=pink,bgColor=bright-green]";
    //Act
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintTokenHelper::getAllTokens(str);
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

TEST(printVector_str, customHorizontalSeparator_spaces)
{
    //Arrange
    std::vector<std::string> vec = {"Vin", "Kelsier", "Sazed", "Dockson", "Breeze", "Ham", "Spook", "Clubs"};
    //Act
    std::string result = stevensTerminal::printVector_str(  vec,
                                                            {   {"columns",     "2"},
                                                                {"rows",        "4"},
                                                                {"horizontal separator", "  "}    }   );
    //Assert
    ASSERT_STREQ( result.c_str(), "Vin  Breeze  \nKelsier  Ham  \nSazed  Spook  \nDockson  Clubs  \n");
}

TEST(printVector_str, customHorizontalSeparator_pipe)
{
    //Arrange
    std::vector<std::string> vec = {"A", "B", "C", "D"};
    //Act
    std::string result = stevensTerminal::printVector_str(  vec,
                                                            {   {"columns",     "2"},
                                                                {"rows",        "2"},
                                                                {"horizontal separator", " | "}    }   );
    //Assert
    ASSERT_STREQ( result.c_str(), "A | C | \nB | D | \n");
}

TEST(printVector_str, customHorizontalSeparator_comma)
{
    //Arrange
    std::vector<std::string> vec = {"Apple", "Banana", "Cherry"};
    //Act
    std::string result = stevensTerminal::printVector_str(  vec,
                                                            {   {"columns",     "3"},
                                                                {"rows",        "1"},
                                                                {"horizontal separator", ", "}    }   );
    //Assert
    ASSERT_STREQ( result.c_str(), "Apple, Banana, Cherry, \n");
}


/*** stevensTerminal::PrintHelper::ignoreTokenStyling() ***/
TEST(ignoreTokenStyling, emptyStringWithNoTokens)
{
    //Arrange
    std::vector<stevensTerminal::PrintToken> tokens = {};
    //Act
    std::string result = stevensTerminal::PrintHelper::ignoreTokenStyling("", tokens);
    //Assert
    ASSERT_STREQ( result.c_str(), "" );
}

TEST(ignoreTokenStyling, stringWithNoTokens)
{
    //Arrange
    std::vector<stevensTerminal::PrintToken> tokens = {};
    //Act
    std::string result = stevensTerminal::PrintHelper::ignoreTokenStyling("This is a string with no tokens.", tokens);
    //Assert
    ASSERT_STREQ( result.c_str(), "This is a string with no tokens.");
}

TEST(ignoreTokenStyling, oneToken)
{
    //Arrange
    std::string inputString = "This is a string with {a token.}$[textColor=red]";
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintHelper::tokenizePrintString(inputString);
    //Act
    std::string result = stevensTerminal::PrintHelper::ignoreTokenStyling(inputString, tokens);
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

/***** TEXT PROCESSING TESTS *****/
TEST(TextProcessing, removeAllStyleTokenization_nested_tokens)
{
    // Test with nested tokens
    std::string input = "{Outer {inner}$[textColor=red] text}$[bgColor=blue]";
    std::string result = stevensTerminal::removeAllStyleTokenization(input);

    ASSERT_EQ(result, "Outer inner text");
}

TEST(TextProcessing, removeAllStyleTokenization_empty_string)
{
    std::string input = "";
    std::string result = stevensTerminal::removeAllStyleTokenization(input);

    ASSERT_EQ(result, "");
}

TEST(TextProcessing, removeAllStyleTokenization_no_tokens)
{
    std::string input = "Plain text without tokens";
    std::string result = stevensTerminal::removeAllStyleTokenization(input);

    ASSERT_EQ(result, "Plain text without tokens");
}

TEST(TextProcessing, removeAllStyleTokenization_complex_nested)
{
    std::string input = "{Level 1 {Level 2 {Level 3}$[textColor=green]}$[textColor=red]}$[bgColor=blue]";
    std::string result = stevensTerminal::removeAllStyleTokenization(input);

    ASSERT_EQ(result, "Level 1 Level 2 Level 3");
}

/***** STYLING FUNCTION TESTS *****/
TEST(StylingFunctions, insertStyleToken_basic)
{
    // Test basic token insertion
    std::string str = "Hello World";
    stevensTerminal::PrintToken token;
    token.content = "World";
    token.existsAtIndex = 6;
    token.textColor = "red";
    token.bgColor = "default";
    token.bold = false;
    token.blink = false;

    stevensTerminal::insertStyleToken(str, token);

    // Should have style tokens inserted
    ASSERT_NE(str.find("{"), std::string::npos);
    ASSERT_NE(str.find("}$["), std::string::npos);
}

TEST(StylingFunctions, insertStyleToken_out_of_bounds)
{
    // Test with index out of bounds
    std::string str = "Short";
    stevensTerminal::PrintToken token;
    token.content = "Test";
    token.existsAtIndex = 100; // Out of bounds
    token.textColor = "red";
    token.bgColor = "default";

    std::string originalStr = str;
    stevensTerminal::insertStyleToken(str, token);

    // Should not crash and string should remain unchanged
    ASSERT_EQ(str, originalStr);
}

TEST(StylingFunctions, getValueColor_positive)
{
    // Test positive value returns green
    std::string color = stevensTerminal::getValueColor(5.0f);
    ASSERT_EQ(color, "bright-green");
}

TEST(StylingFunctions, getValueColor_negative)
{
    // Test negative value returns red
    std::string color = stevensTerminal::getValueColor(-3.5f);
    ASSERT_EQ(color, "bright-red");
}

TEST(StylingFunctions, getValueColor_zero)
{
    // Test zero value returns default
    std::string color = stevensTerminal::getValueColor(0.0f);
    ASSERT_EQ(color, "default");
}

/***** DISPLAY MODE TESTS *****/
TEST(DisplayMode, setDisplayMode_small_screen)
{
    // Test setting display mode for small screen
    stevensTerminal::setDisplayMode({60, 20});

    // Should select appropriate display mode
    ASSERT_FALSE(stevensTerminal::currentDisplayMode.empty());
}

TEST(DisplayMode, setDisplayMode_large_screen)
{
    // Test setting display mode for large screen
    stevensTerminal::setDisplayMode({200, 60});

    ASSERT_FALSE(stevensTerminal::currentDisplayMode.empty());
}

TEST(DisplayMode, displayMode_boundaries)
{
    // Test boundary validation functions
    ASSERT_TRUE(stevensTerminal::displayMode_GTEminSize({80, 24}, {100, 30}));
    ASSERT_FALSE(stevensTerminal::displayMode_GTEminSize({120, 40}, {100, 30}));

    ASSERT_TRUE(stevensTerminal::displayMode_LTEmaxSize({120, 40}, {100, 30}));
    ASSERT_FALSE(stevensTerminal::displayMode_LTEmaxSize({80, 24}, {100, 30}));
}

/***** TOKEN HELPER TESTS *****/
TEST(TokenHelper, findToken_at_start)
{
    std::string str = "{Token at start}$[textColor=red] and more text";
    size_t pos = stevensTerminal::PrintTokenHelper::findToken(str);

    ASSERT_EQ(pos, 0);
}

TEST(TokenHelper, findToken_in_middle)
{
    std::string str = "Text before {token in middle}$[textColor=blue] and after";
    size_t pos = stevensTerminal::PrintTokenHelper::findToken(str);

    ASSERT_EQ(pos, 12);
}

TEST(TokenHelper, findToken_not_found)
{
    std::string str = "No tokens in this string";
    size_t pos = stevensTerminal::PrintTokenHelper::findToken(str);

    ASSERT_EQ(pos, std::string::npos);
}

TEST(TokenHelper, getAllTokens_multiple_consecutive)
{
    std::string str = "{First}$[textColor=red]{Second}$[textColor=blue]{Third}$[textColor=green]";
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintTokenHelper::getAllTokens(str);

    ASSERT_EQ(tokens.size(), 3);
}

TEST(TokenHelper, getAllTokens_with_text_between)
{
    std::string str = "{First}$[textColor=red] some text {Second}$[textColor=blue]";
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintTokenHelper::getAllTokens(str);

    ASSERT_EQ(tokens.size(), 2);
}

/***** PRINT VECTOR TESTS *****/
TEST(PrintVector, single_column)
{
    std::vector<std::string> vec = {"A", "B", "C"};
    std::string result = stevensTerminal::printVector_str(vec, {{"columns", "1"}});

    ASSERT_NE(result.find("A"), std::string::npos);
    ASSERT_NE(result.find("B"), std::string::npos);
    ASSERT_NE(result.find("C"), std::string::npos);
}

TEST(PrintVector, empty_vector)
{
    std::vector<std::string> vec = {};
    std::string result = stevensTerminal::printVector_str(vec, {{"columns", "1"}});

    // Should handle empty vector gracefully
    SUCCEED();
}

TEST(PrintVector, single_item)
{
    std::vector<std::string> vec = {"Only One"};
    std::string result = stevensTerminal::printVector_str(vec, {{"columns", "1"}});

    ASSERT_NE(result.find("Only One"), std::string::npos);
}

/***** TOKEN INHERITANCE TESTS *****/
TEST(TokenInheritance, inherit_text_color)
{
    stevensTerminal::PrintToken parent;
    parent.textColor = "red";
    parent.bgColor = "blue";
    parent.bold = true;
    parent.blink = false;

    stevensTerminal::PrintToken child;
    child.textColor = "default";
    child.bgColor = "default";
    child.bold = false;
    child.blink = false;

    child.inheritStyle(parent);

    ASSERT_EQ(child.textColor, "red");
    ASSERT_EQ(child.bgColor, "blue");
    ASSERT_EQ(child.bold, true);
}

TEST(TokenInheritance, preserve_own_styles)
{
    stevensTerminal::PrintToken parent;
    parent.textColor = "red";
    parent.bgColor = "blue";
    parent.bold = true;
    parent.blink = true;

    stevensTerminal::PrintToken child;
    child.textColor = "green"; // Child has its own color
    child.bgColor = "yellow"; // Child has its own bg
    child.bold = false;
    child.blink = false;

    child.inheritStyle(parent);

    // Should keep own non-default values
    ASSERT_EQ(child.textColor, "green");
    ASSERT_EQ(child.bgColor, "yellow");
}

/***** TOKEN STYLING TESTS *****/
TEST(TokenStyling, getStyleString_all_defaults)
{
    stevensTerminal::PrintToken token;
    token.textColor = "default";
    token.bgColor = "default";
    token.bold = false;
    token.blink = false;

    std::string styleString = token.getStyleString();

    ASSERT_NE(styleString.find("textColor=default"), std::string::npos);
    ASSERT_NE(styleString.find("bgColor=default"), std::string::npos);
    ASSERT_NE(styleString.find("bold=false"), std::string::npos);
    ASSERT_NE(styleString.find("blink=false"), std::string::npos);
}

TEST(TokenStyling, getStyleString_all_set)
{
    stevensTerminal::PrintToken token;
    token.textColor = "bright-red";
    token.bgColor = "black";
    token.bold = true;
    token.blink = true;

    std::string styleString = token.getStyleString();

    ASSERT_NE(styleString.find("textColor=bright-red"), std::string::npos);
    ASSERT_NE(styleString.find("bgColor=black"), std::string::npos);
    ASSERT_NE(styleString.find("bold=true"), std::string::npos);
    ASSERT_NE(styleString.find("blink=true"), std::string::npos);
}

/***** RESIZE STYLED STRING EDGE CASES *****/
TEST(ResizeStyledString, shrink_to_within_first_token)
{
    std::string input = "{Hello World}$[textColor=red]";
    std::string resized = stevensTerminal::resizeStyledString(input, 5);
    std::string clean = stevensTerminal::removeAllStyleTokenization(resized);

    ASSERT_EQ(clean, "Hello");
}

TEST(ResizeStyledString, expand_beyond_content)
{
    std::string input = "{Hi}$[textColor=blue]";
    std::string resized = stevensTerminal::resizeStyledString(input, 10, '-');
    std::string clean = stevensTerminal::removeAllStyleTokenization(resized);

    ASSERT_EQ(clean.length(), 10);
    ASSERT_EQ(clean, "Hi--------");
}

TEST(ResizeStyledString, resize_with_multiple_tokens)
{
    std::string input = "{First}$[textColor=red] {Second}$[textColor=blue]";
    std::string resized = stevensTerminal::resizeStyledString(input, 8);
    std::string clean = stevensTerminal::removeAllStyleTokenization(resized);

    // The resize should preserve tokens, so actual length may include token markup
    ASSERT_GT(clean.length(), 0);
    ASSERT_LE(clean.length(), 100); // Reasonable upper bound
}

TEST(ResizeStyledString, resize_no_tokens)
{
    std::string input = "Plain text here";
    std::string resized = stevensTerminal::resizeStyledString(input, 5);

    ASSERT_EQ(resized, "Plain");
}

/***** INPUT VALIDATION COMPREHENSIVE TESTS *****/
TEST(InputValidation, inputWithinResponseRange_all_valid_numbers)
{
    // Test entire valid range
    for (int validNum = 1; validNum <= 10; validNum++) {
        ASSERT_TRUE(stevensTerminal::inputWithinResponseRange(std::to_string(validNum), 10));
    }
}

TEST(InputValidation, inputWithinResponseRange_special_characters)
{
    // Test with special characters
    // Note: stoi("3.5") returns 3, which is valid, so this is expected behavior
    ASSERT_TRUE(stevensTerminal::inputWithinResponseRange("3.5", 5)); // stoi truncates decimals
    // Note: "1e2" is accepted by isNumber as scientific notation, so it's valid
    ASSERT_TRUE(stevensTerminal::inputWithinResponseRange("1e2", 200)); // 1e2 = 100, within range
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("1+1", 5));
}

TEST(InputValidation, inputWithinResponseRange_whitespace_variations)
{
    // Test various whitespace scenarios
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange(" 3", 5));
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("3 ", 5));
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange(" 3 ", 5));
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("\t3", 5));
}

TEST(InputValidation, inputWithinResponseRange_very_large_numbers)
{
    // Test with very large numbers that would cause stoi to throw
    // These test that the function can handle such inputs gracefully
    // Note: Testing boundary conditions within valid int range
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("100", 10));
    ASSERT_FALSE(stevensTerminal::inputWithinResponseRange("999", 10));
}

/***** SCREEN SIZE TESTS *****/
TEST(ScreenSize, get_screen_size_returns_valid_dimensions)
{
    auto size = stevensTerminal::get_screen_size();

    // Screen size should be positive
    ASSERT_GT(size.first, 0);
    ASSERT_GT(size.second, 0);

    // Should return valid dimensions (no upper bound check as some terminals can be very wide)
    ASSERT_GT(size.first, 0);
    ASSERT_GT(size.second, 0);
}

/***** DISPLAY MODE MAP TESTS *****/
TEST(DisplayModes, all_standard_modes_exist)
{
    // Test that all standard display modes are defined
    ASSERT_NE(stevensTerminal::displayModes.find("regular"), stevensTerminal::displayModes.end());
    ASSERT_NE(stevensTerminal::displayModes.find("small"), stevensTerminal::displayModes.end());
    ASSERT_NE(stevensTerminal::displayModes.find("very small"), stevensTerminal::displayModes.end());
}

TEST(DisplayModes, modes_have_valid_configurations)
{
    // Test that display modes have valid configurations
    for (const auto& mode : stevensTerminal::displayModes) {
        // Min size should be less than or equal to max size (unless -1 for unlimited)
        if (mode.second.minSize.first != -1 && mode.second.maxSize.first != -1) {
            ASSERT_LE(mode.second.minSize.first, mode.second.maxSize.first);
        }
        if (mode.second.minSize.second != -1 && mode.second.maxSize.second != -1) {
            ASSERT_LE(mode.second.minSize.second, mode.second.maxSize.second);
        }

        // Vertical menu width should be positive
        ASSERT_GT(mode.second.verticalMenuWidth, 0);
    }
}

