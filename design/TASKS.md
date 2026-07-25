# Staged implementation

## Completed: typed infrastructure and vertical slice

- [x] Preserve `refs/` as read-only production input.
- [x] Define the Conan/CMake responsibility boundary.
- [x] Distribute recipe behavior as `tula-cmake/3.1.0`
  `python_requires`.
- [x] Provide the one-command bootstrap → Conan → generated-preset workflow.
- [x] Refactor Python into a conventional `src/` layout.
- [x] Validate registry and generated-preset inputs with frozen Pydantic models.
- [x] Separate executable code from installed CMake/templates/profiles data.
- [x] Split packaged CMake into infrastructure and feature resolver layers.
- [x] Derive resolver module and command names from the feature identifier.
- [x] Add Ruff, `ty`, Pytest, coverage, Sphinx, and PEP 561 typing.
- [x] Render Pydantic fields, validators, and JSON schemas in Sphinx.
- [x] Treat documentation warnings as build failures.
- [x] Link the project to the TolTEC `v2026` cookiecutter through Cruft.
- [x] Keep logging as one fmt + spdlog meta-feature.
- [x] Port perflibs Threads/OpenMP/oneMKL configuration and validation.
- [x] Verify required GNU OpenMP in the GCC 13 dev container.
- [x] Generate feature/provider configuration-header macros from the registry.
- [x] Package `tula_boilerplate` and consume it from `tula_downstream`.
- [x] Remove the unused parent-header symlink, empty pre-refactor folders, and
  orphan aggregate examples CMake file.

## Next: system and package matrix

1. Add a oneAPI-capable validation image.
2. Test `MKL::MKL` with sequential, OpenMP, and TBB threading contracts.
3. Build the package support matrix one package per row.
4. Record provider modes and target names from executable evidence.
5. Introduce dependency splitting only when a concrete package requires it.

## Then: package one real Tula module

1. Select one coherent production module.
2. Restore its production tests without behavior changes.
3. Add only the dependency features demonstrated by that module.
4. Complete Tula install/export and Conan package metadata.
5. Verify a minimal consumer of `tula/3.1.0` through the one-command workflow.

## Later: kidscpp

1. Consume packaged `tula/3.1.0`.
2. Preserve production C++ behavior and test results.
3. Remove workspace-relative build coupling.
4. Run standalone and package-chain acceptance in the dev container.
