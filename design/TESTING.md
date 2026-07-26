# Testing strategy

## Objectives

The test loop must provide fast Python feedback, exercise semantic CMake
features, and verify both distribution boundaries:

- installed `tula-cmake` wheel;
- packaged `tula_boilerplate` consumed independently.

## Unit and documentation gate

`just unit` runs:

- Ruff lint and formatting checks;
- `ty` static analysis;
- 27 fast Pytest tests, with documentation sources included in collection;
- branch coverage with an 85% threshold;
- Sphinx HTML documentation build with warnings treated as errors.

Tests cover Pydantic invariants, registry loading, missing resources, selection
rendering, convention-derived provider entry points, selection-aware preset
cache variables, CLI option validation/scoping, installed resources, typed
generated-preset parsing, and exact external-command ordering.

The unit gate also validates that every registry feature has matrix metadata,
every provider is collected automatically, and every feature-owned option has
an axis covering its complete value domain. Executable matrix cases are
marked and deselected from this fast gate.

The generated Sphinx site has a dedicated model page using
`autodoc-pydantic`; it renders field metadata, validator summaries, and JSON
schemas from the runtime classes rather than maintaining parallel tables.

`recipe.py` is excluded from unit coverage because it is executed by every real
Conan integration gate.

## Feature matrix

`just matrix` runs the download-free feature cases as ordinary Pytest
items. `just matrix-all` also runs Conan and CPM providers. The current
catalog derives 62 cases:

- disabled and every enabled provider for each feature;
- every logging-level value;
- disabled, Conan, CPM, and system yaml-cpp providers;
- disabled and CPM csv-parser providers;
- disabled, Conan, and system NetCDF C providers;
- disabled, CPM, and system NetCDF C++ providers;
- disabled and CPM bitmask providers;
- disabled and CPM meta-enum providers;
- disabled, Conan, and CPM clipp providers;
- disabled, Conan, CPM, and system Eigen providers plus both multithreading
  values;
- disabled and Conan Spectra, Boost, FFTW, CCfits, and Ceres providers;
- every perflibs option value;
- explicit capability gates for oneAPI and LLVM OpenMP cases.

Each runnable case uses an isolated temporary project, direct root-scoped Conan
options, the production `BuildWorkflow`, generated-preset assertions, a
feature-specific C++ probe, and CTest. No example profile defines matrix state.

The dev container installs GCC 13, GCC 14, Clang 20, and LLVM 20 OpenMP.
`TULA_TEST_PROFILE` selects the ordinary matrix compiler. GNU and LLVM runtime
cases additionally use `TULA_TEST_GNU_PROFILE` or `TULA_TEST_LLVM_PROFILE`
with the matching capability. oneAPI, Intel OpenMP, and MKL threading remain
capability-gated.

`just gcc14` and `just clang20` are compiler acceptance gates, not reduced
smoke tests. Each runs the applicable 62-case feature matrix and then creates
Tula, kidscpp, and Citlali in dependency order with the selected compiler.
The versioned profiles name their compiler executables explicitly, so the
Conan package identity, generated preset, CMake compiler detection, and
compiled consumer all agree. Clang uses Ubuntu's `libstdc++` ABI and requires
both `libomp-20-dev` and `clang-tools-20`; the latter supplies
`clang-scan-deps` for CMake's C++20 module-dependency scan.

`just providers` selects only the network-marked Conan/CPM cases.

CPM source archives may be reused from `.devcontainer/cache/cpm/`; build/output
trees remain isolated.

### Compiler acceptance results

Measured in the Ubuntu 24.04 ARM64 dev container on 26 July 2026:

| Gate | Compiler | Applicable matrix | Runtime case | Installed chain |
|---|---|---:|---|---|
| `just gcc14` | GNU 14.2.0 | 56 passed, 6 capability-skipped | GNU OpenMP passed | Tula → kidscpp → Citlali and all `test_package` consumers passed |
| `just clang20` | Clang 20.1.2 | 56 passed, 6 capability-skipped | LLVM OpenMP passed | Tula → kidscpp → Citlali and all `test_package` consumers passed |

The six skips are deliberate alternate-image cases: four oneMKL/threading
cases, the Intel OpenMP profile, and the other compiler family's OpenMP
runtime. The catalog still collects all 62 cases.

## Package smoke acceptance

`just boilerplate` builds the minimal example with its Conan-backed logging
default. `just tula` uses the feature defaults owned by the Tula recipe:
Conan-backed logging, yaml-cpp, clipp, and Eigen; CPM-backed csv-parser,
bitmask, meta-enum, and GrPPI; and system perflibs and NetCDF C/C++. It runs
thirteen CTest cases: core header smoke,
ECSV core/header, Eigen utilities, Eigen-backed nddata, ECSV typed tables,
streaming CSV-to-ECSV loading, FlatConfig, YamlConfig, and filename/filesystem
helpers, NetCDF type/I/O behavior, and enum/bitmask metadata and formatting.
The CLI case covers builder parsing and typed configuration projection; the
GrPPI case exercises the normalized dynamic sequential execution policy.
These tests validate package behavior without
duplicating the feature matrix.

This is meaningful compile/runtime coverage, not yet full production behavior
parity.

## Wheel boundary

`just wheel`:

1. copies `tula_cmake` and boilerplate outside the uv workspace;
2. builds the wheel;
3. verifies registry, CMake, template, profile, and `py.typed` contents;
4. installs into a fresh virtual environment;
5. exports the copied Python-require into an isolated Conan home;
6. builds and runs copied boilerplate sources.

## Package-chain boundary

`just downstream`:

1. creates an isolated Conan home;
2. exports the local `tula-cmake` Python-require;
3. creates `tula-boilerplate/3.1.0`;
4. invokes `tula_downstream/build` once;
5. verifies the executable consumed packaged boilerplate.

`just kidscpp` creates `tula/3.1.0`, then creates `kidscpp/3.1.0` against the
installed `tula::headers` target in the same isolated Conan home. Its CTest
gate verifies FFT shape, Welch PSD output, and deterministic timestream
solving. Conan then compiles the independent `test_package` target.

`just citlali` creates the complete installed-package chain in one Conan home:
Tula, kidscpp, then `citlali/4.0.0`. It builds the five-source v4 library
against Spectra, Boost, FFTW, CCfits, and Ceres, then runs two Gaussian model
regressions. With no environment override it uses and removes a fresh Conan
home. Developers may set `CITLALI_CONAN_HOME` to a cache under
`.devcontainer/cache/` for fast compile-fix iterations.

Tula, kidscpp, and Citlali each have a minimal `test_package` that consumes
their installed CMake target. Package creation therefore verifies installed
headers, target metadata, and transitive public requirements in addition to
the source-tree behavior suites.

## Commands

```sh
just unit
just matrix
just matrix-all
just gcc14
just clang20
just compilers
just fast
just providers
just boilerplate
just tula
just wheel
just downstream
just kidscpp
just citlali
just all
```

Logs are retained under `.devcontainer/logs/`.

## Growth policy

Do not recreate a Cartesian matrix speculatively. For each package or semantic
feature:

1. record versions, targets, modes, option domains, and platform requirements;
2. add one feature probe and one option axis per feature-owned option;
3. declare only meaningful companion options and capability requirements;
4. let the catalog-completeness test prove every provider and value is covered;
5. add mixed cases only for meaningful dependency edges;
6. extend package-chain tests when public package metadata changes.
