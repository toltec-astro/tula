# Staged implementation

## Current milestone: logging vertical slice

- [x] Preserve and archive the previous v3 implementation.
- [x] Restore `refs` to the tracked production state.
- [x] Define the revised Conan/CPM/system boundary.
- [x] Implement the pure feature registry.
- [x] Implement logging providers and `tula::logging`.
- [x] Implement generated downstream configuration headers.
- [x] Reduce the test harness to layered provider acceptance.
- [x] Pass unit, disabled, system, Conan, and CPM validation.
- [x] Audit the built wheel and build a standalone boilerplate copy from it.
- [x] Rebuild a minimal dev container and repeat the full GCC 13 acceptance.
- [x] Commit the accepted logging slice.

## Next, only after this milestone

1. Add one simple header-only feature to prove the generic path.
2. Add dependency-edge validation with one feature depending on another.
3. Decide whether system wrapper recipes are needed for that feature.
4. Port Tula modules incrementally from the production reference.
5. Reintroduce `kidscpp` only after Tula itself is packaged and tested.
