#pragma once

// Compatibility adapter. New models should use tula/ceres/fit.h and keep
// their equations independent of optimizer and storage policy.
#include <Eigen/Core>
#include <string_view>
#include <tula/ceres/fit.h>
#include <tuple>
#include <unordered_map>

namespace tula::alg::ceresfit {
using ceres::Problem;
using ceres::Solver;
using Eigen::Dynamic;
using Eigen::Index;
using tula::ceres_utils::ParamSetting;
template <class Scalar = double>
using ParamSettings =
    std::unordered_map<std::string_view, ParamSetting<Scalar>>;
enum class EvalMode { Residual, Model };

template <Index Params = Dynamic, Index Inputs = Dynamic,
          Index Outputs = Dynamic, class S = double>
struct Fitter {
  static_assert(Params > 0, "Ceres parameter count must be static");
  static_assert(std::is_same_v<S, double>, "Ceres parameters are double");
  static constexpr Index NP = Params, ND_IN = Inputs, ND_OUT = Outputs;
  using Scalar = S;
  using InputType = Eigen::Map<const Eigen::Matrix<Scalar, NP, 1>>;
};

namespace detail {
template <class Derived>
auto pack(const Eigen::DenseBase<Derived>& data) {
  std::vector<double> result;
  for (Index col = 0; col < data.cols(); ++col)
    for (Index row = 0; row < data.rows(); ++row) {
      const auto value = data.derived().coeff(row, col);
      if constexpr (requires {
                      value.real();
                      value.imag();
                    }) {
        result.push_back(value.real());
        result.push_back(value.imag());
      } else
        result.push_back(value);
    }
  return result;
}
} // namespace detail

template <class Model, class X, class Y, class E, class P>
auto fit(const X& x, const Y& y, const E& error, P&& parameters,
         const ParamSettings<typename Model::Scalar>& overrides = {},
         const Solver::Options& options = tula::ceres_utils::dense_options(50),
         tula::ceres_utils::LossOptions loss = {tula::ceres_utils::Loss::cauchy,
                                                2.}) {
  static_assert(!std::is_const_v<std::remove_reference_t<P>>,
                "fit requires mutable parameters");
  if (parameters.size() != Model::NP || parameters.innerStride() != 1 ||
      (parameters.rows() != 1 && parameters.cols() != 1))
    throw std::invalid_argument(
        "fit needs a contiguous parameter vector of the model's size");
  if constexpr (Model::ND_IN > 0 && Model::ND_OUT > 0)
    if (y.size() * Model::ND_IN != x.size() * Model::ND_OUT)
      throw std::invalid_argument("model input/output shape mismatch");
  auto xs = detail::pack(x), ys = detail::pack(y), errors = detail::pack(error);
  if (xs.empty() || ys.empty() || ys.size() != errors.size() ||
      ys.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    throw std::invalid_argument("invalid model data or uncertainty shape");
  for (double e : errors)
    if (std::isnan(e) || e <= 0)
      throw std::invalid_argument(
          "uncertainties must be positive (infinity disables a residual)");
  for (const auto& [name, setting] : overrides)
    if (std::find(Model::param_names.begin(), Model::param_names.end(), name) ==
        Model::param_names.end())
      throw std::invalid_argument("unknown model parameter: " +
                                  std::string(name));
  std::vector<ParamSetting<>> settings(Model::NP);
  for (Index i = 0; i < Model::NP; ++i) {
    const auto name = Model::param_names[i];
    if (auto it = overrides.find(name); it != overrides.end())
      settings[i] = it->second;
    else if (auto it = Model::param_settings.find(name);
             it != Model::param_settings.end())
      settings[i] = it->second;
  }
  Problem problem;
  tula::ceres_utils::configure_parameters(
      problem, {parameters.data(), Model::NP}, settings);
  auto model = std::make_unique<Model>();
  model->nx = xs.size();
  model->ny = ys.size();
  model->xdata = xs.data();
  model->ydata = ys.data();
  model->yerr = errors.data();
  auto cost = std::make_unique<
      ceres::AutoDiffCostFunction<Model, ceres::DYNAMIC, Model::NP>>(
      model.get(), static_cast<int>(ys.size()));
  model.release();
  // Whole-dataset robustification is not sample-wise outlier rejection.
  // The new helper supports losses on explicitly chosen residual groups.
  auto loss_function = tula::ceres_utils::make_loss(loss);
  problem.AddResidualBlock(cost.get(), loss_function.get(), parameters.data());
  loss_function.release();
  cost.release();
  auto summary = tula::ceres_utils::solve(problem, options);
  const bool converged = summary.termination_type == ceres::CONVERGENCE;
  return std::make_tuple(converged, std::move(summary));
}

template <class Model, class X, class P, class Y>
void eval(const X& x, const P& parameters, Y&& output) {
  static_assert(!std::is_const_v<std::remove_reference_t<Y>>,
                "eval requires mutable output");
  if (parameters.size() != Model::NP)
    throw std::invalid_argument("model parameter size mismatch");
  auto xs = detail::pack(x), ps = detail::pack(parameters);
  using Scalar = typename std::remove_reference_t<Y>::Scalar;
  constexpr bool complex = requires(Scalar v) {
    v.real();
    v.imag();
  };
  std::vector<double> ys(output.size() * (complex ? 2 : 1));
  if constexpr (Model::ND_IN > 0 && Model::ND_OUT > 0)
    if (output.size() * Model::ND_IN != x.size() * Model::ND_OUT)
      throw std::invalid_argument("model output shape mismatch");
  Model model;
  model.nx = xs.size();
  model.ny = ys.size();
  model.xdata = xs.data();
  model.eval(ps.data(), ys.data());
  Index n = 0;
  for (Index col = 0; col < output.cols(); ++col)
    for (Index row = 0; row < output.rows(); ++row, ++n) {
      if constexpr (complex)
        output.coeffRef(row, col) = Scalar{ys[2 * n], ys[2 * n + 1]};
      else
        output.coeffRef(row, col) = ys[n];
    }
}
} // namespace tula::alg::ceresfit
