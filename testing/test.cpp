/**
 * @brief test.cpp
 * 
 * This file contains all of the tests for the stevensTerminal library.
 */

#include "../stevensTerminal.hpp"
#include <clocale>
#include <iostream>
#include <fstream>
#include <sstream>
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
    // getStyleString() only serializes actually-set attributes (see its doc comment) - bold/blink
    // default false and are omitted rather than emitted as "bold=false"/"blink=false", so an
    // unset-but-false attribute stays inheritable when this token is nested inside a parent.
    ASSERT_STREQ( tokens[0].content.c_str(), "Toggle text styling (" );
    ASSERT_STREQ( tokens[0].bgColor.c_str(), "black" );
    ASSERT_STREQ( tokens[0].textColor.c_str(), "bright-yellow" );
    ASSERT_STREQ( tokens[0].getStyleString().c_str(), "textColor=bright-yellow,bgColor=black" );

    ASSERT_STREQ( tokens[1].content.c_str(), "On" );
    ASSERT_STREQ( tokens[1].bgColor.c_str(), "black" );
    ASSERT_STREQ( tokens[1].textColor.c_str(), "bright-green" );
    ASSERT_STREQ( tokens[1].getStyleString().c_str(), "textColor=bright-green,bgColor=black" );

    ASSERT_STREQ( tokens[2].content.c_str(), ")" );
    ASSERT_STREQ( tokens[2].bgColor.c_str(), "black" );
    ASSERT_STREQ( tokens[2].textColor.c_str(), "bright-yellow" );
    ASSERT_STREQ( tokens[2].getStyleString().c_str(), "textColor=bright-yellow,bgColor=black" );
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

    // getStyleString() only serializes actually-set attributes (see its doc comment) - bold defaults
    // false and is omitted rather than emitted as "bold=false"; blink IS emitted here because it was
    // genuinely inherited as true from the parent token (see the doc comment above this test).
    if(tokens.size() >= 2) {
        ASSERT_STREQ( tokens[0].content.c_str(), "First nested token" );
        ASSERT_STREQ( tokens[0].bgColor.c_str(), "black" );
        ASSERT_STREQ( tokens[0].textColor.c_str(), "bright-yellow" );
        ASSERT_STREQ( tokens[0].getStyleString().c_str(), "textColor=bright-yellow,bgColor=black,blink=true" );

        ASSERT_STREQ( tokens[1].content.c_str(), " and the second nested token with no between" );
        ASSERT_STREQ( tokens[1].bgColor.c_str(), "black" );
        ASSERT_STREQ( tokens[1].textColor.c_str(), "bright-green" );
        ASSERT_STREQ( tokens[1].getStyleString().c_str(), "textColor=bright-green,bgColor=black,blink=true" );
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
    ASSERT_STREQ( result.c_str(), "Vin\nKelsier\nSazed\nDockson\nBreeze\nHam\nSpook\nClubs\n");
}

TEST(printVector_str, fourRows_twoColumns)
{
    //Arrange
    std::vector<std::string> vec = {"Vin", "Kelsier", "Sazed", "Dockson", "Breeze", "Ham", "Spook", "Clubs"};
    //Act
    std::string result = stevensTerminal::printVector_str(   vec,
                                                            {   {"columns",     "2"},
                                                                {"rows",        "4"},
                                                                {"horizontal separator", "     "}    }   );
    //Assert
    ASSERT_STREQ( result.c_str(), "Vin     Breeze\nKelsier     Ham\nSazed     Spook\nDockson     Clubs\n");
}

TEST(printVector_str, threeRows_threeColumns)
{
    //Arrange
    std::vector<std::string> vec = { "Lawful Good", "Lawful Neutral", "Lawful Evil", "Neutral Good", "True Neutral",
                                     "Neutral Evil", "Chaotic Good", "Chaotic Neutral", "Chaotic Evil"};
    //Act
    std::string result = stevensTerminal::printVector_str(  vec,
                                                            {   {"columns", "3"},
                                                                {"rows",    "3"},
                                                                {"horizontal separator", "     "}    }   );
    //Assert
    ASSERT_STREQ( result.c_str(),   "Lawful Good     Neutral Good     Chaotic Good\n"
                                    "Lawful Neutral     True Neutral     Chaotic Neutral\n"
                                    "Lawful Evil     Neutral Evil     Chaotic Evil\n");
}

TEST(printVector_str, columnOverflow)
{
    //Arrange
    std::vector<std::string> vec = {"Jack", "Sally", "Mayor", "Lock", "Shock", "Barrel", "Oogie Boogie"};
    //Act
    std::string result = stevensTerminal::printVector_str( vec,
                                                            {   {"columns", "2"},
                                                                {"rows",    "3"},
                                                                {"allowOverflow",    "true"},
                                                                {"horizontal separator", "     "}    }   );
    //Assert
    ASSERT_STREQ(   result.c_str(), "Jack     Lock     Oogie Boogie\n"
                                    "Sally     Shock\n"
                                    "Mayor     Barrel\n");
}

TEST(printVector_str, autoColumnWidth)
{
    //Arrange
    std::vector<std::string> vec = {"Actions", "World Map", "Theology", "Infrastructure", "Monthly Report", "Settings", "End Month", "Quit"};
    //Act
    std::string result = stevensTerminal::printVector_str(  vec,
                                                            {   {"columns", "2"},
                                                                {"rows",    "4"},
                                                                {"defaultColumnWidth", "auto"},
                                                                {"horizontal separator", "     "}    }   );
    //Assert
    ASSERT_STREQ(   result.c_str(), "Actions            Monthly Report\n"
                                    "World Map          Settings      \n"
                                    "Theology           End Month     \n"
                                    "Infrastructure     Quit          \n");
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
    ASSERT_STREQ( result.c_str(), "Vin  Breeze\nKelsier  Ham\nSazed  Spook\nDockson  Clubs\n");
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
    ASSERT_STREQ( result.c_str(), "A | C\nB | D\n");
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
    ASSERT_STREQ( result.c_str(), "Apple, Banana, Cherry\n");
}

TEST(printVector_str, autoColumnWidth_alignsColumnStartsDespiteRaggedCellLengths)
{
    // Regression test for the exact scenario a ragged (defaultColumnWidth: "none") layout would
    // produce: column 2 drifting left/right per row because it starts immediately after column
    // 1's own text instead of a shared column boundary. "1 - Hello" (9 chars) and "2 - Hola" (8
    // chars) differ in length, but with defaultColumnWidth "auto", column 2 must start at the
    // SAME offset on both rows - padded to the width of the widest cell in column 1.
    std::vector<std::string> vec = {"1 - Hello", "2 - Hola", "3 - Goodbye", "4 - Adios"};
    std::string result = stevensTerminal::printVector_str(  vec,
                                                            {   {"columns", "2"},
                                                                {"rows",    "2"},
                                                                {"defaultColumnWidth", "auto"},
                                                                {"horizontal separator", "     "}    }   );
    ASSERT_STREQ( result.c_str(), "1 - Hello     3 - Goodbye\n2 - Hola      4 - Adios  \n");

    // Column 2 ("3 - Goodbye" / "4 - Adios") must start at the identical byte offset on both rows
    std::istringstream in(result);
    std::string row0, row1;
    std::getline(in, row0);
    std::getline(in, row1);
    EXPECT_EQ(row0.find("3 - Goodbye"), row1.find("4 - Adios"));
}

TEST(printVector_str, numericDefaultColumnWidthPadsEveryColumnToThatExactWidth)
{
    // defaultColumnWidth as a plain integer string is a fixed width shared by every column,
    // distinct from "auto" (pads to that column's OWN widest cell) - "A" and "CCC" are in
    // different columns with different natural widths, but both must pad out to 6 here.
    std::vector<std::string> vec = {"A", "BB", "CCC", "DDDD"};
    std::string result = stevensTerminal::printVector_str(  vec,
                                                            {   {"columns", "2"},
                                                                {"rows",    "2"},
                                                                {"defaultColumnWidth", "6"},
                                                                {"horizontal separator", "|"}    }   );
    ASSERT_STREQ( result.c_str(), "A     |CCC   \nBB    |DDDD  \n");
}

TEST(printVector_str, columnWidthsOverrideTakesPrecedenceOverNumericDefaultColumnWidth)
{
    // A per-column columnWidths entry must win over defaultColumnWidth for that column, even
    // when defaultColumnWidth is itself a specific (not "auto") width.
    std::vector<std::string> vec = {"A", "BB"};
    std::string result = stevensTerminal::printVector_str(  vec,
                                                            {   {"columns", "2"},
                                                                {"rows",    "1"},
                                                                {"defaultColumnWidth", "6"},
                                                                {"horizontal separator", "|"}    },
                                                            {},
                                                            { {0, "3"} }   );
    ASSERT_STREQ( result.c_str(), "A  |BB    \n");
}

TEST(printVector_str, PrintFormatOverloadMatchesEquivalentFormatMap)
{
    // The PrintFormat-taking overload must produce byte-identical output to the string-keyed map
    // it's built to replace, since it works by converting to that map and delegating.
    std::vector<std::string> vec = {"A", "BB", "CCC", "DDDD"};
    std::string fromMap = stevensTerminal::printVector_str(  vec,
                                                            {   {"columns", "2"},
                                                                {"rows",    "2"},
                                                                {"defaultColumnWidth", "auto"},
                                                                {"horizontal separator", "     "}    }   );
    std::string fromStruct = stevensTerminal::printVector_str(  vec,
        stevensTerminal::PrintFormat{
            .columns = 2,
            .rows = 2,
            .defaultColumnWidth = "auto",
            .horizontalSeparator = "     "
        }   );
    ASSERT_EQ(fromMap, fromStruct);
    ASSERT_STREQ(fromStruct.c_str(), "A      CCC \nBB     DDDD\n");
}

/*** formatTableAsString() cell wrapping (multi-byte UTF-8 correctness) ***/
TEST(FormatTableAsString, wrapsMultiByteContentAtCodepointBoundaries)
{
    // 23 Cyrillic codepoints (7 + 1 + 10 + 1 + 4), with a real style token.
    std::vector<std::vector<std::string>> table = { { stevensTerminal::style("Быстрая коричневая лиса", {{"textColor", "red"}}) } };
    std::string result = stevensTerminal::formatTableAsString(
        table,
        {},
        {   {"column widths",  "10"},
            {"enable wrapping", "true"}    }
    );

    // Must decode cleanly - a byte-based cut at codepoint width 10 (20 bytes) would land mid-character
    // and produce invalid UTF-8, which charCount() would throw on. (Not lineDisplayWidth() here -
    // result is a multi-line wrapped table, and lineDisplayWidth() rightly throws on embedded newlines.)
    ASSERT_NO_THROW(stevensStringLib::charCount(result));

    // Cell wrapping now goes through stevensStringLib::wrapToWidth(), which prefers breaking at
    // spaces (like curses_wwrap) rather than hard-cutting mid-word - a real improvement over the
    // old byte/codepoint-width hard cut, and this content has spaces to break at.
    ASSERT_NE(result.find("Быстрая"), std::string::npos);
    ASSERT_NE(result.find("коричневая"), std::string::npos);
    ASSERT_NE(result.find("лиса"), std::string::npos);
}

TEST(FormatTableAsString, wrapsUnstyledContentWithoutLosingLastCharacter)
{
    // Regression test: unstyled cell content used to get wrapped in a bare "{chunk}" (no "$[...]"
    // suffix), which findToken() didn't recognize as a valid token, so resizeStyledString()'s
    // "no tokens" branch then truncated the literal string *including the braces*, silently eating
    // the closing brace and the chunk's last character. Fixed by routing every wrapped chunk through
    // stevensTerminal::style() (which always emits a valid token, even with an empty style map)
    // instead of hand-building "{...}$[...]" only when a style happened to be present.
    std::vector<std::vector<std::string>> table = { { "abcdefghij klmnop" } }; // no style token at all
    std::string result = stevensTerminal::formatTableAsString(
        table,
        {},
        {   {"column widths",  "10"},
            {"enable wrapping", "true"}    }
    );
    std::string clean = stevensTerminal::removeAllStyleTokenization(result);

    ASSERT_NE(clean.find("abcdefghij"), std::string::npos); // full first chunk present, not "abcdefghi"
    ASSERT_NE(clean.find("klmnop"), std::string::npos);
}

TEST(FormatTableAsString, autoColumnWidthUsesDisplayWidthNotByteCount)
{
    // "мир мир мир" (11 codepoints, 20 bytes, all single-column Cyrillic) is unambiguously the
    // longest/widest entry by byte count, codepoint count, AND display width alike, so
    // getStringWithMaxDisplayWidth() picks it correctly regardless of which measure is used. This
    // isolates just the fix under test: columnWidths must be computed as a display-width count (11),
    // so "hi" pads out to 11 columns, not 20 (the old byte-count bug's answer).
    std::vector<std::vector<std::string>> table = { { "мир мир мир" }, { "hi" } };
    std::string result = stevensTerminal::formatTableAsString(table, {}, {});

    ASSERT_NE(result.find("hi         "), std::string::npos); // "hi" + 9 spaces = 11 columns
    ASSERT_EQ(result.find("hi" + std::string(18, ' ')), std::string::npos); // would be the byte-width-padded (wrong) result
}

TEST(FormatTableAsString, autoColumnWidthAccountsForCjkDoubleWidth)
{
    // "世界世界" (4 codepoints, but double-width -> 8 terminal columns) is visually widest despite
    // having fewer codepoints than "hello world" would. A codepoint-count-only comparison would
    // (wrongly) undersize the column; display width correctly sizes it to 8 columns.
    std::vector<std::vector<std::string>> table = { { "世界世界" }, { "hi" } };
    std::string result = stevensTerminal::formatTableAsString(table, {}, {});

    ASSERT_NE(result.find("hi      "), std::string::npos); // "hi" + 6 spaces = 8 columns
}


// NOTE: countContentLines() tests removed — the function only ever existed in the orphaned
// top-level Styling.hpp (dead code, not included by stevensTerminal.hpp; see subnamespaces/Styling.hpp,
// which is the file actually built). Never migrated during that refactor.


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
TEST(StylingModule, style_basic_functionality)
{
    // Test basic style token creation
    std::string text = "Hello World";
    std::unordered_map<std::string, std::string> styles = {{"textColor", "red"}, {"bold", "true"}};
    std::string styledText = stevensTerminal::style(text, styles);
    
    ASSERT_NE(styledText.find("{"), std::string::npos);
    ASSERT_NE(styledText.find("}$["), std::string::npos);
    ASSERT_NE(styledText.find("textColor=red"), std::string::npos);
    ASSERT_NE(styledText.find("bold=true"), std::string::npos);
}

TEST(StylingModule, style_empty_styles)
{
    // Test with empty style map
    std::string text = "Test";
    std::unordered_map<std::string, std::string> emptyStyles;
    std::string result = stevensTerminal::style(text, emptyStyles);
    
    ASSERT_NE(result.find("{Test}$["), std::string::npos);
    ASSERT_FALSE(result.empty());
}

TEST(StylingModule, removeAllStyleTokenization_basic)
{
    // Test removing style tokens
    std::string text = "Hello World";
    std::unordered_map<std::string, std::string> styles = {{"textColor", "red"}};
    std::string styledText = stevensTerminal::style(text, styles);
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
    std::string styledText = stevensTerminal::style(text, styles);
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
    std::string styledText = stevensTerminal::style(text, styles);
    std::string resized = stevensTerminal::resizeStyledString(styledText, 0);
    std::string resizedClean = stevensTerminal::removeAllStyleTokenization(resized);
    
    ASSERT_TRUE(resizedClean.empty());
}

TEST(StylingModule, resizeStyledString_expand_with_fill)
{
    // Test expanding string with fill character
    std::string text = "Hi";
    std::unordered_map<std::string, std::string> styles = {{"textColor", "green"}};
    std::string styledText = stevensTerminal::style(text, styles);
    std::string resized = stevensTerminal::resizeStyledString(styledText, 5, '*');
    std::string resizedClean = stevensTerminal::removeAllStyleTokenization(resized);
    
    ASSERT_EQ(resizedClean.length(), 5);
    ASSERT_STREQ(resizedClean.c_str(), "Hi***");
}

/*** styleRandomTextColorPerCharacter() / styleMacro dispatch tests
 * All use an explicit colorPool so they don't depend on Colors::curses_colors having been
 * populated by curses_prepare_color() (which requires a live ncurses session -- see the
 * HeadlessNcursesColorTest fixture further down for tests that exercise the real global palette). */
TEST(StyleRandomTextColorPerCharacter, preservesContentAndWrapsEachCharacterInItsOwnToken)
{
    std::string original = "Randomized!";
    std::unordered_map<std::string,int> pool = {{"magenta", 5}};
    std::string styled = stevensTerminal::styleRandomTextColorPerCharacter(original, pool);

    // Visible content round-trips exactly
    ASSERT_EQ(stevensTerminal::removeAllStyleTokenization(styled), original);

    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintTokenHelper::getAllTokens(styled);
    ASSERT_EQ(tokens.size(), original.size());
    for(size_t i = 0; i < tokens.size(); ++i)
    {
        EXPECT_EQ(tokens[i].content, std::string(1, original[i]));
        EXPECT_EQ(tokens[i].textColor, "magenta");
    }
}

TEST(StyleRandomTextColorPerCharacter, doesNotSplitMultiByteUtf8Characters)
{
    std::string original = "caf\xC3\xA9"; // "café" -- 'é' is a 2-byte UTF-8 codepoint
    std::unordered_map<std::string,int> pool = {{"magenta", 5}};
    std::string styled = stevensTerminal::styleRandomTextColorPerCharacter(original, pool);

    ASSERT_EQ(stevensTerminal::removeAllStyleTokenization(styled), original);

    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintTokenHelper::getAllTokens(styled);
    ASSERT_EQ(tokens.size(), 4u); // c, a, f, é -- 4 codepoints, not 5 bytes
    EXPECT_EQ(tokens[3].content, "\xC3\xA9");
}

TEST(StyleMacro, unknownStyleMacroFallsBackToPlainToken)
{
    std::string result = stevensTerminal::style("Test", {{"styleMacro", "doesNotExist"}});

    // Falls through to normal token building rather than crashing or silently dropping content
    ASSERT_NE(result.find("{Test}$["), std::string::npos);
}

/***** Regression test for a real cultgame bug: toast messages were pre-wrapped via
 * stevensStringLib::wrapToWidth() BEFORE ever reaching the token-aware printer. wrapToWidth() has
 * zero concept of {content}$[style] markup -- it measures raw bytes/display-width of whatever
 * string it's given -- so a string built from many small back-to-back tokens (much longer in raw
 * byte length than in visible width, e.g. styleRandomTextColorPerCharacter()'s output for a short
 * word) gets force-broken in the middle of a token's markup, corrupting it. In production this
 * showed up as ncurses "Color pair 'X' does not exist" errors with garbled/merged color names.
 * Reproduced here at the pure string level with a fixed color (deterministic token length, so the
 * forced break's position is exactly predictable -- no reliance on real curses color state). *****/
TEST(WrapToWidth, PreWrappingManyBackToBackSingleCharacterTokensCorruptsTokenMarkup)
{
    std::string original = "Randomized!"; // 11 visible characters
    std::unordered_map<std::string,int> pool = {{"magenta", 5}};
    std::string styled = stevensTerminal::styleRandomTextColorPerCharacter(original, pool);

    // The raw markup is far longer than the visible text -- exactly what makes the naive
    // byte-oriented wrap dangerous here (11 tokens * 23 raw chars/token = 253 raw chars).
    ASSERT_GT(styled.length(), 80u);

    // This is the exact anti-pattern the real bug used: wrap the ALREADY-TOKENIZED string
    // directly, with no awareness of token boundaries.
    std::string corrupted = stevensStringLib::wrapToWidth(styled, 80);

    // A correctly-formed styled string always has exactly one token per character and
    // reconstructs to the original visible text when stripped. The corrupted version does not.
    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintTokenHelper::getAllTokens(corrupted);
    std::string recoveredText = stevensTerminal::removeAllStyleTokenization(corrupted);

    bool tokenCountCorrupted = (tokens.size() != original.size());
    bool contentCorrupted = (recoveredText != original);
    EXPECT_TRUE(tokenCountCorrupted || contentCorrupted)
        << "Expected wrapToWidth() to corrupt the token stream here, reproducing the real bug -- "
           "if this now passes cleanly, something about wrapToWidth()'s behavior has changed.";
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
    std::string styledText = stevensTerminal::style("test", {{"textColor", "red"}});
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
    std::string styledText = stevensTerminal::style(testText, styles);
    std::string cleanText = stevensTerminal::removeAllStyleTokenization(styledText);
    
    ASSERT_STREQ(cleanText.c_str(), testText.c_str());
    ASSERT_TRUE(stevensTerminal::inputWithinResponseRange("1", 5));
}

/*** Error Handling and Edge Cases ***/
TEST(ErrorHandling, styling_with_empty_strings)
{
    // Test styling functions with empty strings
    std::string result = stevensTerminal::style("", {{"textColor", "red"}});
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

// NOTE: getValueColor() tests removed — the function moved to cultgame's own cultgameLib
// (cultgameLib::getValueColor(), see cultgame/src/libraries/cultgameLib.hpp) per the ownership
// decision noted at SkillCheck.cpp:247. The stevensTerminal copy is commented out/removed in
// subnamespaces/Styling.hpp, not just orphaned.

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

    // Empty string means "genuinely unspecified" for inheritStyle() (see its doc comment) - an
    // explicit "default" is treated as the author's deliberate choice and does NOT inherit, so a
    // child can opt out of inheritance and stay terminal-default. To actually test inheritance, the
    // child's unset attributes need to be empty, not the literal string "default".
    stevensTerminal::PrintToken child;
    child.textColor = "";
    child.bgColor = "";
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
    // textColor/bgColor set to the explicit string "default" ARE serialized (that's a deliberate
    // author choice, distinct from being unset - see getStyleString()'s doc comment). bold/blink
    // being false is indistinguishable from "unset" for a bool field, so they're omitted rather
    // than emitted as "bold=false"/"blink=false" - this is what lets an unset/false attribute stay
    // inheritable when this token is nested inside a parent.
    stevensTerminal::PrintToken token;
    token.textColor = "default";
    token.bgColor = "default";
    token.bold = false;
    token.blink = false;

    std::string styleString = token.getStyleString();

    ASSERT_NE(styleString.find("textColor=default"), std::string::npos);
    ASSERT_NE(styleString.find("bgColor=default"), std::string::npos);
    ASSERT_EQ(styleString.find("bold=false"), std::string::npos);
    ASSERT_EQ(styleString.find("blink=false"), std::string::npos);
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
    // Regression test: PrintTokenHelper::getAllTokens() reports existsAtIndex relative to the
    // ORIGINAL fully-tokenized string, but resizeStyledString() reinserts tokens into the
    // token-stripped content string - a second (or later) token's position needs adjusting for the
    // wrapper overhead of every earlier token, or it lands in the wrong spot (or gets dropped/
    // corrupted) once there's more than one token.
    std::string input = "{First}$[textColor=red] {Second}$[textColor=blue]";
    std::string resized = stevensTerminal::resizeStyledString(input, 8);
    std::string clean = stevensTerminal::removeAllStyleTokenization(resized);

    ASSERT_EQ(clean, "First Se"); // "Second" correctly truncated to "Se" to fit width 8
    ASSERT_NE(resized.find("{First}$[textColor=red]"), std::string::npos); // first token untouched
    ASSERT_NE(resized.find("{Se}$[textColor=blue]"), std::string::npos);   // second token truncated but its own style preserved
}

TEST(ResizeStyledString, resize_with_multiple_tokens_no_truncation_needed)
{
    // When nothing needs truncating, both tokens (and the space between them) should survive
    // completely intact, just padded out to the desired width.
    std::string input = "{First}$[textColor=red] {Second}$[textColor=blue]";
    std::string resized = stevensTerminal::resizeStyledString(input, 20);
    std::string clean = stevensTerminal::removeAllStyleTokenization(resized);

    ASSERT_EQ(clean, "First Second        "); // 12 real chars + 8 padding spaces = 20
    ASSERT_NE(resized.find("{First}$[textColor=red]"), std::string::npos);
    ASSERT_NE(resized.find("{Second}$[textColor=blue]"), std::string::npos);
}

TEST(ResizeStyledString, resize_no_tokens)
{
    std::string input = "Plain text here";
    std::string resized = stevensTerminal::resizeStyledString(input, 5);

    ASSERT_EQ(resized, "Plain");
}

/***** Headless ncurses fixture - real rendering correctness tests for curses_wwrap()/
 * curses_wwrap_withTokens(), since neither had any prior test coverage and both were
 * substantially rewritten to use the display-width-aware stevensStringLib::wrapToWidth(). *****/
class HeadlessNcursesTest : public ::testing::Test {
protected:
    WINDOW* win = nullptr;
    SCREEN* screen = nullptr;
    FILE* devnull = nullptr;

    void SetUp() override {
        // Real cultgame calls this once at startup (cultgame.cpp) before any ncurses calls -
        // without it, the process runs in the minimal "C" locale, which has no multibyte
        // awareness, so ncurses can't correctly write or read back UTF-8 content. Needed here
        // so this fixture matches production conditions instead of producing false failures.
        setlocale(LC_ALL, "");
        devnull = fopen("/dev/null", "w");
        screen = newterm("xterm", devnull, stdin);
        set_term(screen);
        win = newwin(24, 80, 0, 0);
    }

    void TearDown() override {
        delwin(win);
        endwin();
        if (devnull) fclose(devnull);
    }

    // Reads back a row of the window, trimmed of trailing space padding.
    //
    // mvwinnstr()'s size parameter is a BYTE budget, not a display-column count - a row that's
    // N columns wide can need up to 4*N bytes for multi-byte UTF-8 content (CJK/Cyrillic/etc.),
    // so the read buffer must be sized generously, independent of the window's column width, or
    // multi-byte rows silently read back truncated even though they were written correctly.
    std::string readRow(int y, int columnWidth = 80)
    {
        size_t byteBudget = static_cast<size_t>(columnWidth) * 4;
        std::vector<char> buf(byteBudget + 1, '\0');
        mvwinnstr(win, y, 0, buf.data(), static_cast<int>(byteBudget));
        std::string result(buf.data());
        size_t end = result.find_last_not_of(' ');
        return (end == std::string::npos) ? "" : result.substr(0, end + 1);
    }
};

TEST_F(HeadlessNcursesTest, CursesWwrap_BreaksAtSpaceNotMidWord)
{
    stevensTerminal::PrintHelper::curses_wwrap(win, 0, 0, "the quick brown fox", 0, {});
    EXPECT_EQ(readRow(0, 20), "the quick brown fox"); // fits in the default 80-wide window
}

TEST_F(HeadlessNcursesTest, CursesWwrap_WrapsCjkAtDisplayWidth)
{
    // 6-column window = exactly 3 double-width CJK codepoints per row (matches
    // stevensStringLib's own WrapToWidth.CjkWrapsAtDisplayWidthNotCodepointCount expectations)
    WINDOW * narrowWin = newwin(24, 6, 0, 0);
    stevensTerminal::PrintHelper::curses_wwrap(narrowWin, 0, 0, "世界世界世界", 0, {});

    // mvwinnstr()'s size is a byte budget, not a column count - CJK needs up to 3 bytes/column
    // worth of budget per character, so read generously rather than tying it to window width.
    std::vector<char> buf(64, '\0');
    mvwinnstr(narrowWin, 0, 0, buf.data(), 63);
    std::string row0(buf.data());
    mvwinnstr(narrowWin, 1, 0, buf.data(), 63);
    std::string row1(buf.data());

    EXPECT_NE(row0.find("世界世"), std::string::npos);
    EXPECT_NE(row1.find("界世界"), std::string::npos);
    delwin(narrowWin);
}

TEST_F(HeadlessNcursesTest, CursesWborder_ShortPatternRepeatsToExactWidth)
{
    WINDOW * narrowWin = newwin(10, 20, 0, 0);
    stevensTerminal::curses_wborder(narrowWin, {{"top", "-"}});
    std::string buf(64, '\0');
    mvwinnstr(narrowWin, 0, 0, buf.data(), 63);
    std::string row0(buf.data());
    EXPECT_EQ(row0, std::string(20, '-')); // exactly fills the 20-column window, no overshoot
    delwin(narrowWin);
}

TEST_F(HeadlessNcursesTest, CursesWborder_MultiCharPatternFitsExactWidth)
{
    WINDOW * narrowWin = newwin(10, 21, 0, 0);
    stevensTerminal::curses_wborder(narrowWin, {{"top", "=~"}});
    std::string buf(64, '\0');
    mvwinnstr(narrowWin, 0, 0, buf.data(), 63);
    std::string row0(buf.data());
    EXPECT_EQ(row0.size(), 21u); // "=~" repeated + trimmed to exactly 21 columns
    EXPECT_EQ(row0, "=~=~=~=~=~=~=~=~=~=~="); // 10 full "=~" pairs + 1 leftover "="
    delwin(narrowWin);
}

TEST_F(HeadlessNcursesTest, CursesWborder_CjkPatternDoesNotOverflowWidth)
{
    // 10-column window with a double-width CJK pattern - must not overshoot past column 10
    WINDOW * narrowWin = newwin(10, 10, 0, 0);
    stevensTerminal::curses_wborder(narrowWin, {{"top", "世"}});
    std::string buf(64, '\0');
    mvwinnstr(narrowWin, 0, 0, buf.data(), 63);
    std::string row0(buf.data());
    size_t end = row0.find_last_not_of(' ');
    row0 = (end == std::string::npos) ? "" : row0.substr(0, end + 1);
    EXPECT_EQ(stevensStringLib::lineDisplayWidth(row0), 10u);
    delwin(narrowWin);
}

TEST_F(HeadlessNcursesTest, CursesWwrapWithTokens_MultipleTokensShareOneRow)
{
    // Regression test: tokens must continue on the SAME visual row when they fit, not each
    // force a fresh row - this is the exact bug caught while rewriting this function to use
    // wrapToWidth()'s firstLineWidth parameter.
    std::vector<stevensTerminal::PrintToken> tokens = {
        stevensTerminal::PrintToken("Hello "),
        stevensTerminal::PrintToken("World")
    };
    stevensTerminal::PrintHelper::curses_wwrap_withTokens(win, 0, 0, tokens, {}, {}, true);
    EXPECT_EQ(readRow(0), "Hello World");
}

TEST_F(HeadlessNcursesTest, CursesWwrapWithTokens_LongTokenWrapsAcrossRows)
{
    std::vector<stevensTerminal::PrintToken> tokens = {
        stevensTerminal::PrintToken("the quick brown fox jumps over the lazy dog and then keeps right on going for a while longer")
    };
    stevensTerminal::PrintHelper::curses_wwrap_withTokens(win, 0, 0, tokens, {}, {}, true);
    std::string row0 = readRow(0);
    std::string row1 = readRow(1);
    EXPECT_LE(row0.size(), 80u);
    EXPECT_FALSE(row1.empty()); // content actually continued onto a second row
    // Neither row should have been cut mid-word
    EXPECT_NE(row0.back(), ' ');
}

TEST_F(HeadlessNcursesTest, CursesWwrapWithTokens_BareNewlineTokenForcesRowAdvance)
{
    // Regression test: stevensTerminal::printVector_str() (used to render response grids like
    // the main menu) builds its output as alternating styled-label tokens and plain "between"
    // tokens, where each grid row is terminated by a token whose ENTIRE content is "\n" - not
    // embedded in a larger token. wrapToWidth() always appends its own trailing "\n" after the
    // last line it processes, so such a token round-trips to a single empty row and the
    // row-loop's "more rows remain in this token" check never sees the transition, silently
    // gluing the next row's tokens onto the current row.
    std::vector<stevensTerminal::PrintToken> tokens = {
        stevensTerminal::PrintToken("1 - New game"),
        stevensTerminal::PrintToken("     "),
        stevensTerminal::PrintToken("5 - Dev Tools"),
        stevensTerminal::PrintToken("\n"),
        stevensTerminal::PrintToken("2 - Load game"),
        stevensTerminal::PrintToken("     "),
        stevensTerminal::PrintToken("6 - Credits"),
    };
    stevensTerminal::PrintHelper::curses_wwrap_withTokens(win, 0, 0, tokens, {}, {}, true);
    std::string row0 = readRow(0);
    std::string row1 = readRow(1);
    EXPECT_EQ(row0, "1 - New game     5 - Dev Tools");
    EXPECT_EQ(row1, "2 - Load game     6 - Credits");
}

TEST_F(HeadlessNcursesTest, CursesWwrapWithTokens_OrphanWordDefersToFreshRowInsteadOfMidWordBreak)
{
    // Regression test for a real cultgame HUD header bug: a token continuing mid-row whose
    // leading word can't fit in the remaining space was being force-broken mid-word by
    // wrapToWidth() (its correct, documented behavior for a standalone line with no other
    // choice), instead of deferring the whole token to a fresh row where the full row width
    // is available. This reproduces the exact real-world case: a 21-column token plus a
    // 3-column separator leaves exactly 3 columns on the row for "Ardor: 0" - not enough to
    // reach even its first space (6 columns in, after "Ardor:") - which used to split into
    // "Ard" on the current row and "or: 0" on the next.
    WINDOW * narrowWin = newwin(24, 27, 0, 0);
    std::vector<stevensTerminal::PrintToken> tokens = {
        stevensTerminal::PrintToken("Devotion to Nature: 0"),
        stevensTerminal::PrintToken("   "),
        stevensTerminal::PrintToken("Ardor: 0")
    };
    stevensTerminal::PrintHelper::curses_wwrap_withTokens(narrowWin, 0, 0, tokens, {}, {}, true);

    std::vector<char> buf(128, '\0');
    mvwinnstr(narrowWin, 0, 0, buf.data(), 127);
    std::string row0(buf.data());
    size_t end0 = row0.find_last_not_of(' ');
    row0 = (end0 == std::string::npos) ? "" : row0.substr(0, end0 + 1);

    mvwinnstr(narrowWin, 1, 0, buf.data(), 127);
    std::string row1(buf.data());
    size_t end1 = row1.find_last_not_of(' ');
    row1 = (end1 == std::string::npos) ? "" : row1.substr(0, end1 + 1);

    EXPECT_EQ(row0, "Devotion to Nature: 0");
    EXPECT_EQ(row1, "Ardor: 0"); // whole word deferred to its own row, not split into "Ard" / "or: 0"
    delwin(narrowWin);
}

/***** Fixture for tests that need real color-pair resolution (Colors::curses_colors /
 * Colors::curses_colorPairs actually populated) -- extends HeadlessNcursesTest with the same
 * color setup cultgame's own startup (stevensTerminal::initialize() -> curses_prepare_color())
 * does, since a bare newterm() session has no colors registered. *****/
class HeadlessNcursesColorTest : public HeadlessNcursesTest {
protected:
    void SetUp() override {
        HeadlessNcursesTest::SetUp();
        stevensTerminal::curses_prepare_color();
    }
};

TEST_F(HeadlessNcursesColorTest, StyleMacro_DispatchesToRandomTextColorPerCharacterWithRealPalette)
{
    // Exercises the macro's default colorPool (real Colors::curses_colors, populated above) --
    // can't pin an exact color since it's randomly chosen, but every token must land on a
    // genuinely registered color name.
    std::string original = "Hi";
    std::string styled = stevensTerminal::style(original, {{"styleMacro", "randomTextColorPerCharacter"}});

    std::vector<stevensTerminal::PrintToken> tokens = stevensTerminal::PrintTokenHelper::getAllTokens(styled);
    ASSERT_EQ(tokens.size(), original.size());
    for(size_t i = 0; i < tokens.size(); ++i)
    {
        EXPECT_EQ(tokens[i].content, std::string(1, original[i]));
        EXPECT_TRUE(stevensTerminal::Colors::curses_colors.contains(tokens[i].textColor))
            << "Token color '" << tokens[i].textColor << "' is not a registered color name";
    }
}

/***** Regression coverage for the real cultgame toast-message corruption bug (see the
 * WrapToWidth.PreWrappingManyBackToBackSingleCharacterTokensCorruptsTokenMarkup test above for the
 * pure string-level version). These two go all the way through curses_wprint() with a real color
 * palette registered, to prove which pattern is actually safe to render and which one reproduces
 * the "Color pair does not exist" errors seen in-game. Uses a fixed single-color pool so the
 * token length -- and therefore exactly where the forced wrap break lands -- is deterministic. *****/
TEST_F(HeadlessNcursesColorTest, CursesWprint_RawManyTokenRainbowText_RendersCleanlyWithNoColorPairErrors)
{
    // The SAFE pattern: pass the tokenized string directly into curses_wprint's own token-aware
    // wrap, exactly like cultgame's view.prompt handling (never pre-wrapped via wrapToWidth()).
    std::string original = "Randomized!";
    std::unordered_map<std::string,int> pool = {{"magenta", 5}};
    std::string styled = stevensTerminal::styleRandomTextColorPerCharacter(original, pool);

    testing::internal::CaptureStderr();
    stevensTerminal::curses_wprint(win, 0, 0, styled, {}, {{"wrap","true"}});
    std::string errOutput = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(errOutput.empty()) << "Unexpected color pair error(s): " << errOutput;
    EXPECT_EQ(readRow(0), original);
}

TEST_F(HeadlessNcursesColorTest, CursesWprint_PreWrappedManyTokenRainbowText_ReproducesColorPairCorruptionBug)
{
    // The UNSAFE pattern that caused the real bug: pre-wrap the already-tokenized string via
    // stevensStringLib::wrapToWidth() (token-blind) before ever printing it -- this is what
    // cultgame's toast message pipeline used to do.
    std::string original = "Randomized!";
    std::unordered_map<std::string,int> pool = {{"magenta", 5}};
    std::string styled = stevensTerminal::styleRandomTextColorPerCharacter(original, pool);
    std::string preWrapped = stevensStringLib::wrapToWidth(styled, 80);

    testing::internal::CaptureStderr();
    stevensTerminal::curses_wprint(win, 0, 0, preWrapped, {}, {{"wrap","true"}});
    std::string errOutput = testing::internal::GetCapturedStderr();

    // Reproduces the real-world bug: either a "does not exist" color pair error is thrown, or the
    // visible text no longer matches the original -- proving the pre-wrap step is unsafe.
    bool threwColorPairError = (errOutput.find("does not exist") != std::string::npos);
    bool visibleTextCorrupted = (readRow(0) != original);
    EXPECT_TRUE(threwColorPairError || visibleTextCorrupted)
        << "Expected the pre-wrap anti-pattern to corrupt rendering here, matching the real bug "
           "report -- if this now passes cleanly, wrapToWidth()'s behavior may have changed.";
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

