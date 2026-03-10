# Tula

Tula is a shared utility library for the [TolTEC project](http://toltec.astro.umass.edu).

## Features

- **Modern C++23** — header-only core, compiles with Clang 20 / GCC 14+
- **Conan-centric build** — single `conan install` resolves all deps and generates the full CMake toolchain
- **Tri-modal dependency resolution** — CONAN → CPM → SYSTEM, per-package, with automatic fallback
- **`tula::tula` target** — downstream projects link to one target; no `add_subdirectory(tula)` required
- **CMake 4.1+** modern practices

---

## Quick Start

```bash
# 1. Install deps and generate toolchain
conan install . \
  --profile=tula_cmake/profiles/linux-clang20-debug \
  --build=missing \
  -o Eigen3=auto -o logging=auto -o testing=auto

# 2. Configure
cmake --preset conan-default

# 3. Build
cmake --build build/clang20-debug

# 4. Test
ctest --test-dir build/clang20-debug
```

---

## v3 Architecture Overview

### Toolchain-Centric Design

In tula v3, the **Conan toolchain is the single source of truth**. All dependency resolution happens during the `conan install` step, before CMake's `project()` call returns. Downstream projects consume `tula::tula` directly with no `add_subdirectory` needed.

```
conan install .  →  TulaConan.generate()
                      ├── CMakeDeps (find_package files)
                      └── CMakeToolchain
                            ├── tula_setup block
                            │     ├── include(tula_sensible.cmake)
                            │     └── include(tula_deps.cmake)
                            ├── tula_<Pkg> blocks (one per enabled package)
                            │     ├── set TULA_<PKG>_MODE / cmake vars
                            │     ├── include(<Pkg>.cmake)
                            │     └── tula_deps_add(TULA_DEPS <Pkg>)
                            └── tula_target block
                                  ├── tula_headers  INTERFACE → tula/include/
                                  ├── tula::headers ALIAS tula_headers
                                  ├── tula_all      INTERFACE → tula::headers + ${TULA_DEPS}
                                  └── tula::tula    ALIAS tula_all
```

### Key Invariant

`tula::tula` is always available in the CMake toolchain after `conan install`. Downstream `CMakeLists.txt` is minimal:

```cmake
# kidscpp / tula_example
cmake_minimum_required(VERSION 4.1)
project(MyProject LANGUAGES CXX)
# tula::tula provided by conan_toolchain.cmake
target_link_libraries(mytarget PRIVATE tula::tula)
```

### Tri-Modal Resolution

Each package supports a subset of three modes, tried in order:

| Mode | Description | Trigger |
|------|-------------|---------|
| **CONAN** | Prebuilt binary from Conan Center | `conan install` with package option `=conan` or `=auto` |
| **CPM** | Source download via CPM.cmake | option `=cpm`; CPM cache at `~/.tula_cache/cpm` |
| **SYSTEM** | `find_package()` against system-installed libs | option `=system` |

Force a mode per-package with Conan options:
```bash
conan install . -o Eigen3=cpm      # Force CPM for Eigen3
conan install . -o Eigen3=system   # Force system Eigen3
conan install . -o Eigen3=disabled # Exclude Eigen3 entirely
```

---

## Compiler Profiles

Profiles live in `tula_cmake/profiles/` and compose modular `_base/` fragments.

### Linux

| Profile | Compiler | Use case |
|---------|----------|---------|
| `linux-clang20-debug` | Clang 20 (apt.llvm.org) | Primary development |
| `linux-clang20-release` | Clang 20 | Production builds |
| `linux-clang18-debug/release` | Clang 18 | Compatibility |
| `linux-gcc14-debug/release` | GCC 14 | GCC builds |
| `linux-gcc-debug/release` | System GCC | Fallback |

Clang 20 is installed from `apt.llvm.org/noble/` via `.devcontainer/postCreate.sh`.
GCC 15 is not yet available for aarch64/noble; GCC 14 is the current GCC maximum.

### macOS

| Profile | Compiler |
|---------|----------|
| `brew-llvm-debug/release` | Homebrew LLVM |
| `brew-gcc-debug/release` | Homebrew GCC |

### Preset alias

`default-debug` / `default-release` auto-selects platform compiler.

---

## Build Options

| Variable | Default | Description |
|----------|---------|-------------|
| `TULA_BUILD_TESTS` | `ON` | Build tula test suite |
| `TULA_BUILD_EXAMPLES` | `OFF` | Build examples |
| `TULA_BUILD_DOC` | `OFF` | Build API docs |
| `TULA_CACHE_ROOT` | `~/.tula_cache` | Cache root for CPM |
| `CPM_SOURCE_CACHE` | `${TULA_CACHE_ROOT}/cpm` | CPM git clone cache |

---

## Downstream Integration

Projects that consume tula (e.g., kidscpp, tula_example) use the same Conan install flow:

```bash
# Step 1: install tula deps, generate toolchain for kidscpp
cd kidscpp
conan install . \
  --profile=../tula/tula_cmake/profiles/linux-clang20-debug \
  --build=missing \
  -o Eigen3=auto -o logging=auto -o Ceres=auto -o NetCDF=auto -o NetCDFCXX4=auto

# Step 2: configure (toolchain auto-provides tula::tula)
cmake --preset conan-default

# Step 3: build
cmake --build build/clang20-debug
```

The downstream `conanfile.py` inherits from `TulaConan`:

```python
from tula_cmake.tula_conan import TulaConan

class KidsCppConan(TulaConan):
    pass  # All dep management from TulaConan
```

---

## License

Copyright (c) Zhiyuan Ma. BSD 3-Clause license — see `licenses/` for details.
