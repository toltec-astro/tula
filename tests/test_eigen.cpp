#include <tula/eigen.h>
#include <tula/formatter/matrix.h>

#include <fmt/format.h>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

int main()
{
    using namespace tula::eigen_utils;

    Eigen::MatrixXd matrix{5, 2};
    matrix.reshaped().setLinSpaced(10, 0, 9);
    static_assert(is_eigen_v<decltype(matrix)>);
    static_assert(is_dense_v<decltype(matrix)>);
    static_assert(is_plain_v<decltype(matrix)>);
    static_assert(type_traits<decltype(matrix)>::order == Eigen::ColMajor);

    if (!is_contiguous(matrix)) {
        return 1;
    }
    if (to_stdvec(matrix) !=
        std::vector<double>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9})) {
        return 2;
    }
    if (to_stdvec(matrix, Eigen::RowMajor) !=
        std::vector<double>({0, 5, 1, 6, 2, 7, 3, 8, 4, 9})) {
        return 3;
    }

    auto values = std::vector<double>({0, 1, 2, 3});
    auto mapped = as_eigen(values);
    mapped.coeffRef(2) = 5;
    if (values != std::vector<double>({0, 1, 5, 3})) {
        return 4;
    }

    const std::vector<std::pair<double, double>> pairs{
        {0, 1},
        {2, 3},
        {4, 5},
    };
    const Eigen::Matrix<double, 2, 3> expected{{0, 2, 4}, {1, 3, 5}};
    if (to_eigen(pairs) != expected) {
        return 5;
    }
    return fmt::format("{}", matrix).empty() ? 6 : 0;
}
