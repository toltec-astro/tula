# Testing strategy

## Objectives

The test loop must provide quick local feedback, exercise every acquisition
provider, and verify both distribution boundaries:

- installed `tula-cmake` wheel;
- packaged `tula_boilerplate` consumed by an independent downstream project.

## Layers

### Unit

`just unit` runs:

- Ruff lint;
- Ruff format check;
- ty;
- seven pure Python model tests.

The model tests cover the four-mode contract, registry validation, disabled
prerequisites, dependency-first ordering, deterministic manifest generation,
and installed profile discovery.

### Fast integration

`just fast` builds and executes `tula_boilerplate` with:

- both features disabled;
- formatting from the system with logging disabled;
- formatting and logging from the system.

This proves feature subsets and the system provider without network downloads.

### Provider acceptance

`just providers` builds and executes the same source with:

- formatting/logging from Conan;
- formatting/logging from CPM.

Every provider run uses a fresh Conan output folder. CPM source archives may be
reused from `.devcontainer/cache/cpm/`.

### Tula compile acceptance

`just tula <mode>` configures Tula, compiles `tula_header_smoke`, and runs it
with CTest. The target always compiles the supported dependency-free core. It
also compiles formatting and logging headers when those features are enabled.

This gate does not claim full production behavior parity.

### Wheel boundary

`just wheel`:

1. copies `tula_cmake` and boilerplate outside the uv workspace;
2. builds and audits the wheel;
3. installs it into a fresh virtual environment;
4. exports the copied Conan python-require into an isolated Conan home;
5. locates the packaged GCC 13 profile;
6. builds and runs the copied boilerplate.

### Package-chain boundary

`just downstream`:

1. creates an isolated Conan home;
2. exports the local `tula-cmake` python-require;
3. creates `tula-boilerplate/3.1.0`;
4. invokes `tula_downstream/build` once;
5. verifies that the downstream executable consumed the packaged boilerplate.

This is the UX acceptance test. The checked-in downstream command performs the
bootstrap, Conan, and CMake phases without hiding them in CMake.

## Commands

The root `justfile` is the development and CI interface:

```sh
just unit
just fast
just providers
just boilerplate formatting-only
just tula conan
just wheel
just downstream
just all
```

Each gate streams output and keeps durable logs under `.devcontainer/logs/`.

## Growth policy

Do not recreate the old Cartesian package matrix. For each feature:

1. add registry/schema tests;
2. add one minimal consumer behavior assertion;
3. test each supported provider;
4. test declared dependency edges and meaningful mixed-provider cases;
5. extend package-chain tests only when public package metadata changes.

Coverage grows with contracts, providers, and dependency edges rather than all
possible option combinations.
