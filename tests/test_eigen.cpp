#include <gtest/gtest.h>

#include "test_common.h"
#include <tula/eigen.h>
#include <tula/formatter/matrix.h>
#include <tula/logging.h>

namespace {

using namespace tula::testing;

// NOLINTNEXTLINE
TEST(matrix, iter) {
    Eigen::MatrixXd m{5, 2};
    m.reshaped().setLinSpaced(10, 0, 9);
    EXPECT_NO_THROW(fmtlog("m{}", m));

    Eigen::MatrixXd n{5, 2};
    Eigen::Map<Eigen::VectorXd>(n.data(), n.size()).setLinSpaced(10, 0, 9);

    EXPECT_TRUE(m == n);

    auto begin = m.reshaped().begin();
    auto end = m.reshaped().end();
    EXPECT_EQ(*begin, 0);
    EXPECT_EQ(*(--end), 9);
    for (auto it = begin; it != end; ++it) {
        EXPECT_NO_THROW(fmtlog("*it={}", *it));
    }
}

// NOLINTNEXTLINE
TEST(matrix, traits) {
    using namespace tula::eigen_utils;

    Eigen::MatrixXd m{5, 2};
    m.reshaped().setLinSpaced(10, 0, 9);
    Eigen::Map<Eigen::MatrixXd> n(m.data(), 2, 2);

    static_assert(is_eigen_v<decltype(m)>);
    static_assert(is_dense_v<decltype(m)>);
    static_assert(is_plain_v<decltype(m)>);

    static_assert(is_eigen_v<decltype(n)>);
    static_assert(is_dense_v<decltype(n)>);
    static_assert(!is_plain_v<decltype(n)>);

    static_assert(is_vblock_v<decltype(m.head(2))>);

    using etm = type_traits<decltype(m)>;
    static_assert(std::is_same_v<etm::Derived, Eigen::MatrixXd>);
    static_assert(etm::order == Eigen::ColMajor);
    static_assert(std::is_same_v<etm::Vector, Eigen::VectorXd>);
    static_assert(std::is_same_v<etm::Matrix, Eigen::MatrixXd>);
    static_assert(std::is_same_v<etm::VectorMap, Eigen::Map<Eigen::VectorXd>>);
    static_assert(std::is_same_v<etm::MatrixMap, Eigen::Map<Eigen::MatrixXd>>);

    using etn = type_traits<decltype(n)>;
    static_assert(std::is_same_v<etn::Derived, Eigen::Map<Eigen::MatrixXd>>);
    static_assert(etn::order == Eigen::ColMajor);
    static_assert(std::is_same_v<etn::Vector, Eigen::VectorXd>);
    static_assert(std::is_same_v<etn::Matrix, Eigen::MatrixXd>);
    static_assert(std::is_same_v<etn::VectorMap, Eigen::Map<Eigen::VectorXd>>);
    static_assert(std::is_same_v<etn::MatrixMap, Eigen::Map<Eigen::MatrixXd>>);

    auto check_mat = [](auto &&m) {
        fmtlog("data{} size={} outerstride={} innerstride={} outersize={} "
               "innersize={}",
               m, m.size(), m.outerStride(), m.innerStride(), m.outerSize(),
               m.innerSize());
    };
    check_mat(m);
    EXPECT_TRUE(is_contiguous(m));

    check_mat(n);
    EXPECT_TRUE(is_contiguous(n));

    auto p = m.block(0, 0, 2, 2);
    check_mat(p);
    EXPECT_TRUE(!is_contiguous(p));
}

// NOLINTNEXTLINE
TEST(matrix, convert) {
    using namespace tula::eigen_utils;
    Eigen::MatrixXd m{5, 2};
    m.reshaped().setLinSpaced(10, 0, 9);
    Eigen::Map<Eigen::MatrixXd> n(m.data(), 2, 2);
    auto p = m.block(0, 0, 2, 2);
    EXPECT_EQ(to_stdvec(m),
              std::vector<double>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
    EXPECT_EQ(to_stdvec(n), std::vector<double>({0, 1, 2, 3}));
    EXPECT_EQ(to_stdvec(p), std::vector<double>({0, 1, 5, 6}));
    EXPECT_EQ(to_stdvec(m, Eigen::StorageOptions::RowMajor),
              std::vector<double>({0, 5, 1, 6, 2, 7, 3, 8, 4, 9}));
    EXPECT_EQ(to_stdvec(n, Eigen::StorageOptions::RowMajor),
              std::vector<double>({0, 2, 1, 3}));
    EXPECT_EQ(to_stdvec(p, Eigen::StorageOptions::RowMajor),
              std::vector<double>({0, 5, 1, 6}));

    auto va = std::vector<double>({0, 1, 2, 3});
    auto a = as_eigen(va);
    EXPECT_EQ(to_stdvec(a), va);
    va[0] = 1;
    EXPECT_TRUE(a == Eigen::VectorXd({{1, 1, 2, 3}}));
    a.coeffRef(2) = 5;
    EXPECT_EQ(va, std::vector<double>({1, 1, 5, 3}));
    // const vec
    EXPECT_EQ(as_eigen(std::vector<double>({0, 1, 2, 3})),
              Eigen::VectorXd({{0, 1, 2, 3}}));

    std::vector<std::pair<double, double>> b{{0, 1}, {2, 3}, {4, 5}};
    EXPECT_TRUE(to_eigen(b) == Eigen::MatrixXd({{0, 2, 4}, {1, 3, 5}}));

    Eigen::MatrixXd q{5, 10};
    q.reshaped().setLinSpaced(q.size(), 0, 98);
    fmtlog("q{}", q);
    auto v1 = to_stdvec(q);
    fmtlog("v1{}", v1);
    auto v2 = to_stdvec(q.topRightCorner(2, 2));
    fmtlog("v2{}", v2);
    auto v3 = to_stdvec(q.row(1).segment(1, 2));
    fmtlog("v3{}", v3);
    auto q1 = as_eigen(v3);
    fmtlog("q1{}", q1);
    fmtlog("m1*m1{}", q1.array().square());
    q1 = q1.array().square();
    fmtlog("v3^2{}", v3);
}
} // namespace
