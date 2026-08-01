# Tula components and dependency targets

Status: implemented and measured on the unpublished `v3.x_spack` branch.

## 1. Boundary

The public CMake API has two layers with different responsibilities:

```text
TulaCMake + Spack adapters       Tula C++ library
-------------------------       ----------------
tula_deps::logging        ────> tula::logging
tula_deps::yaml_cpp       ────> tula::yaml
tula_deps::eigen3         ────> tula::eigen
tula_deps::csv_parser     ─┐
                           ├──> tula::ecsv
tula::logging/yaml/eigen  ─┘
tula_deps::perflibs       ────> tula::perflibs
```

`tula_deps::*` targets retain provider-shaped names because they normalize
only CMake discovery and target spelling. They do not abstract one library
behind another library's API.

`tula::*` targets are higher-level Tula components. They carry Tula headers
and the complete transitive usage requirements of those headers.

`tula_cmake::*` is reserved for CMake infrastructure targets if such targets
are ever needed. Dependency adapters are not CMake implementation details, so
they do not use that namespace.

This branch is unreleased and has no compatibility aliases.

## 2. Direct dependency-only use

A project can use an adapter without finding Tula or including a Tula header:

```cmake
find_package(TulaYamlCpp CONFIG REQUIRED)

add_library(reader reader.cpp)
target_link_libraries(reader PRIVATE tula_deps::yaml_cpp)
```

The currently accepted adapter packages are:

| Config package | Public target | Upstream contract |
| --- | --- | --- |
| `TulaLogging` | `tula_deps::logging` | fmt 9.1 + spdlog 1.12 |
| `TulaYamlCpp` | `tula_deps::yaml_cpp` | yaml-cpp 0.8 |
| `TulaCsvParser` | `tula_deps::csv_parser` | pinned Jerry-Ma csv-parser |
| `TulaEigen3` | `tula_deps::eigen3` | Eigen 3.4 |
| `TulaPerflibs` | `tula_deps::perflibs` | Threads and optional resolved OpenMP runtime |
| `TulaNetcdfCxx4` | `tula_deps::netcdf_cxx4` | NetCDF C++4 and NetCDF C |

Each package owns a relocatable config file. The CMake-built adapters live
under `tula_cmake/packages/`; csv-parser is installed by its Spack recipe
because the pinned upstream is header-only and has no suitable installable
CMake package.

## 3. Tula component use

A Tula consumer requests the behavior it uses:

```cmake
find_package(tula 3.1 CONFIG REQUIRED COMPONENTS ecsv)
target_link_libraries(reader PRIVATE tula::ecsv)
```

The same component identity is used consistently:

```text
Spack variant   +ecsv
CMake component ecsv
exported target tula::ecsv
CMake option    TULA_ENABLE_ECSV
config macro    TULA_HAS_ECSV
```

The implemented component slice is:

| Component | Public target | Tula behavior | Direct requirements |
| --- | --- | --- | --- |
| `core` | `tula::core` | dependency-free traits, concepts, metadata, and configured capability header | C++23 |
| `logging` | `tula::logging` | Tula logging and fmt formatter headers | `tula_deps::logging` |
| `yaml` | `tula::yaml` | Tula YAML-backed configuration API | `tula::logging`, `tula_deps::yaml_cpp` |
| `eigen` | `tula::eigen` | Tula Eigen utilities, formatters, and nddata integration | `tula::logging`, `tula_deps::eigen3` |
| `ecsv` | `tula::ecsv` | ECSV headers, metadata conversion, typed tables, and streaming row loading | `tula::logging`, `tula::yaml`, `tula::eigen`, `tula_deps::csv_parser` |
| `perflibs` | `tula::perflibs` | Tula's platform-parallelism capability and compile interface | `tula_deps::perflibs` |
| `enum` | `tula::enum` | reflected enums, bit flags, and fmt formatting | `tula::logging`, `tula_deps::bitmask`, `tula_deps::meta_enum` |
| `cli` | `tula::cli` | typed Clipp-to-configuration construction | `tula::logging`, `tula::enum`, `tula_deps::clipp` |
| `netcdf` | `tula::netcdf` | NetCDF type dispatch, scalar access, and diagnostics | `tula::eigen`, `tula_deps::netcdf_cxx4` |
| `grppi` | `tula::grppi` | dynamic sequential/native/OpenMP execution policy | `tula::logging`, `tula::enum`, `tula::perflibs`, `tula_deps::grppi` |
| `fitting` | `tula::fitting` | Ceres-backed Eigen fitting utilities | `tula::logging`, `tula::eigen`, `Ceres::ceres` |

All Tula components currently required by Kidscpp and Citlali have focused
package and installed-consumer coverage.

`ECSVTable` owns its header, loader, and header view. Const accessors return
stable references to that state; callers may iterate lazy column views without
creating a temporary owner. The Tlaloc real tune-report test exercises this
borrowed-view lifetime in addition to Tula's focused ECSV tests.

## 4. Minimal dependency closure

All optional Tula variants default off. Spack `requires()` directives encode
component-to-component edges before the build starts:

```text
+ecsv
├── +logging ──> tula-logging ──> fmt + spdlog
├── +yaml ─────> tula-yaml-cpp ─> yaml-cpp
├── +eigen ────> tula-eigen3 ───> Eigen
└──────────────> tula-csv-parser
```

The measured `tula+ecsv` spec is:

```text
+logging +yaml +eigen +ecsv
~netcdf ~enum ~cli ~perflibs ~openmp ~grppi ~fitting
```

It contains no Ceres, NetCDF, GrPPI, OpenMP, or unrelated Tula dependency.

The independent perflibs slice has two valid closures:

```text
+perflibs +openmp ──> tula-perflibs+openmp ──> Threads + compiler OpenMP
+perflibs ~openmp ──> tula-perflibs~openmp ──> Threads
```

`<tula/config.h>` reports the high-level capabilities as
`TULA_HAS_PERFLIBS` and `TULA_HAS_OPENMP`. The adapter-owned
`<tula_perflibs/config.h>` reports the provider facts
`TULA_PERFLIBS_HAS_THREADS` and `TULA_PERFLIBS_HAS_OPENMP`. The two layers do
not define each other's macros.

## 5. Installed component verification

The installed `tulaConfig.cmake`:

1. discovers dependencies required by the installed component closure;
2. imports `tulaTargets.cmake`;
3. sets `tula_<component>_FOUND` only when that target exists; and
4. calls CMake's generated `check_required_components(tula)`.

Consequently, requesting an absent required component makes `tula_FOUND`
false during configuration. There is no umbrella target that can silently
mask the mismatch.

## 6. Acceptance evidence

`just tula-component-matrix` performs the same checks under GCC 14.2 and
LLVM/Clang 20.1.2, both in C++23 mode:

1. concretize `tula+ecsv`;
2. assert the exact enabled/disabled variant set;
3. assert that all four adapter packages and no Ceres package occur in the
   concrete DAG;
4. build each package through Spack with package tests enabled;
5. run ten Tula tests, including real TolTEC tune reports;
6. build a consumer of all four `tula_deps::*` targets without Tula;
7. build an installed consumer of `tula COMPONENTS ecsv`; and
8. prove that a missing required component fails configuration.

The real-data test loads eleven regular tune reports from
`tolteca_test_data/data_lmt/toltec/reduced` in the dev container. When the
work-directory tune-report symlinks resolve in the execution environment,
those files are preferred automatically.

Relevant native mechanisms:

- [CMake package components and `check_required_components`](https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html)
- [Spack variants](https://spack.readthedocs.io/en/latest/spec_syntax.html#variants)
- [Spack package `requires()` directives](https://spack.readthedocs.io/en/latest/spack.html#spack.directives.requires)

`just tula-perflibs-matrix` independently verifies `+openmp` and `~openmp`
under both compilers. Every case checks the minimal graph, runs focused Tula
package tests, and builds an installed
`find_package(tula COMPONENTS perflibs)` consumer.

`just tula-enum-cli-matrix` verifies two more closures in both compiler lanes:

```text
+enum ──> +logging + tula-bitmask + tula-meta-enum
+cli  ──> +enum +logging + tula-clipp
```

The enum-only graph rejects Clipp, and both graphs reject YAML, Eigen, ECSV,
perflibs, NetCDF, GrPPI, and Ceres. Separate installed consumers request
`COMPONENTS enum` and `COMPONENTS cli`.

`just tula-netcdf-matrix` verifies:

```text
+netcdf
├── +eigen ──> +logging
└── tula_deps::netcdf_cxx4 ──> netcdf-cxx4 ──> netcdf-c
```

The installed consumer creates a NetCDF file, writes and reads a scalar
through `tula::nc_utils`, and removes the file. YAML, ECSV, enum, CLI,
perflibs, GrPPI, and Ceres remain absent.

`just tula-grppi-matrix` verifies `+grppi` with `+openmp` and `~openmp` under
both compilers. The high-level component owns logging, enum, and perflibs
requirements; the provider-faithful `tula_deps::grppi` adapter owns only the
third-party headers. Enabled cases execute an OpenMP map, while disabled cases
prove that the `omp` execution mode is rejected.

`just tula-fitting-matrix` verifies that `+fitting` brings only logging,
Eigen, and Ceres. Ceres already exports the canonical `Ceres::ceres` target,
so an additional target adapter would add no normalization value.
