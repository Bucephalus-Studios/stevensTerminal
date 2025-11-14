# Comprehensive Refactoring Plan - Stevens Terminal Library

## Progress Summary

### ✅ Completed (Session 1)
1. **horizontalStackedBarGraph** - DONE
   - Extracted `calculateBarGraphLabel()` helper
   - Added `BarGraphLabelStyle` enum
   - Reduced nesting from 4 to 2 levels
   - Reduced size from 150 to 99 lines
   - Improved variable names throughout
   - All tests passing

### 🔄 In Progress
2. **Variable Naming Improvements**
   - Started replacing single-letter variables with descriptive names
   - Need to continue across all functions

### 📋 High Priority Remaining

#### Phase 2A: Color Handling (MEDIUM EFFORT, HIGH IMPACT)
**File:** `s_TerminalPrintHelper.h`

**Current Issue:** `curses_setup_colorPairs()` has 300+ lines of repetitive `init_pair()` calls

**Solution:** Data-driven approach
```cpp
struct ColorPairDefinition {
    int pairNum;
    int foreground;
    int background;
    std::vector<std::string> names; // All the names this pair is known by
};

std::vector<ColorPairDefinition> colorPairDefinitions = {
    {1, COLOR_RED, COLOR_BLACK, {"red", "red_on_black"}},
    {2, COLOR_GREEN, COLOR_BLACK, {"green", "green_on_black"}},
    // ... etc
};

void curses_setup_colorPairs() {
    for(const auto& def : colorPairDefinitions) {
        init_pair(def.pairNum, def.foreground, def.background);
        for(const auto& name : def.names) {
            curses_colors[name] = def.pairNum;
        }
    }
}
```

**Estimated Reduction:** 300 lines → ~50 lines

#### Phase 2B: Border Drawing (LOW EFFORT, MEDIUM IMPACT)
**File:** `stevensTerminal.hpp`
**Function:** `curses_wborder` (lines 1676-1836)

**Current Issue:** Four nearly identical sections for left/right/top/bottom borders

**Solution:** Generic helper function
```cpp
void drawBorderSide(
    WINDOW* win,
    const std::string& side,  // "left", "right", "top", "bottom"
    const std::unordered_map<std::string, std::string>& borderPatterns,
    int height,
    int width
) {
    // Generic implementation handles all four sides
}
```

**Estimated Reduction:** 160 lines → ~80 lines

#### Phase 2C: Grid Construction (MEDIUM EFFORT, HIGH IMPACT)
**Files:** `stevensTerminal.hpp`
**Functions:** `printVector`, `printVector_str`, `promptMultipleResponses`

**Current Issue:** Three separate implementations of 2D grid construction

**Solution:** Template helper
```cpp
template<typename T>
std::vector<std::vector<std::string>> constructGrid(
    const std::vector<T>& elements,
    int rows,
    int columns,
    bool columnFirst
) {
    // Generic grid construction logic
}
```

**Estimated Reduction:** ~100 lines of duplicated logic

### 📋 Critical Priority (Requires Dedicated Session)

#### Phase 3A: verticalMenus Refactoring (HIGH EFFORT, VERY HIGH IMPACT)
**File:** `stevensTerminal.hpp` (lines 421-761)
**Current Size:** 341 lines, 4+ nesting levels

**Breakdown Strategy:**

1. **Extract Layout Calculator**
```cpp
struct MenuLayout {
    int menusPerRow;
    int menuWidth;
    int charactersPerRow;
};

MenuLayout calculateMenuLayout(
    size_t numMenus,
    int availableSpace,
    const std::string& displayMode
);
```

2. **Extract Menu State**
```cpp
struct MenuRenderState {
    std::vector<int> itemOffsets;
    std::vector<int> itemsPrinted;
    std::vector<int> emptyMenuIndices;
    std::vector<std::queue<std::string>> wrappedText;
    std::vector<std::queue<std::string>> queuedItems;
    int currentResponseNum;
};
```

3. **Extract Rendering Functions**
```cpp
void renderMenuLabels(
    const std::vector<std::string>& labels,
    const MenuLayout& layout
);

void renderMenuItem(
    const std::string& item,
    int responseNum,
    int availableWidth,
    MenuRenderState& state
);

void renderMenuBorder(
    const std::string& pattern,
    int position
);
```

**Estimated Result:** 341 lines → ~150 lines main function + 200 lines helpers

#### Phase 3B: printVector_str Refactoring (HIGH EFFORT, HIGH IMPACT)
**File:** `stevensTerminal.hpp` (lines 1042-1375)
**Current Size:** 333 lines

**Breakdown Strategy:**

1. **Extract Format Parser**
```cpp
struct VectorFormatOptions {
    int columns = -1;
    int rows = -1;
    std::string sequence = "column first";
    bool allowOverflow = false;
    std::string listType = "bulleted";
    // ... all other options
};

VectorFormatOptions parseVectorFormat(
    const std::unordered_map<std::string, std::string>& format
);
```

2. **Extract Grid Builder**
```cpp
template<typename T>
std::vector<std::vector<std::string>> buildElementGrid(
    const std::vector<T>& vec,
    const VectorFormatOptions& options
);
```

3. **Extract Column Width Calculator**
```cpp
std::vector<size_t> calculateColumnWidths(
    const std::vector<std::vector<std::string>>& grid,
    const VectorFormatOptions& options
);
```

**Estimated Result:** 333 lines → ~120 lines main + 200 lines helpers

#### Phase 3C: tokenizePrintString Refactoring (VERY HIGH EFFORT, VERY HIGH IMPACT)
**File:** `s_TerminalPrintHelper.h` (lines 751-981)
**Current Size:** 230 lines with complex state machine

**Breakdown Strategy:**

1. **Extract State Machine**
```cpp
enum class TokenizerState {
    SearchingForBrace,
    InBraceContent,
    InStyleDirective,
    ProcessingEscape,
    Complete
};

class TokenParser {
private:
    TokenizerState state;
    std::string currentToken;
    int braceDepth;

public:
    void processCharacter(char c);
    bool isComplete();
    s_TerminalPrintToken getToken();
};
```

2. **Extract Index Adjustment**
```cpp
void adjustTokenIndices(
    std::vector<s_TerminalPrintToken>& tokens,
    size_t modificationPoint,
    int lengthDifference
);
```

3. **Extract Style Inheritance**
```cpp
void applyParentStylesToChildren(
    std::vector<s_TerminalPrintToken>& tokens
);
```

**Estimated Result:** 230 lines → ~100 lines orchestration + 150 lines helpers

### 📋 Medium Priority

#### Phase 4A: Improve Variable Naming Throughout
**Scope:** All files

**Single-Letter Variables to Replace:**
- `i, j, k` → `index`, `rowIndex`, `columnIndex`, `labelIndex`
- `n` → `count`, `tokenIndex`, `nestedLevel`
- `s` → `charAsString`, `singleChar`
- `e` → `elementIndex`, `entryIndex`
- `h, w` → `height`, `width`

**Estimated Time:** 2-3 hours systematic replacement

#### Phase 4B: Extract Magic Constants
**Current Issues:**
- Line 449: `18` → `const int DEFAULT_MENU_WIDTH = 18`
- Line 103: `"none"` → `const std::string NO_LABEL = ""`
- Multiple uses of `80`, `24` for screen sizes

**Solution:** Create constants section at top of namespace

### 📋 Low Priority (Polish)

#### Phase 5A: Add const-correctness
- Mark parameters that shouldn't change as `const`
- Use `const` references where appropriate
- Mark member functions as `const` where applicable

#### Phase 5B: Remove Dead Code
- `word` variable in `wrap()` function (line 348) - unused
- Commented-out code sections
- Unreachable code after early returns

#### Phase 5C: Fix Typos
- Line 1761: "unordereD_map" → "unordered_map"

### 📊 Estimated Impact Summary

| Refactoring | Lines Saved | Complexity Reduction | Risk Level |
|-------------|-------------|---------------------|------------|
| horizontalStackedBarGraph | 51 lines | 4→2 nesting | ✅ DONE |
| Color pairs data-driven | ~250 lines | Major | LOW |
| Border drawing | ~80 lines | Minor | LOW |
| Grid construction | ~100 lines | Medium | MEDIUM |
| Variable naming | 0 lines | Major readability | LOW |
| verticalMenus breakdown | ~150 saved | 4→2 nesting | HIGH |
| printVector_str breakdown | ~130 saved | 3→2 nesting | MEDIUM |
| tokenizePrintString breakdown | ~80 saved | State machine clarity | HIGH |

**Total Estimated Reduction:** ~840 lines (9%)
**Total Complexity Reduction:** Massive improvement in maintainability

### 🎯 Recommended Next Session Plan

1. **Session 2:** Focus on low-risk, high-value improvements
   - Complete variable naming improvements (all files)
   - Data-driven color pairs
   - Extract border drawing helper
   - Extract grid construction helper
   - Commit frequently, test after each change

2. **Session 3:** Tackle the big three
   - One dedicated session per function:
     - verticalMenus
     - printVector_str
     - tokenizePrintString
   - Create comprehensive tests for each before refactoring
   - Refactor incrementally with tests running continuously

3. **Session 4:** Polish and documentation
   - Add const-correctness
   - Remove dead code
   - Update README with new structure
   - Create architecture diagram showing new organization

### 📝 Notes

- **Test Coverage:** Currently 53 tests, all passing
- **Performance:** Benchmarks show no regressions after horizontalStackedBarGraph refactor
- **Backwards Compatibility:** All refactorings maintain existing API
- **Risk Mitigation:** Frequent commits, continuous testing

### 🚀 Success Criteria

- ✅ No function exceeds 2 levels of nesting
- ✅ No function exceeds 150 lines
- ✅ All single-letter variables replaced with descriptive names
- ✅ All magic strings/numbers extracted to constants
- ✅ No code duplication (DRY principle)
- ✅ All 53+ tests passing
- ✅ No performance regressions in benchmarks
- ✅ Improved documentation for all refactored functions

---

**Last Updated:** 2025-11-14
**Status:** Phase 1 partially complete, continuing...
