# Testing strategy

## Objectives

The test loop must expose regressions without making every edit wait for every
network-backed provider.

## Layers

### Unit: seconds

Pure Python tests validate the registry schema, four-mode contract, dependency
validation, and deterministic CMake manifest rendering.

### Fast integration: normally under a minute

Build and execute `tula_boilerplate` with:

- `logging=disabled`, proving feature subsets work;
- `logging=system`, proving installed-package discovery and normalized targets.

This is the default gate because it performs no dependency download.

### Provider acceptance

Build and execute the same unchanged downstream source with:

- `logging=conan`;
- `logging=cpm`.

Caches make repeated execution fast; the first run may download immutable
inputs.

### Full gate

Run lint, type checking, unit tests, then all four downstream modes. CI and
milestone commits use this gate.

## Commands

The test runner is intentionally not part of dev-container provisioning.
Provisioning prepares tools; acceptance invokes the project interfaces
directly:

```sh
cd tula/tula_cmake
uv run ruff check .
uv run ruff format --check .
uv run ty check
uv run python -m unittest discover -s tests -v

base="$(uv run tula-cmake profile linux-gcc13-debug)"
```

For each provider mode, run `conan install` with `base`, the corresponding
`examples/tula_boilerplate/profiles/logging-*` option profile, and a fresh
`--output-folder`; then configure, build, and execute `tula_boilerplate`.
Redirect through `tee` when a durable log is required. Dev-container
acceptance logs are kept under `.devcontainer/logs/`.

Every provider run uses a fresh temporary Conan output folder. This prevents a
generated `CMakeDeps` file from one mode from satisfying another mode by
accident. CPM source archives may be cached separately under
`.devcontainer/cache/cpm/`, so isolation does not require repeated downloads.

## Growth policy

Do not recreate the previous Cartesian package matrix. For each new feature:

1. Add pure registry tests.
2. Add one minimal consumer behavior assertion.
3. Test every supported provider for that feature.
4. Add combination tests only for declared dependencies or known interactions.

Coverage grows with providers and dependency edges rather than every possible
combination.
