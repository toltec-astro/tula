# Tula 3.1

Tula is the shared C++ utility package for TolTEC. The `v3.x` branch uses the
Conan 2 package contract supplied by `tula-cmake/3.1.0`.

The active infrastructure features are:

```text
logging
└── meta-feature: fmt + spdlog

perflibs
└── Threads + optional OpenMP + optional oneMKL
```

`logging` may be disabled or acquired from Conan, CPM, or the system. Every
enabled mode produces `tula::logging`. `perflibs` is disabled or system-resolved
and produces `tula::perflibs`.

Build from this repository with:

```sh
./build
```

The launcher obtains the pinned `tula_cmake` CLI from its GitHub release tag.
The CLI runs Conan install, reads the generated CMake preset, configures, and
builds. In the multi-repository development workspace:

```sh
TULA_CMAKE_DEV_PROJECT=../tula_cmake ./build
```

The release package declares its normal provider choices in `conanfile.py`;
users can override them with Conan profiles or `./build --option NAME=VALUE`.

## Distribution boundary

Tula no longer embeds `tula_cmake` as a Git submodule. Conan resolves the
versioned `tula-cmake/3.1.0` Python-require from the configured TolTEC remote.
The boilerplate and downstream examples are owned by the separate
`tula_cmake` repository.

The package publishes the CMake target `tula::headers`. Its `test_package`
compiles an independent consumer of that installed target after every
`conan create`. Header-only CPM dependencies without Conan packages are
included in Tula's installed header closure; normal Conan dependencies remain
explicit transitive package edges.

Run the complete workspace acceptance from the workspace root:

```sh
just all
```

The Conan 1/CMake production sources under `../refs` remain read-only.
