#include <tula/ceres/fit.h>
#include <tula/config.h>
#include <array>
#include <cmath>

struct Residual {
    template <class T> bool operator()(const T *p, T *r) const {
        r[0] = p[0] - T(3);
        return true;
    }
};
int main() {
    static_assert(TULA_HAS_FITTING == 1);
    std::array<double, 1> p{0};
    ceres::Problem problem;
    tula::ceres_utils::configure_parameters(problem, p);
    tula::ceres_utils::add_autodiff<1, 1>(problem, p, Residual{});
    auto summary = tula::ceres_utils::solve(problem);
    return summary.IsSolutionUsable() && std::abs(p[0] - 3) < 1e-6 ? 0 : 1;
}
