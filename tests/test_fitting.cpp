#include <array>
#include <complex>
#include <iostream>
#include <tula/algorithm/ei_ceresfitter.h>

namespace cu = tula::ceres_utils;
void require(bool b) {
  if (!b) throw std::runtime_error("fitting assertion failed");
}
template <class F>
void rejects(F f) {
  try {
    f();
  } catch (const std::invalid_argument&) {
    return;
  }
  throw std::runtime_error("invalid input accepted");
}
struct Line {
  double x, y;
  template <class T>
  bool operator()(const T* p, T* r) const {
    r[0] = p[0] * T(x) + p[1] - T(y);
    return true;
  }
};
struct LineBatch {
  template <class T>
  bool operator()(const T* p, T* r) const {
    for (int k = 0; k < 8; ++k)
      r[k] = p[0] * T(k) + p[1] - T(2 * k + 4);
    return true;
  }
};
struct LegacyLine : tula::alg::ceresfit::Fitter<2, 1, 1> {
  static constexpr std::array<std::string_view, 2> param_names{"slope",
                                                               "offset"};
  inline static tula::alg::ceresfit::ParamSettings<> param_settings{};
  Eigen::Index nx{}, ny{};
  const double *xdata{}, *ydata{}, *yerr{};
  template <class T>
  void eval(const T* p, T* y) const {
    for (Eigen::Index i = 0; i < nx; ++i)
      y[i] = p[0] * T(xdata[i]) + p[1];
  }
  template <class T>
  bool operator()(const T* p, T* r) const {
    eval(p, r);
    for (Eigen::Index i = 0; i < nx; ++i)
      r[i] = (r[i] - T(ydata[i])) / T(yerr[i]);
    return true;
  }
};
struct LegacyComplex : tula::alg::ceresfit::Fitter<1, 1, 1> {
  static constexpr std::array<std::string_view, 1> param_names{"scale"};
  inline static tula::alg::ceresfit::ParamSettings<> param_settings{};
  Eigen::Index nx{}, ny{};
  const double *xdata{}, *ydata{}, *yerr{};
  template <class T>
  void eval(const T* p, T* r) const {
    for (Eigen::Index n = 0; n < nx; ++n) {
      r[2 * n] = p[0] * T(xdata[n]);
      r[2 * n + 1] = T(2) * p[0] * T(xdata[n]);
    }
  }
  template <class T>
  bool operator()(const T* p, T* r) const {
    eval(p, r);
    for (Eigen::Index n = 0; n < ny; ++n)
      r[n] = (r[n] - T(ydata[n])) / T(yerr[n]);
    return true;
  }
};
int main() try {
  std::array<double, 2> p{0, 0};
  std::array<cu::Parameter, 2> settings{cu::Parameter::getbounded(-3, 3),
                                        cu::Parameter::getfixed(4)};
  ceres::Problem problem;
  cu::configure_parameters(problem, p, settings);
  for (int x = 0; x < 8; ++x)
    cu::add_autodiff<1, 2>(problem, p, Line{double(x), 2. * x + 4});
  const auto summary = cu::solve(problem);
  require(summary.IsSolutionUsable() &&
          summary.termination_type == ceres::CONVERGENCE);
  require(std::abs(p[0] - 2) < 1e-7 && p[1] == 4);
  std::array<double, 2> batch_values{0, 0};
  ceres::Problem batch;
  cu::configure_parameters(batch, batch_values);
  cu::add_autodiff<ceres::DYNAMIC, 2>(batch, batch_values, LineBatch{}, {}, 8);
  require(cu::solve(batch).IsSolutionUsable());
  require(std::abs(batch_values[0] - 2) < 1e-7 &&
          std::abs(batch_values[1] - 4) < 1e-7);
  rejects([&] {
    cu::add_autodiff<ceres::DYNAMIC, 2>(batch, batch_values, LineBatch{});
  });
  auto estimate = [](cu::LossOptions loss) {
    std::array<double, 2> p{0, 0};
    ceres::Problem problem;
    cu::configure_parameters(problem, p);
    for (int x = 0; x < 20; ++x)
      cu::add_autodiff<1, 2>(
          problem, p, Line{double(x), 2. * x + 4 + (x == 10 ? 100 : 0)}, loss);
    require(cu::solve(problem).IsSolutionUsable());
    return std::abs(p[1] - 4);
  };
  require(estimate({cu::Loss::huber, 0.1}) < 0.02);
  require(estimate({}) > 1);
  auto invalid = settings;
  invalid[0] = {.value = 0, .fixed = 1};
  rejects([&] {
    ceres::Problem p0;
    cu::configure_parameters(p0, p, invalid);
  });
  invalid[0] = cu::Parameter::getbounded(1, -1);
  rejects([&] {
    ceres::Problem p0;
    cu::configure_parameters(p0, p, invalid);
  });
  rejects([&] { (void)cu::make_loss({cu::Loss::cauchy, 0}); });
  rejects([&] {
    cu::add_autodiff<1, 2>(problem, std::span(p).first(1), Line{0, 0});
  });
  rejects([&] { cu::configure_parameters(problem, p); });
  rejects([&] {
    ceres::Problem short_block;
    short_block.AddParameterBlock(p.data(), 1);
    cu::add_autodiff<1, 2>(short_block, p, Line{0, 0});
  });
  std::array<cu::Parameter, 2> fixed{cu::Parameter::getfixed(2),
                                     cu::Parameter::getfixed(4)};
  ceres::Problem constant;
  cu::configure_parameters(constant, p, fixed);
  cu::add_autodiff<1, 2>(constant, p, Line{1, 6});
  require(cu::solve(constant).IsSolutionUsable());
  // Noncontiguous Eigen data views, mutable rvalue parameter/output views.
  Eigen::Matrix<double, 2, 8> data;
  for (int x = 0; x < 8; ++x) {
    data(0, x) = x;
    data(1, x) = 2 * x + 4;
  }
  Eigen::Matrix2d params = Eigen::Matrix2d::Zero();
  auto [ok, fitted] = tula::alg::ceresfit::fit<LegacyLine>(
      data.row(0), data.row(1), Eigen::VectorXd::Ones(8), params.col(0));
  require(ok && std::abs(params(0, 0) - 2) < 1e-7 &&
          std::abs(params(1, 0) - 4) < 1e-7);
  // Compatibility defaults keep v1's whole-vector Cauchy(2), even though
  // the new helper defaults to linear loss. Positive infinity masks a point.
  Eigen::VectorXd masked_errors = Eigen::VectorXd::Ones(8);
  masked_errors[7] = std::numeric_limits<double>::infinity();
  const double previous = data(1, 7);
  data(1, 7) = 1e9;
  params.col(0).setZero();
  auto [masked_ok, masked] = tula::alg::ceresfit::fit<LegacyLine>(
      data.row(0), data.row(1), masked_errors, params.col(0));
  require(masked_ok && std::abs(params(0, 0) - 2) < 1e-7);
  double norm = 0;
  for (int n = 0; n < 7; ++n)
    norm += data(1, n) * data(1, n);
  require(std::abs(masked.initial_cost - 2 * std::log1p(norm / 4)) < 1e-12);
  data(1, 7) = previous;
  Eigen::Matrix<double, 2, 8> output;
  tula::alg::ceresfit::eval<LegacyLine>(data.row(0), params.col(0),
                                        output.row(1));
  require(output.row(1).isApprox(data.row(1), 1e-7));
  rejects([&] {
    (void)tula::alg::ceresfit::fit<LegacyLine>(data.row(0), data.row(1).head(3),
                                               Eigen::VectorXd::Ones(3),
                                               params.col(0));
  });
  Eigen::Matrix<std::complex<double>, 2, 8> complex_data, complex_output;
  for (int n = 0; n < 8; ++n)
    complex_data(1, n) = {3. * n, 6. * n};
  Eigen::VectorXd complex_parameter = Eigen::VectorXd::Zero(1);
  auto errors = Eigen::VectorXcd::Constant(8, {1, 1});
  auto [complex_ok, complex_summary] = tula::alg::ceresfit::fit<LegacyComplex>(
      data.row(0), complex_data.row(1), errors, complex_parameter);
  require(complex_ok && std::abs(complex_parameter[0] - 3) < 1e-6);
  tula::alg::ceresfit::eval<LegacyComplex>(data.row(0), complex_parameter,
                                           complex_output.row(1));
  require(complex_output.row(1).isApprox(complex_data.row(1), 1e-6));
  return 0;
} catch (const std::exception& e) {
  std::cerr << e.what() << '\n';
  return 1;
}
