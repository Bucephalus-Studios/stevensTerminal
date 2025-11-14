# stevensTerminal Performance Benchmarks

This directory contains comprehensive performance benchmarks for the stevensTerminal library using Google Benchmark.

## Building and Running

### Prerequisites
- CMake 3.22.1 or newer
- C++23 compatible compiler
- pthread support

### Build Instructions

```bash
# From the benchmarking directory
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Running Benchmarks

```bash
# From the build directory
cd build

# Run all benchmarks
./benchmarkStevensTerminal

# Run specific benchmark patterns
./benchmarkStevensTerminal --benchmark_filter=BM_SimpleTokenization

# Run with different output formats
./benchmarkStevensTerminal --benchmark_format=json
./benchmarkStevensTerminal --benchmark_format=csv

# Run with time unit control
./benchmarkStevensTerminal --benchmark_time_unit=ns

# Run with repetitions for statistical accuracy
./benchmarkStevensTerminal --benchmark_repetitions=5

# Export results to files
./benchmarkStevensTerminal --benchmark_format=csv --benchmark_out=results.csv
./benchmarkStevensTerminal --benchmark_format=json --benchmark_out=results.json
```

### Common Benchmark Commands

```bash
# Quick development testing (fast subset)
./benchmarkStevensTerminal --benchmark_filter="BM_SimpleTokenization|BM_MuggedManScenario" --benchmark_min_time=0.1s

# Nested token performance
./benchmarkStevensTerminal --benchmark_filter="BM_NestedTokens|BM_Preprocess" --benchmark_min_time=0.2s

# Stress testing
./benchmarkStevensTerminal --benchmark_filter="BM_StressTest" --benchmark_min_time=0.5s

# Performance comparison (old vs new optimizations)
./benchmarkStevensTerminal --benchmark_filter="BM_InefficiestSubstring|BM_EfficientPatternMatch"

# Style inheritance benchmarks
./benchmarkStevensTerminal --benchmark_filter="BM_StyleInheritance"
```

### Rebuilding

```bash
# Clean rebuild
cd build
make clean
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Or completely clean
cd ..
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Benchmark Categories

### Basic Tokenization
- `BM_SimpleTokenization`: Tests single token parsing with varying content length
- `BM_MultipleTokens`: Tests multiple token parsing with varying token count

### Nested Token Processing
- `BM_NestedTokensShallow`: Tests shallow nesting (1-10 levels)
- `BM_NestedTokensDeep`: Tests deep nesting (1-25 levels)
- `BM_PreprocessNestedTokens`: Tests preprocessing performance
- `BM_PreprocessDeepNesting`: Tests preprocessing with very deep nesting

### Style Inheritance
- `BM_StyleInheritance_Simple`: Basic style inheritance
- `BM_StyleInheritance_Complex`: Complex style with many properties
- `BM_StyleInheritance_Scaling`: Scaling test with increasing style complexity

### Real-World Scenarios
- `BM_MuggedManScenario`: Real game text from CultGame
- `BM_ComplexStyleInheritanceScenario`: Complex nested styling scenario

### Stress Tests
- `BM_StressTest_VeryDeepNesting`: Extremely deep nesting (50 levels)
- `BM_StressTest_ManyTokens`: Large number of tokens (1000+)

### Performance Comparison
- `BM_InefficiestSubstring_Simulation`: Simulates old inefficient approach
- `BM_EfficientPatternMatch`: Tests optimized pattern matching

## Optimization Analysis

The benchmarks help measure the effectiveness of various optimizations:

1. **Pattern Matching**: Direct character comparison vs substring creation
2. **Memory Allocation**: Pre-allocation and reuse strategies
3. **String Operations**: Efficient string building and manipulation
4. **Recursion**: Optimized recursive processing for nested tokens

## Performance Expectations

On a modern system, you should expect:
- Simple tokenization: < 1µs per token
- Shallow nesting (≤5 levels): < 10µs
- Deep nesting (≤20 levels): < 100µs
- Style inheritance: < 1µs per operation

Significant deviations from these values may indicate performance regressions.

## Adding New Benchmarks

To add new benchmarks:

1. Add your benchmark function to `benchmark_terminal.cpp`
2. Follow the naming convention: `BM_CategoryName_SpecificTest`
3. Use `benchmark::DoNotOptimize()` to prevent compiler optimizations
4. Set appropriate complexity measurements with `state.SetComplexityN()`
5. Set bytes processed with `state.SetBytesProcessed()` when applicable

Example:
```cpp
static void BM_YourNewBenchmark(benchmark::State& state) {
    // Setup
    std::string input = generateTestData(state.range(0));
    
    for (auto _ : state) {
        // Code to benchmark
        auto result = functionToBenchmark(input);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetBytesProcessed(state.iterations() * input.length());
}
BENCHMARK(BM_YourNewBenchmark)->Range(1, 1000)->Complexity();
```