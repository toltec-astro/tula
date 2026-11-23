# Tula

Tula is the shared C++ utility library of the TolTEC software: logging,
configuration, ECSV tables, Eigen and NetCDF helpers, enums and command lines,
parallel execution, and Ceres-based fitting. It is a CMake package made of
components; a project requests the components it uses and builds only their
dependencies. Spack builds it from the recipe in `spack_repo/toltec/tula`.

```cmake
find_package(tula 3.1 CONFIG REQUIRED COMPONENTS ecsv)
target_link_libraries(my_target PRIVATE tula::ecsv)
```

## Documentation

| Guide | Contents |
| --- | --- |
| [Components](docs/components.md) | choose, request and link components; capability macros; ECSV tables |
| [Fitting](docs/fitting.md) | fit caller-defined models with Ceres |

Build conventions, the `tula_deps::*` adapters, recipe rules and the
acceptance gates are documented in
[TulaCMake](https://github.com/toltec-astro/tula_cmake).

## Development

From an activated `tolteca_deploy` development location that lists this
checkout as a development source:

```bash
spack -e "$TOLTECA_CPP_ENV" install --test=root --overwrite tula
```

The component gates (`just tula-component-matrix` and the other
`tula-*-matrix` recipes) run from the sibling `tula_cmake` checkout.
