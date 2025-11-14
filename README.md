# Stevens Terminal Library

A modern, feature-rich C++ library providing advanced terminal UI capabilities with cross-platform support for ncurses (Linux/macOS) and pdcurses (Windows).

## Features

### Core Capabilities

- **Advanced Text Styling**: Innovative token-based styling system with nested style support
- **Cross-Platform**: Works on Linux/macOS (ncurses) and Windows (pdcurses)
- **UI Components**: Menus, tables, graphs, and borders
- **Window Management**: Efficient RAII-based window lifecycle with double-buffering
- **Input Handling**: Validated user input with range checking
- **Comprehensive Utilities**: String, vector, map, math, and file I/O libraries

### Styling System

The library features a unique token-based styling system that allows inline text styling:

```cpp
// Simple styling
stevensTerminal::print("{Hello World}$[textColor=red,bgColor=yellow]");

// Nested styling with inheritance
stevensTerminal::print("{Outer {inner text}$[textColor=red]}$[bgColor=blue]");
```

**Supported Style Properties:**
- Text colors: `red`, `orange`, `yellow`, `green`, `blue`, `cyan`, `purple`, `black`, `grey`, `white`, `default`
- Background colors: Same as text colors
- Attributes: `bold`, `blink`

### UI Components

#### Horizontal Stacked Bar Graphs
```cpp
std::vector<std::string> labels = {"Success", "Warning", "Error"};
std::vector<std::tuple<std::string,std::string>> colors = {
    {"green", "default"},
    {"yellow", "default"},
    {"red", "default"}
};
std::vector<float> distribution = {70.0f, 20.0f, 10.0f};

stevensTerminal::horizontalStackedBarGraph(labels, colors, distribution, 80, true, true);
```

#### Vertical Menus
```cpp
std::vector<std::vector<std::string>> menus = {
    {"Option 1", "Option 2", "Option 3"},
    {"Choice A", "Choice B"},
    {"Item X", "Item Y", "Item Z"}
};

int selection = stevensTerminal::verticalMenus(menus);
```

#### Tables
```cpp
std::vector<std::vector<std::string>> table = {
    {"Name", "Age", "City"},
    {"Alice", "30", "NYC"},
    {"Bob", "25", "LA"}
};

std::unordered_map<std::string, std::string> format;
format["column width"] = "20";

std::string formatted = stevensTerminal::formatTableAsString(table, format);
std::cout << formatted;
```

### Utility Libraries

#### String Library (`stevensStringLib`)
- String searching: `contains()`, `findAll()`, `count()`
- Transformation: `reverse()`, `uppercase()`, `lowercase()`, `trim()`
- Splitting/joining: `separate()`, `join()`
- Validation: `isNumber()`, `isAlpha()`, `isAlphanumeric()`

#### Vector Library (`stevensVectorLib`)
- Searching: `contains()`, `indexOf()`, `findIndex()`
- Transformation: `map()`, `filter()`, `flatten()`
- Statistics: `sum()`, `average()`, `max()`, `min()`

#### Map Library (`stevensMapLib`)
- Key/value operations
- Map transformation and merging
- Sorted vector conversion

#### Math Library (`stevensMathLib`)
- Random number generation
- Rounding functions
- Clamp operations

#### File Library (`stevensFileLib`)
- File reading/writing
- Directory operations
- Content processing

## Building

### Requirements

- **C++20 or later** (C++23 for benchmarks)
- **CMake 3.22.1+**
- **ncurses** (Linux/macOS) or **pdcurses** (Windows)
- **Google Test** (automatically downloaded via FetchContent)
- **Google Benchmark** (automatically downloaded via FetchContent)

### Build Tests

```bash
cd testing
mkdir build && cd build
cmake ..
make
./testStevensTerminal
```

### Build Benchmarks

```bash
cd benchmarking
mkdir build && cd build
cmake ..
make
./benchmarkStevensTerminal
```

### Using in Your Project

#### Header-Only Usage

Simply include the main header:

```cpp
#include "stevensTerminal.hpp"

// Use the library
stevensTerminal::print("{Styled text}$[textColor=green]");
```

#### CMake Integration

```cmake
# Add stevensTerminal to your project
include_directories(path/to/stevensTerminal)

# Link ncurses
find_package(Curses REQUIRED)
target_link_libraries(your_target ${CURSES_LIBRARIES})
```

## API Overview

### Core Functions

#### Printing
```cpp
// Basic styled printing
void print(std::string input,
           std::unordered_map<std::string, std::string> style = {},
           std::unordered_map<std::string, std::string> format = {});

// Print to ncurses window
void curses_wprint(WINDOW* win, std::string printString, ...);

// Print file contents
void printFile(std::string textFilePath, ...);
```

#### Input Handling
```cpp
// Get validated string input
std::string input();

// Get integer input within range
int input(int responseRange);

// Validate input is within range
bool inputWithinResponseRange(std::string response, int responseRange);
```

#### Styling Helpers
```cpp
// Add style token to string
std::string addStyleToken(std::string str,
                         std::unordered_map<std::string,std::string> styleMap);

// Remove all styling
std::string removeAllStyleTokenization(std::string str);

// Resize while preserving styles
std::string resizeStyledString(std::string str, size_t desiredLength, char fillChar = ' ');
```

### Window Management

```cpp
// Get the window manager singleton
WindowManager& windowManager();

// Initialize a window
WINDOW* win = windowManager().initialize("myWindow", height, width, startY, startX);

// Configure window layout
windowManager().configureLayout(layoutSpecs);

// Refresh all windows efficiently
windowManager().refreshAll();

// Cleanup
windowManager().shutdown();
```

### Display Modes

The library supports adaptive display modes based on terminal size:

```cpp
// Set display mode automatically
stevensTerminal::setDisplayMode();

// Check display mode constraints
bool stevensTerminal::displayMode_GTEminSize(std::string displayMode);
bool stevensTerminal::displayMode_LTEmaxSize(std::string displayMode);
```

**Built-in modes:**
- `"very small"`: < 24 columns
- `"small"`: 24-79 columns
- `"regular"`: 80+ columns

## Testing

The library includes 53+ comprehensive unit tests covering:
- Token parsing and nesting (up to 20 levels deep)
- Style inheritance
- Input validation
- Error handling
- Edge cases and boundary conditions

Run tests:
```bash
cd testing/build
./testStevensTerminal
```

Example test output:
```
[==========] Running 53 tests from 14 test suites.
...
[  PASSED  ] 53 tests.
```

## Benchmarking

Performance benchmarks measure:
- Token parsing speed
- Nested token complexity
- String manipulation performance
- Memory usage

Run benchmarks:
```bash
cd benchmarking/build
./benchmarkStevensTerminal
```

Example output:
```
BM_SimpleTokenization/8          1383 ns    bytes_per_second=19.28Mi/s
BM_SimpleTokenization/64         1396 ns    bytes_per_second=60.82Mi/s
BM_SimpleTokenization/512        1985 ns    bytes_per_second=272.16Mi/s
```

## Examples

### Example 1: Styled Menu

```cpp
#include "stevensTerminal.hpp"

int main() {
    stevensTerminal::textStyling = true;

    std::vector<std::vector<std::string>> menus = {
        {
            "{Main Menu}$[textColor=bright-yellow,bold=true]",
            "{Option 1}$[textColor=green]",
            "{Option 2}$[textColor=blue]",
            "{Exit}$[textColor=red]"
        }
    };

    int choice = stevensTerminal::verticalMenus(menus);
    std::cout << "You selected: " << choice << std::endl;

    return 0;
}
```

### Example 2: Progress Bar

```cpp
#include "stevensTerminal.hpp"

int main() {
    stevensTerminal::textStyling = true;

    std::vector<std::string> labels = {"Complete", "Remaining"};
    std::vector<std::tuple<std::string,std::string>> colors = {
        {"green", "default"},
        {"grey", "default"}
    };

    // 75% complete
    std::vector<float> progress = {75.0f, 25.0f};

    stevensTerminal::horizontalStackedBarGraph(labels, colors, progress, 60, true, true);

    return 0;
}
```

### Example 3: Window Management

```cpp
#include "stevensTerminal.hpp"

int main() {
    // Initialize ncurses
    WindowManager& wm = stevensTerminal::windowManager();

    // Create windows
    WINDOW* header = wm.initialize("header", 3, 80, 0, 0);
    WINDOW* content = wm.initialize("content", 20, 80, 3, 0);

    // Print to windows
    stevensTerminal::curses_wprint(header, "{Application Title}$[textColor=bright-cyan,bold=true]");
    stevensTerminal::curses_wprint(content, "Welcome to the application!");

    // Refresh all
    wm.refreshAll();

    // Wait for input
    getch();

    // Cleanup
    wm.shutdown();

    return 0;
}
```

## Recent Improvements (2025-11-14)

### Bug Fixes
- Fixed critical `if` statement semicolon bugs (5 instances) that caused logic errors
- Added division-by-zero protection in `horizontalStackedBarGraph`
- Removed debug output statements from production code

### Error Handling
- Added input validation for vector size mismatches
- Added exception handling for `std::stoi` conversions
- Added bounds checking for width parameters
- Improved error messages with descriptive context

### Build System
- Migrated from local dependencies to CMake FetchContent
- Simplified build process (no manual dependency downloads)
- Automatic version tracking for Google Test and Google Benchmark

### Testing & Quality
- Expanded test suite from 49 to 53+ tests
- Added tests for new error handling
- Added edge case and boundary condition tests
- Added benchmarks for utility library functions

## Architecture

### Header Organization

```
stevensTerminal/
├── Core.hpp              # Platform detection, initialization, display modes
├── Input.hpp             # User input validation
├── Styling.hpp           # Style token manipulation
├── stevensTerminal.hpp   # Main header (includes all modules)
├── classes/
│   ├── WindowManager.hpp           # RAII window management
│   ├── s_TerminalPrintToken.h      # Style token data structure
│   ├── s_TerminalPrintTokenHelper.hpp  # Token parsing engine
│   └── s_TerminalPrintHelper.h     # Core rendering engine
└── libraries/
    ├── stevensStringLib.h   # String utilities
    ├── stevensVectorLib.hpp # Vector operations
    ├── stevensMapLib.hpp    # Map operations
    ├── stevensMathLib.h     # Math utilities
    └── stevensFileLib.hpp   # File I/O
```

### Design Principles

- **Header-Only**: Easy integration, no linking required
- **Namespace Isolation**: All functions in `stevensTerminal` namespace
- **RAII**: Automatic resource management (WindowManager)
- **Type Safety**: Template-based utilities where appropriate
- **Cross-Platform**: Conditional compilation for platform differences

## Performance

The library is optimized for:
- **Efficient Tokenization**: O(n) parsing with minimal allocations
- **Double-Buffering**: Flicker-free window updates
- **Window Reuse**: WindowManager pools windows to avoid recreate overhead
- **Character-Level Matching**: Avoids expensive substring operations

Benchmark results show linear complexity (O(n)) for most operations with excellent throughput (272+ MB/s for token processing).

## License

MIT License - Copyright (c) 2025 Bucephalus-Studios

See LICENSE file for full text.

## Contributing

This library is under active development. Areas for future improvement:
- True color (24-bit) support
- Mouse input handling
- Async/event-driven capabilities
- Additional UI widgets
- Extended platform support

## Support

For issues, questions, or contributions, please contact Bucephalus Studios.
