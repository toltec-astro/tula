# Tula v3.1 feature/provider architecture

## Goal

Preserve the user-facing ability to choose both a subset of logical features
and an acquisition provider for every enabled feature:

```text
disabled | conan | cpm | system
```

The implementation delegates to Conan wherever Conan has a complete mechanism,
while keeping a deliberately small Tula layer for CPM, system discovery,
validation, and normalized targets.

## Boundary

Conan owns:

- settings, profiles, options, requirements, and the dependency graph;
- Conan package acquisition and source/binary selection;
- CMakeDeps, CMakeToolchain, presets, and lockfiles.

`tula_cmake` owns:

- the supported feature/provider matrix;
- dependency and provider validation;
- the CPM and system provider implementations;
- conversion of every provider result into `tula::<feature>`;
- Tula project helpers such as generated configuration headers.

CMake never chooses a provider. It receives the selected mode in a generated
manifest and executes that provider after `project()` has enabled languages.

## Lifecycle

1. A project profile sets `&:logging=disabled|conan|cpm|system`.
2. `TulaConan` validates the selected feature graph.
3. `requirements()` adds requirements only for Conan-backed features.
4. `generate()` runs CMakeDeps and CMakeToolchain and writes
   `tula_features.cmake`.
5. The downstream project calls `tula_resolve_features()` after `project()`.
6. The logging provider creates `tula::logging`, or no target when disabled.
7. `tula_add_config_header()` records the resolved feature state for C++.

The explicit post-project call avoids hidden `CMAKE_PROJECT_INCLUDE` mutation
and makes the integration visible in every downstream `CMakeLists.txt`.

Downstream projects compose their feature-only profile with a compiler profile
bundled in the installed package:

```sh
base="$(uv run tula-cmake profile linux-gcc13-debug)"
uv run conan install . -pr:a="${base}" -pr:a=profiles/logging-conan
```

This keeps compiler policy DRY without source-tree-relative profile includes.

## Registry

`tula_cmake/features.yaml` is the single source of truth for:

- supported providers;
- Conan requirements;
- logical feature dependencies;
- provider module;
- immutable CPM source URLs and SHA-256 checksums.

The Python model is pure and independently unit-tested. The Conan recipe layer
only translates validated selections into requirements and generated files.

## Initial vertical slice

Only `logging` is implemented. It represents spdlog plus external fmt.

- Conan uses `spdlog/1.15.3` and its Conan graph.
- CPM uses checksummed fmt 11.2.0 and spdlog 1.15.3 archives.
- System uses config-mode `find_package(fmt)` and `find_package(spdlog)`.
- Disabled performs no acquisition and creates no logging target.

All enabled modes produce `tula::logging` with the same compile-time log-level
contract.

The built wheel is an acceptance boundary: its registry, CMake modules,
configuration template, and base profile are audited, then a copy of
`tula_boilerplate` is built against that wheel outside the uv workspace.

## Extension rule

A new feature is added only after the logging vertical slice remains green.
Simple packages must use generic provider behavior. A custom CMake module is
permitted only for a real semantic difference such as a metapackage, unusual
upstream targets, or feature-specific compile definitions.

Logical prerequisites are declared in the registry and validated before CMake.
Resolution order will be topological as soon as the second dependent feature is
introduced.
