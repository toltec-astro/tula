# Ceres helper contract

`<tula/ceres/fit.h>` belongs to `tula::fitting` and requires Ceres >=2.2.
Scientific equations remain caller-owned templates; helper types neither inherit
from model classes nor own workflow policy.

- `configure_parameters(problem, span, settings)` registers one stable mutable
  parameter block exactly once. Validate finite initialization, fixed values,
  bounds and sizes before Ceres APIs with fatal precondition checks. Partial
  constants use `SubsetManifold`; all constants use a constant block.
- `add_autodiff<R,P>` adds one meaningful residual group, transferring ownership
  of its statically differentiated functor/cost/optional loss to the Problem.
  A complex measurement can be a two-component group. Linear loss is the
  default; Huber/Cauchy scale is explicit in caller-defined residual units.
  `R=ceres::DYNAMIC` accepts an explicit positive residual count for batched
  linear objectives; do not mistake whole-batch robustification for pointwise
  outlier handling.
- `dense_options` chooses DENSE_QR and one thread. Callers may supply other
  validated Ceres options and own parallelism across independent fits.
- `solve` returns the complete `ceres::Solver::Summary`. Usability and
  convergence are different questions; preserve the parameters corresponding
  to any retained summary when comparing multiple passes.

The Problem must use Ceres' default cost/loss/manifold ownership options. The
caller keeps parameter storage alive and unmoved while Problem uses it. These
helpers do not configure an already registered block or create a global pool.

`<tula/algorithm/ei_ceresfitter.h>` remains a compatibility adapter for existing
model layouts. It packs real/complex Eigen views explicitly, including strided
data, and requires mutable contiguous parameter storage and mutable outputs.
It has no `const_cast` or reinterpretation of complex storage. Compatibility
calls retain whole-dataset Cauchy(2) and 50 iterations with default Ceres stopping
tolerances. An explicit final `LossOptions` argument can override this policy;
new helper calls default to linear loss. Positive infinite uncertainty disables
a residual, preserving legacy boxcar windows; zero, negative and NaN errors,
unknown parameter names and incompatible shapes are rejected.
This adapter is not the recommended base class for new model families.

Research references (checked October 5, 2026):
[Ceres modeling](https://ceres-solver.readthedocs.io/latest/nnls_modeling.html),
[dense QR and solver summaries](https://ceres-solver.readthedocs.io/latest/nnls_solving.html),
[released tags](https://github.com/ceres-solver/ceres-solver/tags).
The latest visible published tag is 2.2.0; newer online examples can use
unreleased APIs. Ownership transfer supports the installed 2.2 constructors.

Tests in `tests/test_fitting.cpp` cover constrained recovery, all-fixed blocks,
pointwise robust loss, invalid registration/settings and strided Eigen adapter
inputs/outputs. `tests/fitting_consumer` compiles and runs the installed new API.
