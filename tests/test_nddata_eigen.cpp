#include <tula/nddata/eigen.h>

#include <type_traits>

int main()
{
    using namespace tula::nddata;

    static_assert(std::is_same_v<type_traits<int>::index_t, std::size_t>);
    static_assert(
        std::is_same_v<type_traits<EigenData<Eigen::MatrixXd>>::index_t,
                       Eigen::Index>);

    Eigen::MatrixXd matrix{{0, 1}, {2, 3}, {4, 5}};
    auto owned = EigenData{matrix};
    if (owned.data != matrix || owned() != matrix) {
        return 1;
    }

    auto referenced = EigenDataRef(matrix);
    matrix.coeffRef(0, 0) = 9;
    if (referenced()(0, 0) != 9) {
        return 2;
    }

    auto block = EigenDataRef(owned().block(0, 0, 2, 2));
    owned().coeffRef(0, 0) = 8;
    return block()(0, 0) == 8 ? 0 : 3;
}
