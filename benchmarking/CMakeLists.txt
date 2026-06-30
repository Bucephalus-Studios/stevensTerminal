add_executable(stevensTerminal_benchmarks
    benchmark_terminal.cpp
)
target_link_libraries(stevensTerminal_benchmarks
    PRIVATE
        stevensTerminal
        benchmark::benchmark
)