# Tula v3.1 development

This branch implements a Conan 2.31-centric feature/provider superbuild.

The active vertical slice contains two logical features:

```text
formatting
└── logging depends on formatting
```

Each feature may be disabled or acquired from Conan, CPM, or the system.
Enabled providers normalize to `tula::formatting` and `tula::logging`.

The workspace demonstrates three infrastructure levels:

- `tula_cmake`: reusable superbuild infrastructure distributed as a Python
  wheel and Conan `python_requires`;
- `examples/tula_boilerplate`: a minimal real package built with
  `tula_cmake`;
- `examples/tula_downstream`: an independent consumer of the packaged
  boilerplate.

From the downstream project the complete build is one command:

```sh
./build
```

The command bootstraps `tula-cmake` with `uvx`, runs `conan install`, reads the
generated CMake preset, configures, and builds. During workspace development,
`TULA_CMAKE_DEV_PROJECT` selects the local uv project.

Run the full container acceptance with:

```sh
just all
```

The production Conan 1/CMake sources under `../refs` are read-only references.
See `design/ARCHITECTURE.md`, `design/TESTING.md`, and `design/TASKS.md`.
