# stevensTerminal Dependencies

## Overview

stevensTerminal uses **git submodules** to manage its library dependencies. This provides automatic version tracking, easy updates, and ensures everyone gets the exact same library versions.

## Dependencies

The following libraries are included as submodules in the `dependencies/` directory:

| Library | Repository | Description |
|---------|-----------|-------------|
| **stevensStringLib** | [github.com/Bucephalus-Studios/stevensStringLib](https://github.com/Bucephalus-Studios/stevensStringLib) | String manipulation utilities |
| **stevensMathLib** | [github.com/Bucephalus-Studios/stevensMathLib](https://github.com/Bucephalus-Studios/stevensMathLib) | Math and random number operations |
| **stevensVectorLib** | [github.com/Bucephalus-Studios/stevensVectorLib](https://github.com/Bucephalus-Studios/stevensVectorLib) | Vector/array utilities |
| **stevensMapLib** | [github.com/Bucephalus-Studios/stevensMapLib](https://github.com/Bucephalus-Studios/stevensMapLib) | Map/dictionary utilities |
| **stevensFileLib** | [github.com/Bucephalus-Studios/stevensFileLib](https://github.com/Bucephalus-Studios/stevensFileLib) | File I/O helpers |

---

## For Users: Cloning stevensTerminal

### Option 1: Clone with Submodules (Recommended)

```bash
git clone --recursive https://github.com/Bucephalus-Studios/stevensTerminal.git
```

The `--recursive` flag automatically initializes and clones all submodules.

### Option 2: Clone Then Initialize Submodules

If you forgot `--recursive`:

```bash
git clone https://github.com/Bucephalus-Studios/stevensTerminal.git
cd stevensTerminal
git submodule update --init --recursive
```

### Verifying Submodules

Check that dependencies are loaded:

```bash
ls dependencies/
# Should show: stevensFileLib  stevensMapLib  stevensMathLib  stevensStringLib  stevensVectorLib
```

---

## For Contributors: Working with Submodules

### Updating to Latest Dependency Versions

To update all dependencies to their latest versions:

```bash
git submodule update --remote
git add dependencies/
git commit -m "Update dependencies to latest versions"
git push
```

To update a specific library:

```bash
git submodule update --remote dependencies/stevensStringLib
git add dependencies/stevensStringLib
git commit -m "Update stevensStringLib to latest version"
git push
```

### Checking Submodule Status

```bash
git submodule status
```

This shows the current commit hash of each submodule.

### Viewing Submodule Changes

```bash
# See which commit each submodule is at
git diff --submodule

# See changes within a specific submodule
cd dependencies/stevensStringLib
git log
cd ../..
```

---

## How It Works

### Include Guard Protection

Each library uses include guards to prevent multiple definition errors:

```cpp
#ifndef STEVENSSTRINGLIB_H
#define STEVENSSTRINGLIB_H
// ... library code ...
#endif
```

### Version Locking

Git submodules lock to specific commits. This means:

- ✅ **Reproducible builds** - Everyone gets the exact same library versions
- ✅ **No surprise breaking changes** - Updates are explicit and controlled
- ✅ **Version tracking** - Git tracks which version of each library you're using

### Core.hpp Includes

stevensTerminal's Core.hpp includes dependencies like this:

```cpp
#include "dependencies/stevensStringLib/stevensStringLib.h"
#include "dependencies/stevensMathLib/stevensMathLib.h"
#include "dependencies/stevensMapLib/stevensMapLib.hpp"
#include "dependencies/stevensVectorLib/stevensVectorLib.hpp"
#include "dependencies/stevensFileLib/stevensFileLib.hpp"
```

---

## Compilation

### With Make or Direct g++

```bash
g++ -std=c++20 -I. -I dependencies/stevensStringLib \
    -I dependencies/stevensMathLib \
    -I dependencies/stevensVectorLib \
    -I dependencies/stevensMapLib \
    -I dependencies/stevensFileLib \
    your_file.cpp -lncurses
```

### With CMake

CMake automatically handles submodule paths. Just:

```bash
cmake -B build
cmake --build build
```

---

## Troubleshooting

### Error: "No such file or directory" for dependency headers

**Cause:** Submodules weren't initialized

**Solution:**
```bash
git submodule update --init --recursive
```

### Submodule shows "dirty" or modified

**Cause:** You or a tool made changes inside a submodule directory

**Solution:**
```bash
# Discard changes in submodule
cd dependencies/stevensStringLib
git checkout .
cd ../..
```

### Want to use a different version of a library

**Option 1:** Modify the submodule (not recommended for users):
```bash
cd dependencies/stevensStringLib
git checkout v2.0.0  # or any commit/branch/tag
cd ../..
git add dependencies/stevensStringLib
git commit -m "Pin stevensStringLib to v2.0.0"
```

**Option 2:** Fork stevensTerminal and modify the submodule references

---

## Advantages of Git Submodules

✅ **Easy updates** - `git submodule update --remote` gets latest versions
✅ **Version locking** - Exact dependency versions are tracked in git
✅ **Single source of truth** - Points to the actual library repositories
✅ **Low maintenance** - No manual file copying
✅ **Transparent** - Users see exactly what versions are used

---

## For Maintainers: Adding/Removing Submodules

### Adding a New Dependency

```bash
git submodule add https://github.com/Bucephalus-Studios/newLib.git dependencies/newLib
git commit -m "Add newLib dependency as submodule"
git push
```

Then update Core.hpp to include it.

### Removing a Dependency

```bash
git submodule deinit dependencies/oldLib
git rm dependencies/oldLib
git commit -m "Remove oldLib dependency"
git push
```

---

## Questions?

**Q: Can I use stevensTerminal without git?**
A: Yes, but you'll need to manually download all dependency repositories and place them in the `dependencies/` directory.

**Q: What if I want to modify a dependency library?**
A: Fork the library repository, make your changes there, then update the submodule URL to point to your fork.

**Q: Do submodules affect compile time?**
A: No, they're just a way to organize source code. Compilation is the same as if you copied the files manually.

**Q: Can I use different versions for different projects?**
A: Yes! Each project can pin to different commits of the submodules. That's the beauty of git submodules.
