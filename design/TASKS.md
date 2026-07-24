# Staged implementation

## Completed: provider and package-chain vertical slice

- [x] Preserve the production reference as read-only input.
- [x] Define the Conan 2 / CMake responsibility boundary.
- [x] Distribute shared recipe behavior as `tula-cmake/3.1.0`
  `python_requires`.
- [x] Add generic `formatting` provider behavior.
- [x] Make `logging` depend on `formatting`.
- [x] Generate a stable dependency-first manifest.
- [x] Make configuration-header feature macros registry-driven.
- [x] Prove disabled, subset, system, Conan, and CPM selections.
- [x] Compile and run the currently supported Tula header surface.
- [x] Build and audit the installed `tula-cmake` wheel.
- [x] Package `tula_boilerplate` as a real Conan package.
- [x] Add `tula_downstream` as an independent package consumer.
- [x] Provide a one-command downstream build entry point covering bootstrap,
  Conan install, generated CMake configure preset, and build preset.
- [x] Verify the complete slice in the rebuilt Ubuntu 24.04 / GCC 13 container.

## Next: package one real Tula module

1. Select one coherent production Tula module and enumerate its required
   dependency features.
2. Add only those features and their provider tests.
3. Restore the module's production tests and examples without behavior changes.
4. Add Tula install/export rules and Conan package metadata.
5. Add a minimal downstream consumer of `tula/3.1.0`.
6. Verify the package through the same one-command UX.

## Later: kidscpp

Move `kidscpp` only after packaged Tula passes the downstream boundary:

1. consume `tula/3.1.0` through Conan;
2. preserve the production C++ behavior and test results;
3. remove workspace-relative build coupling;
4. run standalone and package-chain acceptance in the dev container.
