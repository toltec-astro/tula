# Components

Tula is a set of components. A project requests the components it uses, and
its build gets only their dependencies.

## Choose components

| Component | Use it for | Brings in |
| --- | --- | --- |
| `core` | traits, concepts, `<tula/config.h>` | nothing; always installed |
| `logging` | logging and fmt formatters | fmt, spdlog |
| `yaml` | YAML-backed configuration | `logging`, yaml-cpp |
| `eigen` | Eigen utilities, formatters, nddata | `logging`, Eigen |
| `ecsv` | ECSV tables: headers, metadata, typed columns, streaming rows | `logging`, `yaml`, `eigen`, csv-parser |
| `perflibs` | Threads and, with `+openmp`, OpenMP | Threads, OpenMP runtime |
| `enum` | reflected enums and bit flags | `logging`, bitmask, meta_enum |
| `cli` | typed command-line construction | `logging`, `enum`, Clipp |
| `netcdf` | NetCDF type dispatch and scalar access | `eigen`, NetCDF C++4 |
| `grppi` | sequential, native or OpenMP execution policy | `logging`, `enum`, `perflibs`, GrPPI |
| `fitting` | least-squares fits with Ceres ([Fitting](fitting.md)) | `logging`, `eigen`, Ceres 2.2+ |

Enabling a component enables the components it requires. For example,
`tula+ecsv` concretizes as `+logging +yaml +eigen +ecsv`, with every other
component off and no Ceres or NetCDF in the graph.

## Request components

In the consumer's Spack recipe, request the variants:

```python
depends_on("tula@3.1:3+ecsv", type=("build", "link"))
```

In its CMake, find and link the same components:

```cmake
find_package(tula 3.1 CONFIG REQUIRED COMPONENTS ecsv)
target_link_libraries(my_target PRIVATE tula::ecsv)
```

If the installed Tula lacks a requested component, `find_package` fails at
configuration. Link each component you include headers from; there is no
target that links all of them.

The same name is used at every level:

| Level | Name for `ecsv` |
| --- | --- |
| Spack variant | `+ecsv` |
| CMake component | `ecsv` |
| Target | `tula::ecsv` |
| CMake option (plain CMake build) | `TULA_ENABLE_ECSV` |
| Configuration macro | `TULA_HAS_ECSV` |

## Check capabilities in code

`<tula/config.h>` defines `TULA_HAS_<COMPONENT>` and `TULA_HAS_OPENMP` as 0
or 1:

```cpp
#include <tula/config.h>

#if TULA_HAS_OPENMP
// use the OpenMP execution mode
#endif
```

Whether OpenMP or Threads came from the host or from a Spack build is not
reported there. Use `<tula_perflibs/config.h>` for the facts of the
`tula-perflibs` adapter.

## Use ECSV tables

An `ECSVTable` owns its header, loader and header view. Const accessors return
references to that state, valid for the table's lifetime, so column views can
be iterated without copying them out:

```cpp
#include <tula/ecsv/table.h>
```

`examples/ecsv_reader` reads ECSV files with this API.

## Use an adapter without Tula

The `tula_deps::*` adapters can be used directly; see TulaCMake's
`docs/dependency-adapters.md`.
