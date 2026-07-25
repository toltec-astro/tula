# Testing strategy

## Objectives

The test loop must provide fast Python feedback, exercise semantic CMake
features, and verify both distribution boundaries:

- installed `tula-cmake` wheel;
- packaged `tula_boilerplate` consumed independently.

## Unit and documentation gate

`just unit` runs:

- Ruff lint and formatting checks;
- `ty` static analysis;
- 16 Pytest tests, with documentation sources included in collection;
- branch coverage with an 85% threshold;
- Sphinx HTML documentation build with warnings treated as errors.

Tests cover Pydantic invariants, registry loading, missing resources, selection
rendering, convention-derived resolver paths and commands, CLI behavior,
installed resources, typed generated-preset parsing, and exact
external-command ordering.

The generated Sphinx site has a dedicated model page using
`autodoc-pydantic`; it renders field metadata, validator summaries, and JSON
schemas from the runtime classes rather than maintaining parallel tables.

`recipe.py` is excluded from unit coverage because it is executed by every real
Conan integration gate.

## Fast integration

`just fast` verifies:

- boilerplate with logging disabled;
- boilerplate using system fmt + spdlog through `tula::logging`;
- Tula using system perflibs with Threads and required GNU OpenMP.

## Provider acceptance

`just providers` builds the same boilerplate source with:

- Conan-provided fmt + spdlog;
- CPM-provided fmt + spdlog.

CPM source archives may be reused from `.devcontainer/cache/cpm/`; build/output
trees remain isolated.

## Tula compile acceptance

`just tula <mode>` compiles `tula_header_smoke` and runs CTest. Logging modes
compile both formatter and logging headers. `perflibs-system` verifies imported
Threads/OpenMP targets and exported capability definitions.

This is meaningful compile/runtime coverage, not yet full production behavior
parity.

## Wheel boundary

`just wheel`:

1. copies `tula_cmake` and boilerplate outside the uv workspace;
2. builds the wheel;
3. verifies registry, CMake, template, profile, and `py.typed` contents;
4. installs into a fresh virtual environment;
5. exports the copied Python-require into an isolated Conan home;
6. builds and runs copied boilerplate sources.

## Package-chain boundary

`just downstream`:

1. creates an isolated Conan home;
2. exports the local `tula-cmake` Python-require;
3. creates `tula-boilerplate/3.1.0`;
4. invokes `tula_downstream/build` once;
5. verifies the executable consumed packaged boilerplate.

## Commands

```sh
just unit
just fast
just providers
just boilerplate system
just tula perflibs-system
just wheel
just downstream
just all
```

Logs are retained under `.devcontainer/logs/`.

## Growth policy

Do not recreate a Cartesian matrix speculatively. For each package or semantic
feature:

1. record its versions, targets, modes, and platform requirements;
2. add schema/model tests;
3. add one minimal behavior assertion;
4. exercise every claimed provider;
5. add mixed cases only for meaningful dependency edges;
6. extend package-chain tests when public package metadata changes.
