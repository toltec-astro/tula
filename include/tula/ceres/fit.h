#pragma once

#include <algorithm>
#include <ceres/ceres.h>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace tula::ceres_utils {

template <class Scalar = double>
struct ParamSetting {
  using OptScalar = std::optional<Scalar>;
  std::optional<Scalar> value, fixed, lower_bound, upper_bound;
  static ParamSetting getfixed(Scalar value) { return {.fixed = value}; }
  static ParamSetting getbounded(std::optional<Scalar> lower,
                                 std::optional<Scalar> upper) {
    return {.lower_bound = lower, .upper_bound = upper};
  }
};
using Parameter = ParamSetting<double>;
enum class Loss { linear, huber, cauchy };
struct LossOptions {
  Loss kind{Loss::linear};
  double scale{1}; // in the caller's whitened residual units
};

inline std::unique_ptr<ceres::LossFunction>
    make_loss(LossOptions options = {}) {
  if (!std::isfinite(options.scale) || options.scale <= 0)
    throw std::invalid_argument("loss scale must be positive and finite");
  switch (options.kind) {
  case Loss::linear:
    return {};
  case Loss::huber:
    return std::make_unique<ceres::HuberLoss>(options.scale);
  case Loss::cauchy:
    return std::make_unique<ceres::CauchyLoss>(options.scale);
  }
  throw std::invalid_argument("unknown robust loss");
}

// Validate before calling Ceres APIs that enforce preconditions with CHECK.
// Register each block once. The caller owns stable, mutable parameter storage
// for the problem's lifetime; Problem must use its default ownership options.
inline void configure_parameters(ceres::Problem& problem,
                                 std::span<double> values,
                                 std::span<const Parameter> settings = {}) {
  if (values.empty() ||
      values.size() >
          static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
      (!settings.empty() && settings.size() != values.size()))
    throw std::invalid_argument("invalid Ceres parameter block/settings shape");
  if (problem.HasParameterBlock(values.data()))
    throw std::invalid_argument("parameter block is already configured");
  std::vector<double> initial(values.begin(), values.end());
  std::vector<int> fixed;
  for (std::size_t i = 0; i < values.size(); ++i) {
    const auto s = settings.empty() ? Parameter{} : settings[i];
    for (auto x : {s.value, s.fixed, s.lower_bound, s.upper_bound})
      if (x && !std::isfinite(*x))
        throw std::invalid_argument("nonfinite parameter setting");
    if (s.value && s.fixed && *s.value != *s.fixed)
      throw std::invalid_argument("conflicting initial and fixed values");
    if (s.lower_bound && s.upper_bound && *s.lower_bound >= *s.upper_bound)
      throw std::invalid_argument(
          "parameter bounds must be strictly ordered; use fixed for constants");
    initial[i] = s.fixed.value_or(s.value.value_or(initial[i]));
    if (!std::isfinite(initial[i]) ||
        (s.lower_bound && initial[i] < *s.lower_bound) ||
        (s.upper_bound && initial[i] > *s.upper_bound))
      throw std::invalid_argument(
          "initial parameter is nonfinite or outside bounds");
    if (s.fixed) fixed.push_back(static_cast<int>(i));
  }
  const auto n = static_cast<int>(values.size());
  std::copy(initial.begin(), initial.end(), values.begin());
  problem.AddParameterBlock(values.data(), n);
  for (int i = 0; i < n && !settings.empty(); ++i) {
    if (settings[i].lower_bound)
      problem.SetParameterLowerBound(values.data(), i,
                                     *settings[i].lower_bound);
    if (settings[i].upper_bound)
      problem.SetParameterUpperBound(values.data(), i,
                                     *settings[i].upper_bound);
  }
  if (fixed.size() == values.size())
    problem.SetParameterBlockConstant(values.data());
  else if (!fixed.empty())
    problem.SetManifold(values.data(), new ceres::SubsetManifold(n, fixed));
}

// One call per explicitly chosen residual group. A loss acts on that group:
// e.g. one complex sample, or a whole dataset when reproducing a legacy fit.
template <int Residuals, int Parameters, class Functor>
ceres::ResidualBlockId add_autodiff(ceres::Problem& problem,
                                    std::span<double> parameters,
                                    Functor functor, LossOptions loss = {},
                                    int residual_count = Residuals) {
  static_assert((Residuals > 0 || Residuals == ceres::DYNAMIC) &&
                Parameters > 0);
  if (residual_count <= 0 ||
      (Residuals != ceres::DYNAMIC && residual_count != Residuals))
    throw std::invalid_argument("autodiff residual count mismatch");
  if (parameters.size() != Parameters)
    throw std::invalid_argument("autodiff parameter block size mismatch");
  if (problem.HasParameterBlock(parameters.data()) &&
      problem.ParameterBlockSize(parameters.data()) != Parameters)
    throw std::invalid_argument(
        "autodiff conflicts with the registered parameter block size");
  auto owned_loss = make_loss(loss);
  auto owned_functor = std::make_unique<Functor>(std::move(functor));
  using Cost = ceres::AutoDiffCostFunction<Functor, Residuals, Parameters>;
  auto cost = [&] {
    if constexpr (Residuals == ceres::DYNAMIC)
      return std::make_unique<Cost>(owned_functor.get(), residual_count);
    else
      return std::make_unique<Cost>(owned_functor.get());
  }();
  owned_functor
      .release(); // AutoDiffCostFunction owns the functor in Ceres 2.2+.
  auto id =
      problem.AddResidualBlock(cost.get(), owned_loss.get(), parameters.data());
  cost.release();
  owned_loss
      .release(); // Problem owns cost/loss/manifold with its default options.
  return id;
}

inline ceres::Solver::Options dense_options(int max_iterations = 100) {
  if (max_iterations <= 0)
    throw std::invalid_argument("max iterations must be positive");
  ceres::Solver::Options options;
  options.linear_solver_type = ceres::DENSE_QR;
  options.num_threads =
      1; // callers parallelize independent problems explicitly
  options.max_num_iterations = max_iterations;
  options.logging_type = ceres::SILENT;
  options.minimizer_progress_to_stdout = false;
  return options;
}

inline ceres::Solver::Summary
    solve(ceres::Problem& problem,
          const ceres::Solver::Options& options = dense_options()) {
  std::string error;
  if (!options.IsValid(&error)) throw std::invalid_argument(error);
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  return summary; // keep termination, usability, cost, iterations and timings
}
} // namespace tula::ceres_utils
