# Tula 3.1

Tula is the shared C++ utility library for TolTEC software. On the
`v3.x_spack` branch it is an ordinary CMake package with an owned Spack recipe.
TulaCMake supplies reusable build conventions; Spack supplies the concrete
dependency graph.

## Public contract

Tula is component-based. Consumers request and link only the cohesive modules
they use:

```cmake
find_package(tula 3.1 CONFIG REQUIRED COMPONENTS ecsv)
target_link_libraries(my_target PRIVATE tula::ecsv)
```

The accepted module names and their transitive dependency graph are defined in
[design/COMPONENTS.md](design/COMPONENTS.md). There is no all-features
umbrella target.

Third-party adapters use a separate, provider-faithful namespace:

```cmake
find_package(TulaYamlCpp CONFIG REQUIRED)
target_link_libraries(my_target PRIVATE tula_deps::yaml_cpp)
```

These targets normalize CMake discovery without claiming a higher-level API.
Tula components remain under `tula::*`.

The generated `<tula/config.h>` records enabled capabilities as
`TULA_HAS_*` macros. It does not record whether Spack used an external,
source build, or binary cache.

## Spack package

The decentralized repository is `spack_repo/toltec/tula`. Its recipe owns:

- source versions;
- capability variants;
- conditional dependency edges;
- mapping variants to ordinary CMake options; and
- package test execution.

Provider adapters shared by TolTEC projects live in TulaCMake's Spack
repository. Tula's recipe depends on those packages conditionally; project
CMake does not fetch sources.

`spack_repo/develop.yaml` declares that the `tula` development spec maps to
this repository root. Location tooling composes that repo-owned declaration;
it does not duplicate Tula's source path in a central package map.

## Development

From an activated `tolteca_deploy` development location:

```console
cd ../toltec_astro_dev
source dotbashrc
just cpp-install
spack -e "$TOLTECA_CPP_ENV" install --test=root --overwrite tula
```

Select `development/linux-gcc14` or `development/linux-llvm20` in
`location.yaml`; both use C++23. The focused ECSV slice runs ten Tula tests,
an installed ECSV component consumer, a dependency-only
adapter consumer, and a missing-component rejection fixture.

ECSV table/header/loader accessors expose const references to table-owned
state. This keeps lazy header-column views valid for the lifetime of the table;
the behavior is covered both by Tula tests and Tlaloc's real tune-report test.

The `perflibs` component is measured separately with OpenMP enabled and
disabled under both compilers. Consumers request it explicitly:

```cmake
find_package(tula 3.1 CONFIG REQUIRED COMPONENTS perflibs)
target_link_libraries(my_target PRIVATE tula::perflibs)
```

The generated config reports `TULA_HAS_PERFLIBS` and `TULA_HAS_OPENMP`;
adapter-specific facts remain in `<tula_perflibs/config.h>`.

Run that complete acceptance surface with:

```console
cd ../tula_cmake
just tula-component-matrix
just tula-perflibs-matrix
just tula-enum-cli-matrix
just tula-netcdf-matrix
just tula-grppi-matrix
just tula-fitting-matrix
```

The preserved Conan implementation remains on its baseline branch and in the
workspace archive. `refs/` is read-only evidence, never a build input.
