#include <benchmark/benchmark.h>
#include "../stevensTerminal.hpp"
#include <string>
#include <vector>
#include <random>

// Test data generators
namespace BenchmarkData {
    
    // Generate simple token strings
    std::string generateSimpleToken(int length) {
        std::string result = "{";
        for(int i = 0; i < length; ++i) {
            result += "a";
        }
        result += "}$[textColor=red]";
        return result;
    }
    
    // Generate complex nested token strings
    std::string generateNestedTokens(int depth) {
        std::string result = "{";
        std::string content = "Level 0 ";
        
        for(int i = 1; i < depth; ++i) {
            content += "{Level " + std::to_string(i) + " ";
        }
        
        // Close all the nested braces with styles
        for(int i = depth - 1; i >= 1; --i) {
            content += "}$[textColor=red" + std::to_string(i) + "]";
        }
        
        result += content + "}$[bgColor=blue]";
        return result;
    }
    
    // Generate multiple token string
    std::string generateMultipleTokens(int count, int tokenLength = 10) {
        std::string result;
        std::vector<std::string> colors = {"red", "green", "blue", "yellow", "cyan", "magenta", "white"};
        
        for(int i = 0; i < count; ++i) {
            result += "{";
            for(int j = 0; j < tokenLength; ++j) {
                result += static_cast<char>('a' + (i + j) % 26);
            }
            result += "}$[textColor=" + colors[i % colors.size()] + "]";
        }
        
        return result;
    }
    
    // Generate deeply nested stress test
    std::string generateDeepNesting(int depth) {
        std::string result = "{";
        
        for(int i = 0; i < depth; ++i) {
            result += "L" + std::to_string(i) + " {";
        }
        
        result += "DEEP_CONTENT";
        
        for(int i = depth - 1; i >= 0; --i) {
            result += "}$[textColor=color" + std::to_string(i) + "]";
        }
        
        result += "}$[bgColor=black]";
        return result;
    }
    
    // Generate complex style inheritance test
    std::string generateComplexStyleInheritance() {
        return "{Outer text {Nested with {Deep content}$[textColor=green,bold=true]} more text}$[textColor=red,bgColor=blue,italic=true]} final}$[bgColor=black,underline=true,blink=true]";
    }
    
    // Generate style string with many properties
    std::string generateComplexStyle() {
        return "textColor=bright-red,bgColor=dark-blue,bold=true,italic=true,underline=true,blink=true,reverse=true";
    }
}

// ==== BASIC TOKENIZATION BENCHMARKS ====

static void BM_SimpleTokenization(benchmark::State& state) {
    std::string input = BenchmarkData::generateSimpleToken(state.range(0));
    
    for (auto _ : state) {
        std::string inputCopy = input; // Copy since tokenization modifies input
        auto tokens = stevensTerminal::PrintHelper::tokenizePrintString(inputCopy);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetBytesProcessed(state.iterations() * input.length());
}
BENCHMARK(BM_SimpleTokenization)->Range(8, 8192)->Complexity();

static void BM_MultipleTokens(benchmark::State& state) {
    std::string input = BenchmarkData::generateMultipleTokens(state.range(0));
    
    for (auto _ : state) {
        std::string inputCopy = input;
        auto tokens = stevensTerminal::PrintHelper::tokenizePrintString(inputCopy);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetBytesProcessed(state.iterations() * input.length());
}
BENCHMARK(BM_MultipleTokens)->Range(1, 100)->Complexity();

// ==== NESTED TOKEN BENCHMARKS ====

static void BM_NestedTokensShallow(benchmark::State& state) {
    std::string input = BenchmarkData::generateNestedTokens(state.range(0));
    
    for (auto _ : state) {
        std::string inputCopy = input;
        auto tokens = stevensTerminal::PrintHelper::tokenizePrintString(inputCopy);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetBytesProcessed(state.iterations() * input.length());
}
BENCHMARK(BM_NestedTokensShallow)->Range(1, 10)->Complexity();

static void BM_NestedTokensDeep(benchmark::State& state) {
    std::string input = BenchmarkData::generateDeepNesting(state.range(0));
    
    for (auto _ : state) {
        std::string inputCopy = input;
        auto tokens = stevensTerminal::PrintHelper::tokenizePrintString(inputCopy);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetBytesProcessed(state.iterations() * input.length());
}
BENCHMARK(BM_NestedTokensDeep)->Range(1, 25)->Complexity();

// ==== PREPROCESSING BENCHMARKS ====

static void BM_PreprocessNestedTokens(benchmark::State& state) {
    std::string input = BenchmarkData::generateNestedTokens(state.range(0));
    
    for (auto _ : state) {
        std::string inputCopy = input;
        auto result = stevensTerminal::PrintHelper::preprocessNestedTokens(inputCopy);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetBytesProcessed(state.iterations() * input.length());
}
BENCHMARK(BM_PreprocessNestedTokens)->Range(1, 20)->Complexity();

static void BM_PreprocessDeepNesting(benchmark::State& state) {
    std::string input = BenchmarkData::generateDeepNesting(state.range(0));
    
    for (auto _ : state) {
        std::string inputCopy = input;
        auto result = stevensTerminal::PrintHelper::preprocessNestedTokens(inputCopy);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetBytesProcessed(state.iterations() * input.length());
}
BENCHMARK(BM_PreprocessDeepNesting)->Range(1, 30)->Complexity();

// ==== STYLE INHERITANCE BENCHMARKS ====

static void BM_StyleInheritance_Simple(benchmark::State& state) {
    std::string nestedStyle = "textColor=green";
    std::string parentStyle = "bgColor=blue,bold=true";
    
    for (auto _ : state) {
        auto result = stevensTerminal::PrintHelper::inheritParentStyle(nestedStyle, parentStyle);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetBytesProcessed(state.iterations() * (nestedStyle.length() + parentStyle.length()));
}
BENCHMARK(BM_StyleInheritance_Simple);

static void BM_StyleInheritance_Complex(benchmark::State& state) {
    std::string nestedStyle = BenchmarkData::generateComplexStyle();
    std::string parentStyle = "bgColor=black,underline=true,blink=false,reverse=true,dim=true";
    
    for (auto _ : state) {
        auto result = stevensTerminal::PrintHelper::inheritParentStyle(nestedStyle, parentStyle);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetBytesProcessed(state.iterations() * (nestedStyle.length() + parentStyle.length()));
}
BENCHMARK(BM_StyleInheritance_Complex);

static void BM_StyleInheritance_Scaling(benchmark::State& state) {
    std::string baseStyle = "textColor=red,bgColor=blue";
    std::string parentStyle = baseStyle;
    
    // Add more style properties based on range
    for(int i = 0; i < state.range(0); ++i) {
        parentStyle += ",prop" + std::to_string(i) + "=value" + std::to_string(i);
    }
    
    std::string nestedStyle = "textColor=green";
    
    for (auto _ : state) {
        auto result = stevensTerminal::PrintHelper::inheritParentStyle(nestedStyle, parentStyle);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetBytesProcessed(state.iterations() * (nestedStyle.length() + parentStyle.length()));
}
BENCHMARK(BM_StyleInheritance_Scaling)->Range(1, 50)->Complexity();

// ==== PATTERN MATCHING BENCHMARKS ====

static void BM_PatternMatching_TokenEnd(benchmark::State& state) {
    std::string input = "some text before }$[ and after";
    size_t pos = 18; // Position of }
    
    for (auto _ : state) {
        bool result = stevensTerminal::PrintHelper::matchesTokenEnd(input, pos);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_PatternMatching_TokenEnd);

// ==== REAL-WORLD SCENARIO BENCHMARKS ====

static void BM_MuggedManScenario(benchmark::State& state) {
    std::string input = "{1 - {+5 local opinion}$[textColor=bright-green], {-5 gold}$[textColor=red] Take the man to an inn to care for his wounds}$[textColor=bright-yellow,bgColor=black]";
    
    for (auto _ : state) {
        std::string inputCopy = input;
        auto tokens = stevensTerminal::PrintHelper::tokenizePrintString(inputCopy);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetBytesProcessed(state.iterations() * input.length());
}
BENCHMARK(BM_MuggedManScenario);

static void BM_ComplexStyleInheritanceScenario(benchmark::State& state) {
    std::string input = BenchmarkData::generateComplexStyleInheritance();
    
    for (auto _ : state) {
        std::string inputCopy = input;
        auto tokens = stevensTerminal::PrintHelper::tokenizePrintString(inputCopy);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetBytesProcessed(state.iterations() * input.length());
}
BENCHMARK(BM_ComplexStyleInheritanceScenario);

// ==== STRESS TEST BENCHMARKS ====

static void BM_StressTest_VeryDeepNesting(benchmark::State& state) {
    std::string input = BenchmarkData::generateDeepNesting(50); // Very deep
    
    for (auto _ : state) {
        std::string inputCopy = input;
        auto tokens = stevensTerminal::PrintHelper::tokenizePrintString(inputCopy);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetBytesProcessed(state.iterations() * input.length());
}
BENCHMARK(BM_StressTest_VeryDeepNesting);

static void BM_StressTest_ManyTokens(benchmark::State& state) {
    std::string input = BenchmarkData::generateMultipleTokens(1000, 20); // Many tokens
    
    for (auto _ : state) {
        std::string inputCopy = input;
        auto tokens = stevensTerminal::PrintHelper::tokenizePrintString(inputCopy);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetBytesProcessed(state.iterations() * input.length());
}
BENCHMARK(BM_StressTest_ManyTokens);

// ==== MEMORY ALLOCATION BENCHMARKS ====

static void BM_MemoryUsage_TokenCreation(benchmark::State& state) {
    std::string input = BenchmarkData::generateMultipleTokens(state.range(0));
    
    for (auto _ : state) {
        std::string inputCopy = input;
        auto tokens = stevensTerminal::PrintHelper::tokenizePrintString(inputCopy);
        
        // Force memory allocation tracking
        std::vector<stevensTerminal::PrintToken> tokensCopy = tokens;
        benchmark::DoNotOptimize(tokensCopy);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_MemoryUsage_TokenCreation)->Range(1, 200)->Complexity();

// ==== COMPARISON BENCHMARKS (could be used to test old vs new implementations) ====

// Simulate old inefficient substring usage
static void BM_InefficiestSubstring_Simulation(benchmark::State& state) {
    std::string input = BenchmarkData::generateSimpleToken(1000);
    
    for (auto _ : state) {
        // Simulate lots of substr calls (what we optimized away)
        std::string result;
        for(size_t i = 0; i < input.length() - 3; i += 10) {
            std::string sub = input.substr(i, 3); // Expensive operation
            if(sub == "}$[") {
                result += "found";
            }
            benchmark::DoNotOptimize(sub);
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_InefficiestSubstring_Simulation);

// Efficient character-by-character checking (our optimization)
static void BM_EfficientPatternMatch(benchmark::State& state) {
    std::string input = BenchmarkData::generateSimpleToken(1000);
    
    for (auto _ : state) {
        std::string result;
        for(size_t i = 0; i < input.length() - 2; i += 10) {
            // Our optimized approach
            if(input[i] == '}' && input[i+1] == '$' && input[i+2] == '[') {
                result += "found";
            }
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_EfficientPatternMatch);

// ==== STRING LIBRARY BENCHMARKS ====



static void BM_VectorLib_Contains(benchmark::State& state) {
    std::vector<int> vec;
    for(int i = 0; i < state.range(0); ++i) {
        vec.push_back(i);
    }
    int searchValue = state.range(0) - 1; // Worst case: last element
    
    for (auto _ : state) {
        bool result = stevensVectorLib::contains(vec, searchValue);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_VectorLib_Contains)->Range(8, 8192)->Complexity();


// ==== UTF-8 CODEPOINT WRAPPING BENCHMARKS ====
// Cost of the UTF-8 <-> UTF-32 decode/encode added to PrintHelper::computeWrapSegments() for
// multi-byte correctness. Compares the current (codepoint-based) implementation against a
// reference copy of the original (byte-based, pre-fix) implementation on the same ASCII inputs,
// plus the current implementation alone on multi-byte input (the original has no valid
// multi-byte baseline - it doesn't wrap multi-byte text correctly at all).

namespace BenchmarkOriginal {
    // Verbatim copy of computeWrapSegments() as it existed before the UTF-8 codepoint fix
    // (see stevensTerminal git history). Byte-based - kept only for this comparison, not
    // correct for multi-byte input.
    inline std::vector<std::string> computeWrapSegments_bytesOnly(
                            std::string line,
                            int xMove,
                            int width,
                            int borderAdjustment)
    {
        std::vector<std::string> segments;
        int textWidth = (width - borderAdjustment) - xMove;

        if (textWidth <= 0) return segments;

        while (true) {
            if ((int)line.length() <= textWidth) {
                segments.push_back(line);
                break;
            }
            size_t cutPos = line.rfind(' ', (size_t)(textWidth - 1));
            if (cutPos == std::string::npos) {
                segments.push_back(line.substr(0, textWidth));
                line = line.substr(textWidth);
            } else {
                segments.push_back(line.substr(0, cutPos));
                line = line.substr(cutPos + 1);
            }
        }

        return segments;
    }
}

namespace WrapBenchmarkData {
    inline std::string shortAsciiLine() {
        return "The quick brown fox jumps over the lazy dog"; // 45 chars
    }

    inline std::string longAsciiLine() {
        std::string line;
        for (int i = 0; i < 20; ++i) line += "The quick brown fox jumps over the lazy dog ";
        return line; // ~900 chars, wraps many times at narrow widths
    }

    inline std::string multiByteLine() {
        std::string line;
        for (int i = 0; i < 20; ++i) line += "Быстрая коричневая лиса перепрыгивает через ленивую собаку ";
        return line; // Cyrillic, comparable character count to longAsciiLine, 2 bytes/char
    }
}

/*** computeWrapSegments: short ASCII line, narrow width (frequent wraps) ***/
static void BM_ComputeWrapSegments_Original_ShortAscii(benchmark::State& state) {
    std::string line = WrapBenchmarkData::shortAsciiLine();
    for (auto _ : state) {
        auto result = BenchmarkOriginal::computeWrapSegments_bytesOnly(line, 0, 20, 0);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ComputeWrapSegments_Original_ShortAscii);

static void BM_ComputeWrapSegments_Fixed_ShortAscii(benchmark::State& state) {
    std::string line = WrapBenchmarkData::shortAsciiLine();
    for (auto _ : state) {
        auto result = stevensTerminal::PrintHelper::computeWrapSegments(line, 0, 20, 0);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ComputeWrapSegments_Fixed_ShortAscii);

/*** computeWrapSegments: long ASCII line, many wraps per call ***/
static void BM_ComputeWrapSegments_Original_LongAscii(benchmark::State& state) {
    std::string line = WrapBenchmarkData::longAsciiLine();
    for (auto _ : state) {
        auto result = BenchmarkOriginal::computeWrapSegments_bytesOnly(line, 0, 40, 0);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ComputeWrapSegments_Original_LongAscii);

static void BM_ComputeWrapSegments_Fixed_LongAscii(benchmark::State& state) {
    std::string line = WrapBenchmarkData::longAsciiLine();
    for (auto _ : state) {
        auto result = stevensTerminal::PrintHelper::computeWrapSegments(line, 0, 40, 0);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ComputeWrapSegments_Fixed_LongAscii);

/*** computeWrapSegments: multi-byte (Cyrillic) line - no valid "Original" baseline ***/
static void BM_ComputeWrapSegments_Fixed_MultiByte(benchmark::State& state) {
    std::string line = WrapBenchmarkData::multiByteLine();
    for (auto _ : state) {
        auto result = stevensTerminal::PrintHelper::computeWrapSegments(line, 0, 40, 0);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ComputeWrapSegments_Fixed_MultiByte);


// ==== BORDER-FITTING BENCHMARKS (curses_wborder top/bottom pattern-fitting logic) ====
// Same before/after comparison as computeWrapSegments, isolating just the string-prep logic
// (not ncurses I/O, which is identical before/after) - see curses_wborder() in stevensTerminal.cpp.

namespace BenchmarkOriginal {
    // Verbatim copy of curses_wborder()'s top-pattern-fitting logic as it existed before the
    // UTF-8 codepoint fix. Byte-based - kept only for this comparison, not correct for
    // multi-byte patterns.
    inline std::string fitBorderPattern_bytesOnly(const std::string & pattern, int width) {
        if ((int)pattern.length() >= width) {
            return pattern.substr(0, width);
        }
        std::string strToPrint = stevensStringLib::multiply(pattern, (width / pattern.length()));
        int i = 0;
        while ((int)strToPrint.length() != width) {
            if ((int)strToPrint.length() > width) {
                strToPrint.pop_back();
            } else if ((int)strToPrint.length() < width) {
                strToPrint += pattern[i % pattern.length()]; // original circularIndex() was byte-based too
                i++;
            }
        }
        return strToPrint;
    }

    // Fixed logic, extracted to a pure function for a fair comparison against the above
    // (real curses_wborder() also does real ncurses I/O, which is identical either way and
    // would just add noise to this specific comparison).
    inline std::string fitBorderPattern_codepoints(const std::string & pattern, int width) {
        if ((int)stevensStringLib::charCount(pattern) >= width) {
            return stevensStringLib::utf32to8(stevensStringLib::utf8to32(pattern).substr(0, width));
        }
        std::string strToPrint = stevensStringLib::multiply(pattern, (width / stevensStringLib::charCount(pattern)));
        std::u32string u32strToPrint = stevensStringLib::utf8to32(strToPrint);
        int i = 0;
        while ((int)u32strToPrint.length() != width) {
            if ((int)u32strToPrint.length() > width) {
                u32strToPrint.pop_back();
            } else if ((int)u32strToPrint.length() < width) {
                u32strToPrint += stevensStringLib::utf8to32(stevensStringLib::circularIndex(pattern, i));
                i++;
            }
        }
        return stevensStringLib::utf32to8(u32strToPrint);
    }
}

static void BM_BorderFit_Original_AsciiPattern(benchmark::State& state) {
    for (auto _ : state) {
        auto result = BenchmarkOriginal::fitBorderPattern_bytesOnly("-~", 80);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_BorderFit_Original_AsciiPattern);

static void BM_BorderFit_Fixed_AsciiPattern(benchmark::State& state) {
    for (auto _ : state) {
        auto result = BenchmarkOriginal::fitBorderPattern_codepoints("-~", 80);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_BorderFit_Fixed_AsciiPattern);

/*** Multi-byte (box-drawing) pattern - no valid "Original" baseline (would tear characters) ***/
static void BM_BorderFit_Fixed_MultiBytePattern(benchmark::State& state) {
    for (auto _ : state) {
        auto result = BenchmarkOriginal::fitBorderPattern_codepoints("\xE2\x99\xA6\xE2\x95\x90", 80); // diamond + double-line
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_BorderFit_Fixed_MultiBytePattern);


// ==== REAL curses_wwrap()/curses_wwrap_withTokens() END-TO-END BENCHMARKS ====
// Uses a headless ncurses screen (newterm() to /dev/null) so the real functions - including
// real ncurses I/O, not just the string prep - can be benchmarked directly.

class HeadlessNcursesFixture : public benchmark::Fixture {
public:
    WINDOW* win = nullptr;
    SCREEN* screen = nullptr;
    FILE* devnull = nullptr;

    void SetUp(benchmark::State&) override {
        devnull = fopen("/dev/null", "w");
        screen = newterm("xterm", devnull, stdin);
        set_term(screen);
        win = newwin(24, 80, 0, 0);
    }

    void TearDown(benchmark::State&) override {
        delwin(win);
        endwin();
        if (devnull) fclose(devnull);
    }
};

BENCHMARK_F(HeadlessNcursesFixture, BM_CursesWwrap_Original_LongAscii)(benchmark::State& state) {
    std::string line = WrapBenchmarkData::longAsciiLine();
    for (auto _ : state) {
        // Reference (pre-fix) curses_wwrap: byte-based length/substr/rfind.
        std::istringstream in(line);
        std::string l, output;
        int lineCutOffIndex = 0;
        while (getline(in, l)) {
            int textWidth = 40;
            while (true) {
                if (textWidth > 0 && (int)l.length() > textWidth) {
                    lineCutOffIndex = l.rfind(" ", (size_t)(textWidth - 1));
                    if (lineCutOffIndex == std::string::npos) {
                        output = l.substr(0, textWidth);
                        mvwprintw(win, 0, 0, "%s", output.c_str());
                        l = l.substr(textWidth);
                    } else {
                        output = l.substr(0, lineCutOffIndex);
                        mvwprintw(win, 0, 0, "%s", output.c_str());
                        l = l.substr(lineCutOffIndex + 1);
                    }
                } else {
                    output = l;
                    mvwprintw(win, 0, 0, "%s", output.c_str());
                    break;
                }
            }
        }
    }
}

BENCHMARK_F(HeadlessNcursesFixture, BM_CursesWwrap_Fixed_LongAscii)(benchmark::State& state) {
    std::string line = WrapBenchmarkData::longAsciiLine();
    for (auto _ : state) {
        stevensTerminal::PrintHelper::curses_wwrap(win, 0, 0, line, 0, {});
    }
}

BENCHMARK_F(HeadlessNcursesFixture, BM_CursesWwrap_Fixed_MultiByte)(benchmark::State& state) {
    std::string line = WrapBenchmarkData::multiByteLine();
    for (auto _ : state) {
        stevensTerminal::PrintHelper::curses_wwrap(win, 0, 0, line, 0, {});
    }
}

/*** curses_wwrap_withTokens: absolute cost only (Fixed) - complex enough that a faithful
     byte-based reference copy isn't worth the duplication; computeWrapSegments/curses_wwrap
     comparisons above already establish the per-conversion cost of this same pattern. ***/
BENCHMARK_F(HeadlessNcursesFixture, BM_CursesWwrapWithTokens_Fixed_LongAscii)(benchmark::State& state) {
    std::string input = WrapBenchmarkData::longAsciiLine();
    std::unordered_map<std::string,std::string> format = {{"wrap", "true"}};
    std::unordered_map<std::string,std::string> style = {};
    for (auto _ : state) {
        std::string copy = input;
        stevensTerminal::PrintHelper::curses_wprint(win, 0, 0, copy, style, format, true);
    }
}

BENCHMARK_F(HeadlessNcursesFixture, BM_CursesWwrapWithTokens_Fixed_MultiByte)(benchmark::State& state) {
    std::string input = WrapBenchmarkData::multiByteLine();
    std::unordered_map<std::string,std::string> format = {{"wrap", "true"}};
    std::unordered_map<std::string,std::string> style = {};
    for (auto _ : state) {
        std::string copy = input;
        stevensTerminal::PrintHelper::curses_wprint(win, 0, 0, copy, style, format, true);
    }
}

BENCHMARK_MAIN();
