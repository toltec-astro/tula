#include <array>
#include <cmath>
#include <iostream>
#include <span>
#include <stdexcept>
#include <tula/ceres/fit.h>

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
  return 0;
} catch (const std::exception& e) {
  std::cerr << e.what() << '\n';
  return 1;
}
