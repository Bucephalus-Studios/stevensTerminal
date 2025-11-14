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
        auto tokens = s_TerminalPrintHelper::tokenizePrintString(inputCopy);
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
        auto tokens = s_TerminalPrintHelper::tokenizePrintString(inputCopy);
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
        auto tokens = s_TerminalPrintHelper::tokenizePrintString(inputCopy);
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
        auto tokens = s_TerminalPrintHelper::tokenizePrintString(inputCopy);
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
        auto result = s_TerminalPrintHelper::preprocessNestedTokens(inputCopy);
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
        auto result = s_TerminalPrintHelper::preprocessNestedTokens(inputCopy);
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
        auto result = s_TerminalPrintHelper::inheritParentStyle(nestedStyle, parentStyle);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetBytesProcessed(state.iterations() * (nestedStyle.length() + parentStyle.length()));
}
BENCHMARK(BM_StyleInheritance_Simple);

static void BM_StyleInheritance_Complex(benchmark::State& state) {
    std::string nestedStyle = BenchmarkData::generateComplexStyle();
    std::string parentStyle = "bgColor=black,underline=true,blink=false,reverse=true,dim=true";
    
    for (auto _ : state) {
        auto result = s_TerminalPrintHelper::inheritParentStyle(nestedStyle, parentStyle);
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
        auto result = s_TerminalPrintHelper::inheritParentStyle(nestedStyle, parentStyle);
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
        bool result = s_TerminalPrintHelper::matchesTokenEnd(input, pos);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_PatternMatching_TokenEnd);

// ==== REAL-WORLD SCENARIO BENCHMARKS ====

static void BM_MuggedManScenario(benchmark::State& state) {
    std::string input = "{1 - {+5 local opinion}$[textColor=bright-green], {-5 gold}$[textColor=red] Take the man to an inn to care for his wounds}$[textColor=bright-yellow,bgColor=black]";
    
    for (auto _ : state) {
        std::string inputCopy = input;
        auto tokens = s_TerminalPrintHelper::tokenizePrintString(inputCopy);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetBytesProcessed(state.iterations() * input.length());
}
BENCHMARK(BM_MuggedManScenario);

static void BM_ComplexStyleInheritanceScenario(benchmark::State& state) {
    std::string input = BenchmarkData::generateComplexStyleInheritance();
    
    for (auto _ : state) {
        std::string inputCopy = input;
        auto tokens = s_TerminalPrintHelper::tokenizePrintString(inputCopy);
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
        auto tokens = s_TerminalPrintHelper::tokenizePrintString(inputCopy);
        benchmark::DoNotOptimize(tokens);
    }
    
    state.SetBytesProcessed(state.iterations() * input.length());
}
BENCHMARK(BM_StressTest_VeryDeepNesting);

static void BM_StressTest_ManyTokens(benchmark::State& state) {
    std::string input = BenchmarkData::generateMultipleTokens(1000, 20); // Many tokens
    
    for (auto _ : state) {
        std::string inputCopy = input;
        auto tokens = s_TerminalPrintHelper::tokenizePrintString(inputCopy);
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
        auto tokens = s_TerminalPrintHelper::tokenizePrintString(inputCopy);
        
        // Force memory allocation tracking
        std::vector<s_TerminalPrintToken> tokensCopy = tokens;
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

BENCHMARK_MAIN();
