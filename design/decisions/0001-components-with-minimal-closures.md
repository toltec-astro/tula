---
status: "accepted"
date: 2026-07-31
decision-makers: user [cpp 2026-07-31 01:59, 02:12]
---

# Tula is built as components with minimal dependency closures

## Context and Problem Statement

Tula collects unrelated utilities; some need heavy dependencies (Ceres for
fitting, NetCDF, GrPPI). Should Tula be one package build with all features,
or be split so that a consumer builds only what it uses?

## Considered Options

* One build with all features (a single target, every dependency)
* Components with explicit edges; each build contains only the requested
  components and their requirements

## Decision Outcome

Chosen option: "Components with explicit edges". The user [cpp 2026-07-31
01:59]: "We should also make sure that we can build tula in such way that
only needed components are made avaialbe (e.g., skipping compiling ceres if
we are not doing the fitting). This is critical to avoid extra useless time
spent on compiling deps that are not relevant." and "We should also make sure
that we define clear the dependecy relation between the tula components. For
example, almost in all cases we will need logging because the ecsv portion
of tula will call logging functions to print stuff. Need to test and
formalize that aspect within tula". Restated as a requirement [cpp
2026-07-31 02:12]: "tula has components and componets can be pulled in on
individual basis. when individual componets is pulled in, only needed deps
get pulled in and compiled".

### Consequences

* Good, because a consumer that does not request `fitting` never compiles
  Ceres.
* Good, because the edges are checked twice: by Spack before the build and
  by CMake in a plain build.
* Bad, because each component closure needs its own gate
  (`tula-*-matrix` in TulaCMake), and a new component needs a new gate case.
