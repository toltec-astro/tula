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
- [x] Move provider-mode dispatch into the generic CMake driver.
- [x] Export one `tula_resolve_<feature>_<mode>()` entry point per mode.
- [x] Project enabled-feature options through generated preset cache variables.
- [x] Accept repeatable root-package `--option NAME=VALUE` CLI overrides.
- [x] Add Ruff, `ty`, Pytest, coverage, Sphinx, and PEP 561 typing.
- [x] Render Pydantic fields, validators, and JSON schemas in Sphinx.
- [x] Treat documentation warnings as build failures.
- [x] Link the project to the TolTEC `v2026` cookiecutter through Cruft.
- [x] Keep logging as one fmt + spdlog meta-feature.
- [x] Port perflibs Threads/OpenMP/oneMKL configuration and validation.
- [x] Verify required GNU OpenMP in the GCC 13 dev container.
- [x] Generate feature/provider configuration-header macros from the registry.
- [x] Package `tula_boilerplate` and consume it from `tula_downstream`.
- [x] Replace boilerplate matrix profiles with registry-driven Pytest feature
  matrix tests.
- [x] Derive one matrix case for every provider and feature-option value.
- [x] Capability-gate oneAPI and alternate OpenMP runtime cases explicitly.
- [x] Add Conan/CPM/system yaml-cpp providers and matrix cases.
- [x] Port ECSV core/header behavior tests to the GCC 13 CTest gate.
- [x] Update ECSV fmt formatters for fmt 11's const formatter API.
- [x] Add Eigen Conan/CPM/system providers and multithreading matrix cases.
- [x] Port Eigen and Eigen-backed nddata behavior tests.
- [x] Port ECSV typed table loading and repair its column-reference API.
- [x] Remove the direct include of Eigen's internal `XprHelper.h`.
- [x] Add the pinned header-only csv-parser CPM feature and matrix cases.
- [x] Port production ECSV streaming row loading, including custom delimiters
  and quoted fields.
- [x] Port FlatConfig and YamlConfig behavior tests using existing feature
  targets rather than inventing package-less resolver entries.
- [x] Port filename/filesystem behavior tests on the C++23 standard library.
- [x] Add separate NetCDF C Conan/system and C++ CPM/system feature contracts.
- [x] Verify all six NetCDF provider cases on GCC 13.
- [x] Port and expand the Tula NetCDF behavior test.
- [x] Add pinned bitmask and meta-enum CPM features and all four matrix cases.
- [x] Port the production enum/bitmask behavior test and correct the dormant
  `BitFlag` concept constraint.
- [x] Add clipp 1.2.3 Conan/CPM providers and all three matrix cases.
- [x] Port the production CLI builder behavior test and normalize enum defaults
  consistently in configuration and help text.
- [x] Remove the unused parent-header symlink, empty pre-refactor folders, and
  orphan aggregate examples CMake file.
- [x] Add a pinned GrPPI CPM feature, feature-matrix probe, and Tula behavior
  test.
- [x] Install/package `tula/3.1.0` and publish `tula::headers`.
- [x] Allow recipes to declare validated registry-backed
  `tula_default_options`.
- [x] Restore the archived kidscpp v3 timestream-only scope as a packaged Tula
  consumer.
- [x] Remove sweep fitting, Ceres coupling, and private GCC variant
  specializations from the active kidscpp model.
- [x] Verify Welch/PSD and deterministic timestream solver behavior.
- [x] Create Citlali `v4.x_conan2` directly from `v4.x`; leave the reference
  checkout untouched.
- [x] Add Conan-backed Spectra, Boost, FFTW, CCfits, and Ceres features with
  disabled/provider matrix probes.
- [x] Align Eigen at 3.4.0 across Tula, Spectra, and Ceres.
- [x] Propagate public Conan headers and libraries across the
  Tula → kidscpp → Citlali package chain.
- [x] Build and package the five-source Citlali v4 library on GCC 13.
- [x] Update ECSV and Ceres call sites for current, behavior-equivalent APIs.
- [x] Add Gaussian model behavior regressions.
- [x] Split `tula_cmake` into a top-level release repository.
- [x] Move boilerplate and downstream ownership into `tula_cmake`.
- [x] Remove the released Tula submodule and uv-workspace dependency.
- [x] Add the same pinned one-command launcher to boilerplate, Tula, kidscpp,
  and Citlali.
- [x] Add installed-target Conan `test_package` consumers to Tula, kidscpp,
  and Citlali.
- [x] Define the GitHub-source plus TolTEC Conan-remote distribution boundary.

## Next: system and package matrix

1. Add a oneAPI-capable validation image.
2. Test `MKL::MKL` with sequential, OpenMP, and TBB threading matrix cases.
3. Continue the package support matrix one package per row.
4. Record provider modes and target names from executable evidence.
5. Introduce dependency splitting only when a concrete package requires it.

## Then: continue real Tula modules

1. Select the next GCC13-compatible dependency-backed Tula module.
2. Restore each module's production tests without behavior changes.
3. Add only the dependency features demonstrated by that module.
4. Expand behavior tests only with the dependency package being introduced.

## Current: Citlali completion

1. Deploy the TolTEC Artifactory CE virtual Conan remote and publish shared
   client configuration.
2. Add CI package creation, lockfile capture, upload, and promotion in
   dependency order.
3. Decide which current v4 CLI behaviors belong above the library boundary.
4. Add NetCDF result serialization fixtures to kidscpp.
5. Keep kidscpp sweep finding/fitting, GUI, and the former broad CLI out of
   the v3 package.
