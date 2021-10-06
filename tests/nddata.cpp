#include <gtest/gtest.h>

#include "common.h"
#include <tula/formatter/matrix.h>
#include <tula/logging.h>
#include <tula/nddata/eigen.h>

namespace {

using namespace tula::testing;

TEST(nddata, impl_traits) {

    using namespace tula::nddata;
    static_assert(
        std::is_same_v<internal::impl_traits<int>::index_t, std::size_t>);
    static_assert(std::is_same_v<
                  internal::impl_traits<EigenData<Eigen::MatrixXd>>::index_t,
                  Eigen::Index>);
}

// NOLINTNEXTLINE
TEST(nddata, eigen_data) {
    using namespace tula::nddata;
    Eigen::MatrixXd m{{0, 1}, {2, 3}, {4, 5}};
    auto mm = EigenData{m};
    fmtlog("mm{}", mm());
    EXPECT_EQ(mm.data, m);
    EXPECT_EQ(mm(), m);

    auto nn = EigenDataRef(m);
    // static_assert(std::is_same_v<decltype(EigenDataRef(m))::ref_t, void>);
    fmtlog("nn{}", nn());
    EXPECT_EQ(nn.data, m);
    EXPECT_EQ(nn(), m);
    m.coeffRef(0, 0) = 9;
    fmtlog("nn{}", nn());
    EXPECT_EQ(nn()(0, 0), 9.);

    auto pp = EigenDataRef(mm().block(0, 0, 2, 2));
    // static_assert(std::is_same_v<decltype(EigenDataRef(m))::ref_t, void>);
    fmtlog("pp{}", pp());
    EXPECT_EQ(pp.data, mm().block(0, 0, 2, 2));
    EXPECT_EQ(pp(), mm().block(0, 0, 2, 2));
    mm().coeffRef(0, 0) = 8;
    fmtlog("pp{}", pp());
    EXPECT_EQ(pp()(0, 0), 8.);
}

} // namespace
