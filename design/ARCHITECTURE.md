# Tula v3.1 superbuild architecture

## Package roles

| Package | Role |
| --- | --- |
| `tula_cmake` | Typed superbuild infrastructure: Conan recipe mixin, validated registry, CMake feature resolvers, profiles, templates, documentation, and bootstrap CLI. |
| `tula_boilerplate` | Minimal package built with `tula_cmake`; both an example and a real Conan package. |
| `tula_downstream` | Independent consumer requiring only `tula-boilerplate/3.1.0`. |
| `tula` | Actual C++ package built with `tula_cmake`; the current slice compiles logging and perflibs contracts. |
| `kidscpp` | Actual downstream of packaged Tula, deferred until Tula packaging and behavior parity are complete. |

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
uvx --from tula-cmake==3.1.0 tula-cmake build .
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

## Python package structure

```text
tula_cmake/
├── .cruft.json
├── conanfile.py
├── pyproject.toml
├── justfile
├── docs/
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

## Template management

Cruft links `tula_cmake` to
`https://github.com/Jerry-Ma/cookiecutter-pypackage.git`, checkout `v2026`.
`.cruft.json` records the exact template commit and generation context.

Project-specific choices intentionally override parts of the template:

- Python `>=3.11` rather than the template's current `>=3.13`;
- an explicit package version while the release process is still being built;
- Conan-specific dependencies and package data;
- Sphinx documentation focused on architecture and generated API.

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
3. the generated CMake feature manifest;
4. configuration-header feature/provider macros.

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

The GCC 13 dev-container gate verifies Threads plus required GNU OpenMP. oneMKL
branches require a dedicated oneAPI image before they can be marked verified.

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

## Next boundary

1. Add a oneAPI-capable validation image for MKL/runtime combinations.
2. Grow the package matrix one dependency row at a time.
3. Split meta-features only when concrete package UX requires it.
4. Restore one coherent production Tula module and its unchanged tests.
5. Complete Tula install/export and verify an independent `tula/3.1.0`
   consumer.
6. Move `kidscpp` only after that boundary is green.
