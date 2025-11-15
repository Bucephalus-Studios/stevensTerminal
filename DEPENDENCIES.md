# stevensTerminal Dependencies

## Overview

stevensTerminal bundles several utility libraries in the `bundled/` directory to provide a batteries-included experience. All bundled dependencies use include guards to prevent multiple definition errors.

## Bundled Libraries

The following libraries are included:

- **stevensStringLib** - String manipulation utilities
- **stevensMathLib** - Math and random number operations
- **stevensVectorLib** - Vector/array utilities
- **stevensMapLib** - Map/dictionary utilities
- **stevensFileLib** - File I/O helpers

## How Bundling Works

### Include Guards Protection

Each bundled library has include guards that prevent it from being included multiple times in the same compilation unit. For example:

```cpp
#ifndef STEVENS_BUNDLED_STRINGLIB_H
#define STEVENS_BUNDLED_STRINGLIB_H
// ... library code ...
#endif
```

### Version Conflict Handling

If you're using any of these libraries standalone in your project AND you include stevensTerminal, **the first version included wins**:

```cpp
// YOUR CODE
#include "stevensStringLib.h"        // Your standalone v2.0
#include "stevensTerminal.hpp"       // Bundles v1.5 internally

// Result: Your v2.0 is used (stevensTerminal's bundled v1.5 is skipped due to include guards)
```

### Best Practices

1. **If you don't use these libraries standalone**: Just `#include "stevensTerminal.hpp"` and you're good to go!

2. **If you use these libraries standalone**:
   - Include them BEFORE stevensTerminal.hpp to use your preferred versions
   - OR don't worry about it - the include guards will prevent conflicts (first included wins)

3. **If you need specific versions**: Include the standalone libraries first with your required versions

## Example Usage

### Simple Case (No Standalone Libraries)
```cpp
#include "stevensTerminal.hpp"

int main() {
    stevensTerminal::print("Hello World!");
    return 0;
}
```

### With Standalone Libraries
```cpp
// Include your preferred versions first
#include "my/path/to/stevensStringLib.h"  // Your v3.0
#include "my/path/to/stevensMathLib.h"    // Your v2.1

// Now include stevensTerminal (it will skip its bundled versions)
#include "stevensTerminal.hpp"

int main() {
    // stevensTerminal internally uses YOUR versions (v3.0, v2.1)
    // because they were included first
    stevensTerminal::print("Hello!");

    // You can also use them directly
    std::string result = stevensStringLib::cap1stChar("hello");
    return 0;
}
```

## Why Not Namespace Isolation?

You might wonder why we don't isolate bundled dependencies in a `stevensTerminal::bundled::` namespace. This approach was attempted but causes compilation issues because:

1. When you wrap `#include` in a namespace, it captures ALL includes (including standard library headers)
2. This creates conflicts like `stevensTerminal::bundled::std::vector` vs `std::vector`
3. The include guard approach is simpler, standard C++ practice, and works reliably

## Technical Details

### Include Order Matters

C++ processes includes top-to-bottom. The first time a library is included, its include guard is "opened" and the code is processed. Subsequent includes of the same library see the guard is already defined and skip the content.

### No Symbol Conflicts

Since the "first included wins" rule applies at the **preprocessor** level (before compilation), there are no linker symbol conflicts. You get exactly one version of each library per compilation unit.

### Recommendation for Library Maintainers

If you're building a library that depends on stevensTerminal's bundled libraries, you should:

1. Document which versions of the bundled libraries stevensTerminal uses
2. Test compatibility with those versions
3. Let users know they can override by including their preferred versions first

## Updating Bundled Libraries

To update a bundled library to a newer version:

1. Copy the new library header to `bundled/`
2. Ensure it has the correct include guards (see existing bundled headers for the pattern)
3. Test compilation
4. Update this documentation with the new version number

## Questions?

If you encounter version conflicts or unexpected behavior:

1. Check your include order (standalone libraries should come before stevensTerminal.hpp)
2. Verify all headers have proper include guards
3. Check for duplicate symbols at link time (rare, but possible if libraries are compiled separately)
