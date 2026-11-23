# Tula agent guide

Tula is the shared C++ utility library for TolTEC software: a component-based
CMake package with its own Spack recipe. Zhiyuan Ma <zhiyuanma@umass.edu> is
the maintainer.

## Read first

1. `design/spec.md`: the contract. Its domains are the guides in `docs/`, in
   the same order. TulaCMake's `design/spec.md` holds the build conventions
   Tula follows.
2. `design/decisions/`: the decision records. Only records with
   `status: "accepted"` are binding.
3. `design/architecture.md`: the mechanisms that realize the contract.
4. `docs/`: the user guides.

## Ownership

- Tula owns the `tula::` components, their variants and the edges between
  them, and the adapters that only Tula components use (`tula-bitmask`,
  `tula-meta-enum`, `tula-clipp`, `tula-grppi`).
- TulaCMake owns the build conventions, the shared `tula_deps::` adapters and
  the component gates; Spack owns the concrete dependency graph;
  `tolteca_deploy` owns the deployment locations.

## Development

- Build and test from an activated `tolteca_deploy` development location, as
  `README.md` "Development" describes
  (`spack -e "$TOLTECA_CPP_ENV" install --test=root --overwrite tula`). The
  component gates run from `tula_cmake` with `just`.
- A new component needs its variant, its `requires()` edges in the recipe,
  the matching `tula_require_module()` checks and `find_dependency` calls,
  a behavior test, and a gate case in `tula_cmake`.
- Follow the `tolteca-coding-guideline` skill for names, terms and
  docstrings, and the `design-records` skill for `design/`. The skills are in
  [`tolteca_agent`](https://github.com/toltec-astro/tolteca_helpdesk/tree/main/src/tolteca_agent/data/skills).
  In the TolTECA development workspace, also follow its workspace rules. Do
  not copy their text here.
