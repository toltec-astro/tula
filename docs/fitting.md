# Fitting

The `fitting` component (`tula::fitting`, header `<tula/ceres/fit.h>`) sets up
and solves least-squares problems with Ceres 2.2 or later. You write the
model equations; the helpers register parameters, add residuals and run the
solver.

## Fit a model

```cpp
#include <tula/ceres/fit.h>
#include <array>

namespace cu = tula::ceres_utils;

struct Line {
    double x, y;
    template <class T> bool operator()(const T* p, T* r) const {
        r[0] = p[0] * T(x) + p[1] - T(y);
        return true;
    }
};

int main() {
    std::array<double, 2> p{0, 0};               // slope, offset
    std::array<cu::Parameter, 2> settings{
        cu::Parameter::getbounded(-3, 3),         // slope in [-3, 3]
        cu::Parameter::getfixed(4)};              // offset fixed at 4
    ceres::Problem problem;
    cu::configure_parameters(problem, p, settings);
    for (int x = 0; x < 8; ++x)
        cu::add_autodiff<1, 2>(problem, p, Line{double(x), 2. * x + 4});
    const auto summary = cu::solve(problem);
    return summary.IsSolutionUsable() ? 0 : 1;
}
```

Link `tula::fitting` and request `tula+fitting` in Spack.

## Parameters

`configure_parameters(problem, values, settings)` registers `values` as one
parameter block. `settings` is optional; when given, it has one `Parameter`
per value:

| Field | Meaning |
| --- | --- |
| `value` | initial value (otherwise the current content of `values`) |
| `fixed` | the parameter stays at this value |
| `lower_bound`, `upper_bound` | bounds; must be strictly ordered |

`Parameter::getfixed(v)` and `Parameter::getbounded(lo, hi)` build the common
cases. When all values are fixed, the block is constant.

Register a block once. Keep `values` alive and in place while the problem
uses it; the solution is written back into it.

## Residuals and loss

`add_autodiff<R, P>(problem, values, functor, loss = {}, residual_count = R)`
adds one residual group of `R` residuals over a block of `P` parameters, with
automatic differentiation. Choose the group deliberately: a loss acts on the
whole group, for example one complex sample (`R = 2`).

| Loss | `LossOptions` |
| --- | --- |
| none (default) | `{}` |
| Huber | `{cu::Loss::huber, scale}` |
| Cauchy | `{cu::Loss::cauchy, scale}` |

`scale` is in the units of your residuals (for example, after dividing by the
uncertainty). For a group whose size is known only at run time, use
`R = ceres::DYNAMIC` and pass `residual_count`.

## Solve

`solve(problem)` uses `dense_options()`: dense QR, one thread, at most 100
iterations, no progress output. Pass your own `ceres::Solver::Options` to
change them. To fit many independent problems, run them in parallel
yourself.

`solve` returns the full `ceres::Solver::Summary`. Check
`IsSolutionUsable()` and `termination_type` separately: a usable solution has
not necessarily converged.

## Errors

The helpers check their input before calling Ceres and raise
`std::invalid_argument` for:

- an empty block, or settings of a different length;
- a block that is already registered;
- non-finite settings, an initial value that differs from the fixed value,
  bounds that are not strictly ordered, or an initial value outside its
  bounds;
- a residual count or block size that does not match the template arguments;
- a loss scale that is not positive and finite;
- invalid solver options.
