# Tula specification

Tula is the shared C++ utility library of the TolTEC software: logging,
configuration, ECSV tables, Eigen and NetCDF helpers, enums and command lines,
parallel execution, and fitting. Kidscpp, Citlali and affiliated packages
(Tlaloc) use it. It is an installed CMake package built by its Spack recipe.

Tula follows the TulaCMake specification for CMake conventions, dependency
adapters, Spack recipes and acceptance gates (`tula_cmake/design/spec.md`).
This specification states only what Tula adds.

Domains, in documentation-chapter order:

1. Components
2. Fitting

## Components

A consumer builds and links only the parts of Tula it uses, and gets only
their dependencies.

- Tula is a set of components. Each component is one installed target
  `tula::<component>`; there is no umbrella target and there are no
  compatibility aliases.
- One name identifies a component everywhere: Spack variant `+<component>`,
  CMake component `<component>`, target `tula::<component>`, CMake option
  `TULA_ENABLE_<COMPONENT>`, configuration macro `TULA_HAS_<COMPONENT>`.
- Every component is off by default. A component declares the components and
  adapters its headers need, and the recipe enforces those edges before the
  build starts. A build with a set of components contains no dependency that
  only other components need (for example, no Ceres without `fitting`).
- `tula::core` is always installed: dependency-free traits, concepts and the
  configured capability header.
- `find_package(tula 3.1 CONFIG REQUIRED COMPONENTS <names>)` succeeds only
  when every requested component is installed. A missing component fails the
  configuration.
- `<tula/config.h>` reports the installed components and OpenMP as
  `TULA_HAS_*` macros. It does not report how Spack provided a dependency. An
  adapter's provider facts stay in that adapter's own header.
- ECSV tables own their header, loader and header view. Their const accessors
  return references that stay valid for the table's lifetime, so lazy column
  views need no separate owner.

| Component | Provides | Requires |
| --- | --- | --- |
| `core` | traits, concepts, metadata, capability header | — |
| `logging` | logging and fmt formatters | `tula_deps::logging` |
| `yaml` | YAML-backed configuration | `logging`, `tula_deps::yaml_cpp` |
| `eigen` | Eigen utilities, formatters, nddata | `logging`, `tula_deps::eigen3` |
| `ecsv` | ECSV headers, metadata, typed tables, streaming rows | `logging`, `yaml`, `eigen`, `tula_deps::csv_parser` |
| `perflibs` | platform parallelism capability | `tula_deps::perflibs` |
| `enum` | reflected enums, bit flags, formatting | `logging`, `tula_deps::bitmask`, `tula_deps::meta_enum` |
| `cli` | typed command-line construction | `logging`, `enum`, `tula_deps::clipp` |
| `netcdf` | NetCDF type dispatch, scalar access, diagnostics | `eigen`, `tula_deps::netcdf_cxx4` |
| `grppi` | sequential, native or OpenMP execution policy | `logging`, `enum`, `perflibs`, `tula_deps::grppi` |
| `fitting` | Ceres-based fitting helpers | `logging`, `eigen`, `Ceres::ceres` |

The `openmp` variant enables OpenMP in `perflibs` and `grppi` and requires
`perflibs`.

## Fitting

A model author fits caller-defined equations with Ceres without repeating the
parameter, loss and solver setup in each model.

- `tula::fitting` requires Ceres 2.2 or later and uses its `Ceres::ceres`
  target directly.
- The caller owns the equations (templated residual functors) and the
  parameter storage. The helpers own no model class and no workflow policy.
- A parameter block is registered once, with optional per-parameter initial
  value, fixed value and bounds. Invalid settings (non-finite values,
  conflicting initial and fixed values, unordered bounds, an initial value
  outside its bounds, a size mismatch, a second registration) raise
  `std::invalid_argument` before any Ceres call. Partly fixed blocks keep the
  free parameters variable; a fully fixed block is constant.
- A residual group is added with automatic differentiation. Its loss is
  linear unless the caller selects Huber or Cauchy with an explicit scale in
  the caller's residual units. A dynamic group takes an explicit positive
  residual count.
- The default solver options use dense QR and one thread; callers
  parallelize independent fits. Solving returns the complete Ceres summary;
  whether a solution is usable and whether it converged are separate
  questions for the caller.
- The caller keeps the parameter storage alive and in place while the
  problem uses it. The problem uses Ceres's default ownership of costs,
  losses and manifolds.

## Handoffs

- **TulaCMake** owns the CMake conventions, the shared adapters
  (`tula_deps::logging`, `yaml_cpp`, `eigen3`, `csv_parser`, `perflibs`,
  `netcdf_cxx4`) and the `tula-*-matrix` gates that verify each component
  closure. Tula's own recipe repository holds the adapters that only Tula
  components use: `tula-bitmask`, `tula-meta-enum`, `tula-clipp`,
  `tula-grppi`. The version header records artifact identity only.
- **Kidscpp, Citlali** request the components they use with
  `find_package(tula ... COMPONENTS ...)` and the matching variants on their
  `tula` dependency.
- **Affiliated packages** (Tlaloc) use Tula the same way. Tula's tests do not
  depend on them.
