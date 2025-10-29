# Tula

Tula is a shared utility library for the [TolTEC project](http://toltec.astro.umass.edu).

Please see the [API documentation](https://toltec-astro.github.io/tula) for details.

## Features

- **Modern C++23** - Uses latest C++ standards and idioms
- **Header-only core** - Easy integration with precompiled headers
- **Tri-modal package management** - Automatic fallback: CONAN → CPM → SYSTEM
- **Unified cache** - All dependencies cached in `~/.tula_cache`
- **CMake 4.1+** - Modern CMake practices
- **Modular design** - Use sensible defaults without dependency management

## Build & Install

### Quick Start

```bash
# Configure with default preset (CONAN mode, auto-fallback to CPM/SYSTEM)
cmake --preset brew-llvm-debug

# Build
cmake --build build_brew-llvm-debug

# Run tests
ctest --test-dir build_brew-llvm-debug
```

### Available Presets

- `brew-llvm-{release,debug}` - Homebrew LLVM/Clang compiler
- `brew-gcc-15-{release,debug}` - Homebrew GCC 15 compiler

All presets use automatic package mode fallback (CONAN → CPM → SYSTEM).

## tula_cmake Architecture

The `tula_cmake` system consists of two modular files:

### `tula_sensible.cmake` - Sensible Build Defaults (Standalone)

Public-facing file providing sensible build configuration and utilities. **Can be used independently** without dependency management.

**What it provides:**
- CMake version check (4.1+)
- Utility module path setup (`utils/`)
- Build-in-source guard
- Build type detection
- RPATH configuration
- Output directory setup (`lib/`, `bin/`)
- C++23 language settings
- Platform-specific compiler configuration (macOS/Linux)
- Build summary messages

**Usage (standalone):**
```cmake
# Use only sensible defaults without dependency management
project(MyProject LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${TULA_CMAKE_DIR}" ${CMAKE_MODULE_PATH})
include(tula_sensible)
# Now you have all sensible defaults applied
```

### `tula_cmake.cmake` - Tri-Modal Dependency Management

Extends `tula_sensible` with automatic dependency resolution. **Use this for tula projects** that need the dependency system.

**Additional features:**
- Tri-modal package resolution (CONAN → CPM → SYSTEM)
- Target registration and creation
- Conan integration (programmatic `conan install`)
- Per-package mode override
- Unified cache directory configuration

**Usage (full system):**
```cmake
project(MyProject LANGUAGES CXX)

include(tula_cmake)          # Includes tula_sensible automatically
include(Eigen3)              # Register dependency
tula_deps_create_targets()   # Resolve and create targets
```

## Configuration Variables

### Cache Configuration

| Variable | Default | Description |
|----------|---------|-------------|
| `TULA_CACHE_ROOT` | `$HOME/.tula_cache` | Root directory for all tula cache files |
| `CPM_SOURCE_CACHE` | `${TULA_CACHE_ROOT}/cpm` | CPM source cache (git repositories) |

**Note:** Conan uses system default cache (`~/.conan2`) - configure with `conan config home`

**Example:**
```bash
# Use custom cache location
cmake --preset brew-llvm-debug -DTULA_CACHE_ROOT=/custom/cache
```

### Dependency Resolution Modes

The dependency system automatically tries modes in this order:

1. **CONAN** - Runs `conan install` programmatically, includes generated toolchain
2. **CPM** - Downloads from GitLab/GitHub into `~/.tula_cache/cpm`
3. **SYSTEM** - Uses `find_package()` with system-installed libraries

#### Per-Package Mode Override

Force a specific package to use a particular acquisition mode:

| Variable | Values | Description |
|----------|--------|-------------|
| `TULA_<PACKAGE>_MODE` | `CONAN`, `CPM`, `SYSTEM` | Force specific mode for package |

**Examples:**
```bash
# Force Eigen3 to use CPM (skip CONAN)
cmake --preset brew-llvm-debug -DTULA_Eigen3_MODE=CPM

# Force Eigen3 to use SYSTEM (skip CONAN and CPM)
cmake --preset brew-llvm-debug -DTULA_Eigen3_MODE=SYSTEM

# Test Conan failure handling (should fall back to CPM)
cmake --preset brew-llvm-debug -DCONAN_COMMAND=/nonexistent/conan

# Force mode will FATAL_ERROR if the forced mode fails
cmake --preset brew-llvm-debug \
  -DCONAN_COMMAND=/nonexistent/conan \
  -DTULA_Eigen3_MODE=CONAN  # This will error, not fall back
```

### Build Options

| Variable | Default | Description |
|----------|---------|-------------|
| `TULA_BUILD_TESTS` | `ON` (standalone) | Build test suite |
| `TULA_BUILD_EXAMPLES` | `OFF` | Build example programs |
| `TULA_BUILD_DOC` | `OFF` | Build documentation |

**Example:**
```bash
# Build with tests and examples
cmake --preset brew-llvm-debug -DTULA_BUILD_EXAMPLES=ON
```

### Standalone vs Downstream

| Variable | Auto-detected | Description |
|----------|---------------|-------------|
| `TULA_STANDALONE` | `${CMAKE_SOURCE_DIR} == ${CMAKE_CURRENT_SOURCE_DIR}` | Whether building tula standalone or as subdirectory |

In **standalone mode**:
- `tula_all` target includes all modules for comprehensive testing
- Tests, examples, and docs can be enabled
- All dependencies (including MPI, FITS) are built

In **downstream mode** (when used as subdirectory):
- Only `tula::headers` is provided
- Downstream project includes specific modules as needed
- No tests/examples/docs built

**Example downstream usage:**
```cmake
# In your CMakeLists.txt
add_subdirectory(tula)

# Include only what you need
include(Eigen3)
include(NetCDFCXX4)

target_link_libraries(my_target PRIVATE 
    tula::headers
    tula::Eigen3
    tula::NetCDFCXX4
)
```

## Available Modules

### Core Libraries (Target Files)

**Updated to v3 (naming convention):**
- ✅ **Eigen3** - Linear algebra

**Legacy (callback-based, pending migration):**
- ⏳ **bitmask** - Bitmask operations
- ⏳ **CCfits** - FITS file I/O for astronomy
- ⏳ **Ceres** - Nonlinear optimization
- ⏳ **FFTW** - Fast Fourier Transform
- ⏳ **fmt** - Formatting library
- ⏳ **logging** - Logging stack (spdlog)
- ⏳ **MXX** - Modern MPI C++ wrapper
- ⏳ **NetCDF** - NetCDF C library
- ⏳ **NetCDFCXX4** - NetCDF C++ bindings
- ⏳ **perflibs** - Performance libraries (OpenMP/MKL/Threads)
- ⏳ **Re2** - Google RE2 regular expressions
- ⏳ **Spectra** - Eigenvalue solver
- ⏳ **testing** - Testing stack (GTest/Benchmark)
- ⏳ **Yaml** - YAML parser (yaml-cpp)

See `tula_cmake/cmake/targets/README.md` for migration status and templates.

## Target File Structure (v3 Architecture)

Each package is defined in a target file (e.g., `Eigen3.cmake`) with naming convention functions:

```cmake
# TULA_{PACKAGE}_TRY_CONAN() - Try to find via Conan
function(TULA_Eigen3_TRY_CONAN)
    # Search CMAKE_INCLUDE_PATH set by conan_toolchain.cmake
    # Set TULA_Eigen3_CONAN_SUCCESS=TRUE/FALSE
endfunction()

# TULA_{PACKAGE}_TRY_CPM() - Try to download via CPM
function(TULA_Eigen3_TRY_CPM)
    # Download from GitLab/GitHub
    # Set TULA_Eigen3_CPM_SUCCESS=TRUE/FALSE
endfunction()

# TULA_{PACKAGE}_TRY_SYSTEM() - Try to find system-installed
function(TULA_Eigen3_TRY_SYSTEM)
    # Use find_package(Eigen3 CONFIG)
    # Set TULA_Eigen3_SYSTEM_SUCCESS=TRUE/FALSE
endfunction()

# TULA_{PACKAGE}_CREATE_WRAPPER() - Create tula::Package target
function(TULA_Eigen3_CREATE_WRAPPER)
    # Wraps upstream Eigen3::Eigen with tula-specific configuration
    make_tula_target(Eigen3 INTERFACE 
        LIBRARIES Eigen3::Eigen tula::perflibs)
endfunction()

# Register package for resolution
tula_deps_register(Eigen3)
```

**Two-Phase Workflow:**

1. **Registration** - Include target files to register packages:
   ```cmake
   include(Eigen3)
   include(NetCDF)
   ```

2. **Resolution** - Create all targets in one batch:
   ```cmake
   tula_deps_create_targets()  # Runs conan install, then tries modes for each package
   ```

### Acquisition Modes

#### CONAN Mode (Default First Try)

Runs `conan install` programmatically with `-o eigen3=True` options. Includes generated `conan_toolchain.cmake`.

**Pros:** Prebuilt binaries, fast, version-controlled (conanfile.py)  
**Cons:** Requires Conan 2.x installed

#### CPM Mode (Default Second Try)

Uses CPM.cmake to download and build packages from source.

**Pros:** No external tools needed, always latest versions  
**Cons:** Longer build times (first build only - cached in `~/.tula_cache/cpm`)

#### SYSTEM Mode (Default Last Try)

Uses system-installed packages via `find_package()`.

**Setup (macOS):**
```bash
brew install eigen yaml-cpp netcdf netcdf-cxx
```

**Setup (Ubuntu):**
```bash
sudo apt install libeigen3-dev libyaml-cpp-dev libnetcdf-dev libnetcdf-c++4-dev
```

**Pros:** Uses existing installations, no downloads  
**Cons:** Version inconsistencies, manual installation required

## Cache Management

All downloaded dependencies are cached in `~/.tula_cache`:

```
~/.tula_cache/
└── cpm/          # CPM source cache (git repositories)
```

**Note:** Conan uses system default cache (`~/.conan2`)

**Benefits:**
- Shared across all tula projects
- Survives build directory deletion
- Reduces download/compile time

**Clean cache:**
```bash
# Remove all cached dependencies
rm -rf ~/.tula_cache

# Remove only CPM cache
rm -rf ~/.tula_cache/cpm

# Remove Conan cache (separate from tula cache)
rm -rf ~/.conan2
```

## License

This project is Copyright (c) Zhiyuan Ma and licensed under the terms of the BSD 3-Clause license. See the licenses folder for more information.
