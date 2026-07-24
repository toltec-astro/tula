# Tula v3.1 superbuild architecture

## Package roles

The system has four distinct roles:

| Package | Role |
| --- | --- |
| `tula_cmake` | Superbuild infrastructure: Conan recipe policy, feature registry, provider implementations, generated CMake, profiles, and the bootstrap CLI. |
| `tula_boilerplate` | Minimal package built with `tula_cmake`. It is both provider acceptance code and a real Conan package. |
| `tula` | Actual C++ package built with `tula_cmake`. The current slice compiles a supported header smoke target but does not package the full production interface yet. |
| `kidscpp` | Actual downstream of packaged Tula. It remains outside the active slice until Tula packaging and behavior parity are complete. |

`tula_downstream` closes the current infrastructure loop. It is a minimal
consumer that requires only `tula-boilerplate/3.1.0`, locates the
Conan-generated CMake package, links `tula_boilerplate::headers`, and runs.

Build infrastructure is not a C++ link dependency. A downstream package
benefits transitively from the conventions established by `tula_cmake`, but it
does not link or import `tula_cmake`. Conan resolves the boilerplate recipe's
`python_requires` when the package graph needs that recipe.

## User experience

The canonical Conan 2 flow remains explicit:

1. Bootstrap the `tula-cmake` CLI and Conan.
2. Run `conan install`.
3. Configure and build with Conan's generated CMake preset.

The downstream repository exposes that sequence as one command:

```sh
./build
```

For a released system the script runs:

```sh
uvx --from tula-cmake==3.1.0 tula-cmake build .
```

`uvx` creates an isolated Python environment containing `tula-cmake` and its
pinned Conan dependency. `tula-cmake build` then:

1. optionally runs `conan config install <source>` for organization remotes and
   shared profiles;
2. ensures a default profile exists when no explicit profile is supplied;
3. runs `conan install --build=missing`;
4. reads the generated `CMakeUserPresets.json`;
5. runs `cmake --preset <generated> --fresh`;
6. runs `cmake --build --preset <generated>`.

This wrapper does not make CMake invoke Conan. It preserves the flow recommended
by the Conan documentation while providing the one-command entry point users
had in the production CMake system.

The checked-in downstream script supports `TULA_CMAKE_DEV_PROJECT` so the same
command uses the local uv workspace during development. The release path uses
the published wheel.

## Why `python_requires`

`tula_cmake/3.1.0` is exported as a Conan `python-require`. Recipes opt in with:

```python
python_requires = "tula-cmake/3.1.0"
python_requires_extend = "tula-cmake.TulaConan"
```

This is Conan's supported distribution mechanism for shared recipe behavior.
The exported recipe also carries the registry, CMake modules, configuration
template, and profiles. `tula_boilerplate` and `tula` therefore contain only
their package-specific recipe metadata and source/package methods.

The Python wheel and Conan python-require serve different bootstrap boundaries:

- the wheel supplies the user-facing CLI and an isolated Conan installation;
- the Conan python-require supplies reusable recipe behavior and build files to
  package graphs.

Both artifacts have the same version and are tested independently.

## Conan/CMake boundary

Conan 2.31 owns:

- settings, profiles, options, requirements, and graph resolution;
- package acquisition and binary selection;
- `CMakeDeps`, `CMakeToolchain`, and generated presets;
- resolution of `python_requires`.

`tula_cmake` owns:

- the supported feature/provider registry;
- dependency and provider validation;
- dependency-first feature resolution order;
- CPM and system provider implementations;
- normalization to `tula::<feature>` targets;
- generated project configuration headers;
- the thin bootstrap/build CLI.

CMake never chooses a provider. It executes the manifest selected during
`conan install`.

`CMakeDeps` remains the active stable generator. `CMakeConfigDeps` in Conan
2.31 has important improvements but is still documented as experimental.
`cmake-conan` is also not used because explicit `conan install` remains the
recommended general flow.

## Feature model

The current registry contains:

```text
formatting
└── no logical prerequisites

logging
└── requires formatting
```

Both features support:

```text
disabled | conan | cpm | system
```

`disabled` is implicit in the Python model. The YAML registry declares enabled
providers, requirements, logical dependencies, resolver module/command, and
immutable CPM inputs.

`formatting` is the generic provider proof:

- Conan requires `fmt/11.2.0`;
- CPM fetches the checksummed fmt 11.2.0 archive;
- system uses config-mode `find_package(fmt)`;
- all enabled modes normalize to `tula::formatting`.

`logging` is the custom provider:

- it requires the already-resolved `tula::formatting` feature;
- Conan requires `spdlog/1.15.3`;
- CPM fetches the checksummed spdlog 1.15.3 archive with external fmt;
- system uses config-mode `find_package(spdlog)`;
- all enabled modes normalize to `tula::logging`.

The generated manifest lists features in a stable dependency-first order.
Selection validation rejects unknown modes, cycles, unknown dependencies, and
enabled features whose prerequisites are disabled.

## Generated C++ contract

`tula_add_config_header()` iterates the resolved feature list. It emits, for
every feature:

```c
#define <PREFIX>_HAS_<FEATURE> 0_or_1
#define <PREFIX>_<FEATURE>_PROVIDER "disabled|conan|cpm|system"
```

The template no longer contains logging-specific feature definitions. Adding a
registry feature automatically extends the generated availability/provider
contract.

## Current vertical slice

The verified package chain is:

```text
tula-cmake wheel + Conan python-require
                │
                ▼
tula-boilerplate/3.1.0 Conan package
                │
                ▼
tula_downstream executable
```

The package-chain test uses an isolated Conan home:

1. export the local `tula-cmake` python-require;
2. create `tula-boilerplate/3.1.0` with both features disabled;
3. invoke `tula_downstream/build` once;
4. let the CLI run Conan and the generated CMake preset;
5. execute the downstream binary and verify package metadata.

Tula itself now compiles and runs a smoke target that covers the dependency-free
core headers plus formatting/logging headers when enabled. This is meaningful
compile coverage, but it is not yet the complete production header/test surface.

## Next package boundary

The next software milestone is to make Tula a conventional packaged dependency:

1. restore features needed by one coherent Tula module;
2. restore that module's production tests;
3. add Tula install/export and Conan package metadata;
4. create a minimal consumer of `tula/3.1.0`;
5. only then migrate `kidscpp` to the packaged Tula contract.
