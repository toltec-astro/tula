# Tula v3.1 superbuild architecture

## Package roles

| Package | Role |
| --- | --- |
| `tula_cmake` | Typed superbuild infrastructure: Conan recipe mixin, validated registry, CMake feature resolvers, profiles, templates, documentation, and bootstrap CLI. |
| `tula_boilerplate` | Minimal package built with `tula_cmake`; both an example and a real Conan package. |
| `tula_downstream` | Independent consumer requiring only `tula-boilerplate/3.1.0`. |
| `tula` | Actual packaged C++ library built with `tula_cmake`; its compiler gates verify the restored header modules through thirteen behavior tests. |
| `kidscpp` | Actual downstream of packaged Tula; the active v3 surface is the intentionally trimmed timestream solver and PSD library. |
| `citlali` | Final v4 application library; consumes packaged kidscpp and adds the astronomy/numerics dependency slice needed by its current five compiled modules. |

Build infrastructure is not a C++ link dependency. Conan evaluates the
boilerplate or Tula recipe's `python_requires`; ordinary downstream code sees
only the package and CMake targets it consumes.

## User workflow

The downstream repository exposes:

```sh
./build
```

For a release, that script executes:

```sh
uvx \
  --from git+https://github.com/toltec-astro/tula_cmake.git@v3.1.0 \
  tula-cmake build .
```

`BuildWorkflow` then:

1. optionally runs `conan config install`;
2. ensures a default profile when no profile was supplied;
3. runs `conan install --build=missing`;
4. validates `CMakeUserPresets.json` and Conan's generated preset document;
5. configures with `cmake --preset <name> --fresh`;
6. builds with `cmake --build --preset <name>`.

This preserves Conan's explicit install-plus-preset flow. CMake never invokes
Conan.

The package repositories expose the same launcher. `TULA_CMAKE_DEV_PROJECT`
selects a local infrastructure checkout; `TULA_CMAKE_SOURCE` overrides the
tagged CLI source; and `TULA_CONAN_CONFIG_SOURCE` supplies shared remotes and
profiles through `conan config install`.

## Source and package distribution

GitHub tags are the source authority. A TolTEC Conan virtual remote will
provide the `tula-cmake/3.1.0`, `tula/3.1.0`, `kidscpp/3.1.0`, and
`citlali/4.0.0` recipes and compatible binaries. Its intended backing
repositories are a development local repository, release local repository,
and cached ConanCenter remote.

Tula and kidscpp remain ordinary Conan graph nodes. They are not CPM features:
moving first-party packages into FetchContent would hide package versions,
options, package IDs, public edges, and lockfile state from Conan.

Until the remote exists, the workspace gate simulates the release pipeline in
one isolated Conan home by exporting tula_cmake and creating Tula, kidscpp,
then Citlali.

## Python package structure

```text
tula_cmake/
├── .cruft.json
├── conanfile.py
├── pyproject.toml
├── justfile
├── docs/
├── examples/
│   ├── tula_boilerplate/
│   └── tula_downstream/
├── tests/
└── src/tula_cmake/
    ├── models.py
    ├── registry.py
    ├── recipe.py
    ├── workflow.py
    ├── cli.py
    ├── resources.py
    ├── py.typed
    └── data/
        ├── registry.yaml
        ├── cmake/
        │   ├── infrastructure/
        │   │   ├── TulaProject.cmake
        │   │   ├── TulaConfigHeader.cmake
        │   │   └── TulaCPM.cmake
        │   └── resolvers/
        │       ├── eigen.cmake
        │       ├── logging.cmake
        │       ├── netcdf_c.cmake
        │       ├── netcdf_cxx4.cmake
        │       ├── perflibs.cmake
        │       ├── spectra.cmake
        │       ├── boost.cmake
        │       ├── fftw.cmake
        │       ├── ccfits.cmake
        │       ├── ceres.cmake
        │       ├── csv_parser.cmake
        │       └── yaml_cpp.cmake
        ├── templates/
        └── profiles/
```

Responsibilities:

- `models.py`: frozen Pydantic models for registry, options, selections, build
  requests, and CMake preset JSON;
- `registry.py`: YAML loading, resource validation, graph ordering, selection
  validation, and deterministic CMake rendering;
- `recipe.py`: thin Conan lifecycle adapter;
- `workflow.py`: process orchestration independent of CLI parsing;
- `cli.py`: Typer command surface and user-facing error translation;
- `resources.py`: `importlib.resources` access to installed data.

CMake and templates are package data because their paths must remain valid from
an installed wheel and a Conan export. Installing them as global data files
would make ownership and discovery environment-dependent.

The CMake tree separates framework code from package semantics:

- `cmake/infrastructure/` contains the public modules placed on
  `CMAKE_MODULE_PATH`;
- `cmake/resolvers/` contains one implementation per registry feature.

Resolver wiring is a checked convention, not YAML data. A feature named
`logging` maps to `resolvers/logging.cmake` and exports one entry point per
declared mode:

```text
tula_resolve_logging_conan()
tula_resolve_logging_cpm()
tula_resolve_logging_system()
```

The same convention maps `yaml_cpp` to `resolvers/yaml_cpp.cmake`. Its Conan,
CPM, and system entry points all create the single normalized
`tula::yaml_cpp` target.

`TulaProject.cmake` derives `tula_resolve_<feature>_<mode>()`, invokes it, and
verifies `tula::<feature>`. Provider selection therefore exists once in the
driver; resolver modules do not accept `MODE` or repeat a mode switch. Private
helpers inside a feature module may share acquisition or target-normalization
logic.

The Pydantic models include field descriptions used by both runtime JSON
schema and Sphinx. `autodoc-pydantic` renders each model explicitly, while
Sphinx warnings are errors. This follows the useful documentation boundary in
the read-only Tollan reference without importing Tollan's astronomy-specific
Sphinx stack.

## Repository ownership audit

Every retained top-level directory has an active owner:

- `tula_cmake/` is a top-level repository owning infrastructure, matrix tests,
  the boilerplate, and its downstream consumer;
- `tula/` owns the C++ utility headers and behavior tests;
- `kidscpp/` and `citlali/` own their independent package surfaces;
- `tula/design/` is the current cross-package architecture record.

The release topology removes Tula's `tula_cmake` Git submodule and uv
workspace. The infrastructure wheel bootstraps the user command; the matching
Conan Python-require supplies recipe behavior. Generated build, cache,
documentation, and virtual-environment directories remain ignored and
reproducible rather than source-owned.

## Template management

Cruft links `tula_cmake` to
`https://github.com/Jerry-Ma/cookiecutter-pypackage.git`, checkout `v2026`.
`.cruft.json` records the exact template commit and generation context.

Project-specific choices intentionally override parts of the template:

- Python `>=3.11` rather than the template's current `>=3.13`;
- an explicit package version while the release process is still being built;
- Conan-specific dependencies and package data;
- Sphinx documentation focused on architecture and generated API.

The read-only Tollan package supplied the nominal TolTEC documentation
conventions adopted here: generated Pydantic model pages, copyable code blocks,
mixed Markdown/reStructuredText input, version metadata from the imported
package, and documentation included in the test collection.

`just cruft-check` detects upstream template movement; `just cruft-update`
provides the reviewed update path.

## Conan/CMake boundary

Conan owns settings, profiles, options, graph resolution, package acquisition,
`CMakeDeps`, `CMakeToolchain`, presets, and `python_requires`.

`tula_cmake` owns the validated feature schema, semantic feature resolvers,
normalized `tula::<feature>` targets, generated configuration headers, and the
one-command orchestration layer.

The registry feeds:

1. Conan option domains and defaults;
2. Conan requirements for selected Conan-backed features;
3. enabled-feature CMake cache variables in the Conan-generated preset;
4. the generated CMake feature manifest;
5. configuration-header feature/provider macros.

### User-option boundary

Feature modes and customization points are Conan options because they affect
the dependency graph, compilation, or package identity. The supported input
surfaces are:

1. checked-in/composed profiles for reproducible builds;
2. `tula-cmake build --option NAME=VALUE` for higher-priority root-package
   experiments.

The CLI scopes overrides with Conan's `&:` consumer pattern and passes them as
`--options:host`. Conan validates their domains. `TulaConan.generate()` then
maps options owned by enabled features through
`CMakeToolchain.cache_variables`; consequently they appear in
`CMakePresets.json` and are supplied as normal `-D` values during configure.

Disabled features do not emit their CMake variables, avoiding unused-variable
warnings. Immutable resolver implementation data—currently CPM URLs and
hashes—remains in the feature manifest. The generated preset is not an input
file and should not be edited; direct CMake overrides could desynchronize CMake
from the Conan graph.

## Feature contracts

### Logging

`logging` is one meta-feature:

- Conan requires `fmt/11.2.0` and `spdlog/1.15.3`;
- CPM downloads both pinned, checksummed archives;
- system mode requires config packages for both;
- all enabled paths create `tula::logging`;
- the target links both normalized upstream targets;
- `logging_level` maps to `SPDLOG_ACTIVE_LEVEL`.

There is no independent fmt feature or option. Dependency splitting is deferred
until the package matrix exposes a real consumer requiring such a public
contract.

### Perflibs

`perflibs` ports the most complex production CMake behavior:

- `Threads::Threads` is always required when enabled;
- OpenMP policy is `auto`, `disabled`, or `required`;
- runtime intent is `auto`, `gnu`, `intel`, or `llvm`;
- oneAPI can enable config-mode `find_package(MKL CONFIG REQUIRED)`;
- modern oneMKL is consumed as `MKL::MKL`;
- MKL threading is `sequential`, `openmp`, or `tbb`;
- invalid combinations fail before target creation.

The normalized target exports:

```text
TULA_PERFLIBS_HAS_THREADS
TULA_PERFLIBS_HAS_OPENMP
TULA_PERFLIBS_HAS_MKL
TULA_PERFLIBS_OPENMP_RUNTIME
TULA_PERFLIBS_MKL_THREADING
```

The dev container carries GCC 13, GCC 14, Clang 20, GNU OpenMP, and LLVM
OpenMP. Compiler-specific gates verify Threads plus the matching required
OpenMP runtime, then recreate the installed Tula → kidscpp → Citlali graph
under that compiler identity. The Clang profile uses Ubuntu's `libstdc++11`
ABI and the container provides `clang-tools-20` for CMake's dependency
scanner. oneMKL branches still require a dedicated oneAPI image.

### yaml-cpp and the first Tula behavior slice

`yaml_cpp` pins `yaml-cpp/0.9.0` for Conan and the matching checksummed source
archive for CPM. The system provider uses the Ubuntu 24.04 yaml-cpp 0.8 package;
all three providers compile and execute the same YAML parse/emit probe.

Tula links the normalized target through `tula::headers`. Its ECSV test ports
production assertions for ECSV version/header parsing, schema and column
metadata, header formatting, and typed metadata extraction. The two fmt
formatters in `ecsv/hdr.h` are now `const`, as required by fmt 11, without
changing their output.

### Eigen, nddata, and ECSV tables

`eigen` uses the maintained Eigen 3.4.1 release for Conan and CPM and accepts
the Ubuntu Eigen 3.4.0 config package in system mode. Eigen 5.0.1 is current,
but it is a breaking major release; preserving production behavior takes
priority during this port. The feature depends on `perflibs`, creates
`tula::eigen`, and maps `eigen_multithreading=disabled` to
`EIGEN_DONT_PARALLELIZE`.

Six derived matrix cases cover disabled, Conan, CPM, system, and both
multithreading values. Tula now tests Eigen traits and storage conversion,
owned/referenced Eigen nddata, and ECSV typed table loading. The port exposed
and corrected `ECSVTable::col()`: it now returns the existing `ColDataRef`
abstraction for both Eigen-backed numeric columns and vector-backed strings.
Mutable and const `array_data()` overloads preserve the intended access model.
The direct include of Eigen's internal `XprHelper.h` was removed; `Eigen/Core`
already provides the required declarations.

### CSV parser and streaming ECSV rows

`csv_parser` uses the same Jerry-Ma fork as the production system, pinned at
commit `bc3bebcc16fb74144e9d94035346b3d9150b39c5` and protected by a SHA-256
archive checksum. The fork has no release tags, Conan recipe, installable CMake
package, or upstream CMake project. The v3 contract therefore declares only
the provider with executable evidence: CPM downloads the header-only source,
and the resolver publishes its `include/` directory as `tula::csv_parser`.

Two derived matrix cases cover disabled and CPM acquisition. The feature probe
parses a custom-delimited row with a quoted delimiter. Tula's sixth CTest case
passes the same parser range directly to `ECSVTable::load_rows()`, verifying
that header parsing leaves the stream at the first data row and that quoted
strings, integers, and floating-point values reach their typed columns. This
ports the production streaming path without adding a second CSV abstraction.

### Configuration and filesystem helpers

Not every Tula module should become a `tula_cmake` feature. The registry owns
external acquisition and configuration boundaries:

- `FlatConfig` and filename/filesystem helpers require `logging` and the C++23
  standard library;
- `YamlConfig` requires the already-modeled `logging` and `yaml_cpp` targets.

Creating `config` or `filesystem` resolver entries would duplicate those
requirements without acquiring a package. Instead, three Tula behavior tests
port the production contracts: typed/optional `FlatConfig` access and
callbacks; nested `YamlConfig` lookup, merge, validation, and formatting; and
filename pattern expansion, directory creation, and regex discovery. All five
active/reference headers are otherwise behavior-identical. The only existing
source delta is the qualified `fmt::format_to` call required by modern fmt.

### NetCDF C and C++ layers

The production `<netcdf>` API comes from two upstream packages, so v3 models
two features instead of hiding a mixed acquisition graph:

- `netcdf_c` supports Conan `netcdf/4.8.1` and the system
  `netCDF::netcdf` config target, normalized as `tula::netcdf_c`;
- `netcdf_cxx4` depends on `netcdf_c`, supports CPM and system acquisition,
  and normalizes both as `tula::netcdf_cxx4`.

Ubuntu 24.04 supplies NetCDF C 4.9.2 and C++ 4.3.1. The C++ API's latest
release remains v4.3.1. Its installed Debian package exports pkg-config rather
than a CMake config, so system resolution uses CMake's imported
`PkgConfig::NETCDF_CXX4` target.

The v4.3.1 top-level CMake cannot be embedded: it uses `CMAKE_SOURCE_DIR` for
its own inputs. The CPM resolver therefore uses download-only acquisition and
defines one static target from the upstream `cxx4/nc*.cpp` sources, public
headers, and `tula::netcdf_c`. No upstream sources are patched.

Six matrix cases cover disabled and every provider across the two packages.
Tula's tenth behavior test ports the reference type assertion and adds typed
dispatch, scalar file I/O, and pretty-printing checks.

### Bitmask and compile-time enum metadata

The production enum surface is a composition of two independent header-only
packages rather than one synthetic `enum` dependency:

- `bitmask` pins `oliora/bitmask` commit `0454f327` and exports
  `tula::bitmask`;
- `meta_enum` pins `Jerry-Ma/meta_enum` commit `f940f15b` and exports
  `tula::meta_enum`.

Neither project has a Conan Center recipe needed by the current system
contract, so both expose only the executable CPM path plus disabled state.
Each archive is checksum-protected. The meta-enum probe intentionally tests
the parser primitives consumed by `TULA_ENUM`; the upstream convenience macro
has an `emta_enum` typo and is neither patched nor used.

Tula's eleventh behavior test ports enum metadata, nested enum registration,
bitmask composition, and formatting expectations. It exposed and corrected a
dormant concept error: `BitFlag` had treated `mask_value` as a nested type
instead of a constexpr expression. The fix restores the intended concept
without changing the enum or formatting contract.

### Clipp and the Tula CLI builder

`clipp` uses its current release 1.2.3 from Conan Center or the matching
checksummed upstream archive through CPM. Both providers create
`tula::clipp`; no distribution-system provider is claimed without executable
package evidence.

Three matrix cases cover disabled, Conan, and CPM state with the same parse
probe. Tula's twelfth behavior test exercises the production builder layer:
boolean flags, typed integer input, enum-backed choices, default projection,
positional values, and `FlatConfig` storage. The test exposed help text
formatting raw enum defaults even though storage already normalized them.
Help generation now calls the same `normalize_value()` path, preserving the
intended enum name and satisfying fmt 11's typed formatting rules.

### GrPPI and the timestream execution policy

`grppi` uses the TolTEC `Jerry-Ma/grppi` C++20 fork pinned at commit
`12f5c11b` and protected by a SHA-256 archive checksum. The fork is a
header-only source dependency without a suitable Conan Center contract, so
v3 exposes the two states for which there is executable evidence: disabled
and CPM.

The normalized `tula::grppi` target requires logging, bitmask, meta-enum, and
perflibs. It publishes the upstream include directory, inherits Threads and
OpenMP from `tula::perflibs`, and defines `GRPPI_OMP` only when the OpenMP
target exists. The feature matrix executes a sequential `grppi::map` probe;
Tula separately exercises `tula::grppi_utils::dyn_ex("seq")`. This dependency
was added because the trimmed kidscpp timestream solver uses the execution
policy directly.

## Registry-driven feature matrix tests

Feature-matrix validation belongs to `tula_cmake`, not to
`tula_boilerplate`, Tula, or downstream packages. The test system has four
small parts:

```text
data/registry.yaml
        │ product domains
        ▼
tests/feature_matrix/matrix.yaml ── probe + option-axis/capability metadata
        │
        ▼
pytest parameter collection ── one node ID per provider or option value
        │
        ▼
one temporary Conan/CMake matrix project ── configure → build → CTest
```

Provider cases are derived automatically from every feature's declared modes,
including disabled. Each feature-owned option must have one test axis; every
allowed value becomes a case. `test_feature_matrix_catalog.py` fails during the fast
unit gate if a feature or option axis is missing or inconsistent with the
production registry.

The matrix metadata contains only information that cannot be inferred:

- the C++ probe associated with a feature;
- the provider used to exercise an option axis;
- companion options needed to make a value meaningful;
- environment capabilities such as `oneapi` or `llvm-openmp`.

Each executable matrix case copies one static internal project into `tmp_path`,
passes root-scoped Conan options directly, invokes the production
`BuildWorkflow`, checks generated-preset cache variables, verifies normalized
target presence/absence, compiles the probe, and runs CTest. Pytest supplies
selection, node IDs, capture, timing, skip reasons, and optional JUnit output;
there is no custom runner, template engine, timestamped results tree, or second
hand-written provider matrix.

Network-backed Conan/CPM cases carry a Pytest marker. System facilities use
explicit capability gates, so unavailable oneAPI/Intel/LLVM cases are visible
as collected skips and become runnable in a suitable image through
`TULA_TEST_CAPABILITIES`. Intel and LLVM runtime cases additionally require
`TULA_TEST_INTEL_PROFILE` or `TULA_TEST_LLVM_PROFILE`; the fixture rejects a
runtime selection under the wrong compiler family. oneAPI cases assert that
`TULA_PERFLIBS_HAS_MKL=1`, and required/disabled OpenMP cases assert the
published target capability.

`tula_boilerplate` now has no matrix profiles. Its acceptance test builds the
default minimal example once. Tula likewise has a separate package smoke test;
neither package is multiplied across infrastructure choices.

## Generated C++ contract

For every registry feature, `tula_add_config_header()` emits:

```c
#define <PREFIX>_HAS_<FEATURE> 0_or_1
#define <PREFIX>_<FEATURE>_PROVIDER "disabled|conan|cpm|system"
```

The template contains no feature-specific list. Adding a registry feature
automatically extends the generated contract.

## Verified package chain

```text
tula-cmake wheel + Conan python-require
                │
                ▼
tula-boilerplate/3.1.0
                │
                ▼
tula_downstream executable
```

The acceptance test uses an isolated Conan home, creates the boilerplate
package, runs the checked-in downstream command, and verifies the resulting
executable.

Tula, kidscpp, and Citlali also contain Conan `test_package` consumers. These
compile against installed package targets after `conan create`, closing the
gap between source-tree behavior tests and uploadable artifacts.

The production chain now has a second independently verified boundary:

```text
tula-cmake/3.1.0 python-require
                │
                ▼
tula/3.1.0 headers + generated feature config
                │  CMakeDeps target: tula::headers
                ▼
kidscpp/3.1.0 static library
                │  public Conan edge: transitive headers + libraries
                ▼
citlali/4.0.0 static library
```

Tula's recipe configures, builds, and installs the source tree, packages the
public headers plus generated `tula/config.h`, and publishes `tula::headers`.
Header-only CPM providers register their checked include trees with the
infrastructure; Tula installs that bundled closure so package consumers receive
bitmask, meta-enum, csv-parser, and GrPPI headers without rerunning CPM.
`kidscpp` declares `tula/3.1.0` as a normal Conan requirement. It also extends
the Tula recipe mixin because its own build needs normalized targets for CPM
and system dependencies while compiling.

Citlali is developed on branch `v4.x_conan2`, created directly from `v4.x`;
`refs/citlali` remains an unmodified production reference. Its Conan recipe
requires `kidscpp/3.1.0` and explicitly propagates public headers and libraries.
The same visibility is declared on kidscpp's Tula edge so a third-level
consumer receives `tula::headers` without a sibling checkout.

Projects can set `tula_default_options` on the mixin to define their normal
feature set without duplicating registry option declarations. The mixin
validates those keys and still permits command-line and profile overrides.
Projects separately list `tula_public_features`; Conan requirements for those
normalized targets propagate headers and libraries to installed consumers.

## Trimmed kidscpp v3 surface

The active `kidscpp` tree follows the intent recorded on its archived v3
branch: C++ owns timestream solving; sweep finding and fitting remain outside
this package. The much larger preserved archive and `refs/kidscpp` are audit
inputs, not source trees to restore wholesale.

The package contains raw and solved timestream data models, calibration-model
metadata, Welch/PSD utilities, the raw-I/Q solver, and NetCDF result
serialization. Sweep finders, sweep fitters, the GUI, the legacy multipurpose
CLI, and the runtime `std::variant` over sweep/timestream types are absent.
The GCC 9–13 specialization of private `std::variant` internals was not
carried forward. The solver keeps the archived numerical equations and output
metadata but no longer pulls Ceres through a sweep fitter merely to obtain an
eleven-parameter count.

Three CTests provide the initial behavior boundary: odd-length FFT shape,
finite one-sided Welch output, and deterministic raw I/Q conversion to
detuning/dissipation. The latter verifies calibration-model and output-kind
metadata. Fit-report fallback now checks whether a fit-report source was
requested before reading TolTEC filename metadata, so built-in calibration
works with a minimal in-memory timestream.

## Citlali dependency slice

The first Citlali slice introduces five Conan-backed features independently:

| Feature | Conan reference | Normalized target | Focused probe |
| --- | --- | --- | --- |
| `spectra` | `spectra/1.0.1` | `tula::spectra` | symmetric eigensolver |
| `boost` | `boost/1.91.0` | `tula::boost` | Boost header/version use |
| `fftw` | `fftw/3.3.10` | `tula::fftw` | double-precision plan/execute |
| `ccfits` | `ccfits/2.6` | `tula::ccfits` | FITS object construction |
| `ceres` | `ceres-solver/2.2.0` | `tula::ceres` | bounded scalar solve |

Each feature has disabled and Conan matrix cases. Spectra and Ceres share
Eigen 3.4.0, matching their Conan Center graphs. Ceres disables generated
Schur specializations for this GCC 13 validation path: Citlali uses dense
QR, while the smaller binary avoids unnecessary compile-time memory pressure.

The current v4 library intentionally preserves the five source files compiled
by the `v4.x` CMake definition: calibration, telescope, mapmaking, PTC
sensitivity, and Gaussian models. The old CLI is not part of this slice
because it owns removed kidscpp sweep APIs and generated version headers.
Two tests lock Gaussian 1-D values and the existing flattened 2-D mesh result.

Three compatibility edits preserve behavior while accepting current APIs:

- ECSV `ColDataRef` values are read through their public `.data` view;
- Ceres 2.2 `SubsetManifold`/`SetManifold` replaces the removed local
  parameterization API;
- the existing Eigen scan-index log includes Tula's matrix formatter.

## Next boundary

1. Add a oneAPI-capable validation image for MKL/runtime combinations.
2. Grow the package matrix one dependency row at a time.
3. Add a minimal consumer of packaged `citlali::citlali`.
4. Decide the v4 CLI boundary independently of the verified library slice.
5. Add focused NetCDF result serialization fixtures to kidscpp.
6. Audit remaining timestream-only entry points without restoring sweep, GUI,
   or broad CLI ownership.
