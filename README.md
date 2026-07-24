# Tula v3.1 development

This branch implements a Conan 2.31-centric superbuild with a one-command
downstream user experience.

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

The workspace demonstrates:

- `tula_cmake`: typed superbuild infrastructure distributed as a Python wheel
  and Conan `python_requires`;
- `examples/tula_boilerplate`: minimal real package using `tula_cmake`;
- `examples/tula_downstream`: independent consumer of the packaged
  boilerplate;
- `tula`: actual C++ package with logging and perflibs compile acceptance.

From the downstream project:

```sh
./build
```

The command bootstraps `tula-cmake`, runs `conan install`, reads the generated
CMake preset, configures, and builds. `TULA_CMAKE_DEV_PROJECT` selects the local
uv project during workspace development.

Run the complete container acceptance with:

```sh
just all
```

The Conan 1/CMake production sources under `../refs` remain read-only. See
`design/ARCHITECTURE.md`, `design/TESTING.md`, and `design/TASKS.md`.
