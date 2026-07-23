# tula v3.1 development

This branch rebuilds the dependency feature/provider system from a minimal
vertical slice. The only registered feature is currently `logging`, and the
downstream `tula_boilerplate` demonstrates all four supported selections:

```text
disabled | conan | cpm | system
```

The previous provider-matrix implementation is preserved on the local
`archive/v3-provider-matrix-2026-07-23` branches and in the workspace archive.
The production Conan 1/CMake code under `../refs` is reference-only.

See `design/ARCHITECTURE.md` and `design/TESTING.md` before extending the
registry.
