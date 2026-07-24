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

The root `justfile` is the test interface. Dev-container provisioning installs
the tools but contains no test orchestration:

```sh
just unit
just fast
just providers
just boilerplate cpm
just tula conan
just wheel
just all
```

Each gate streams output and keeps durable logs under `.devcontainer/logs/`.

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
