#include <gtest/gtest.h>

#include "../common.h"
#include <tula/formatter/matrix.h>

namespace {

using namespace tula::testing;

TEST(test_formatter, matrix) {
    Eigen::MatrixXd m{5, 10};
    m.setConstant(std::nan(""));
    m.reshaped().setLinSpaced(m.size(), 0, m.size() - 1);

    EXPECT_NO_THROW(fmtlog("default m{}", m));
    EXPECT_NO_THROW(fmtlog("m{:r1c5}", m));
    EXPECT_NO_THROW(fmtlog("m{:r1c}", m));
    EXPECT_NO_THROW(fmtlog("m{:rc1}", m));

    auto c = m.col(0);
    EXPECT_NO_THROW(fmtlog("default c{}", c));
    EXPECT_NO_THROW(fmtlog("c{:}", c));
    EXPECT_NO_THROW(fmtlog("c{:rc}", c));
    EXPECT_NO_THROW(fmtlog("c{:s}", c));
    EXPECT_NO_THROW(fmtlog("c{:s3}", c));

    std::vector<double> v = {0, 1, 2, 3, 4, 5, 6, 7};
    EXPECT_NO_THROW(fmtlog("default v{:s4}", v));
    EXPECT_NO_THROW(fmtlog("v{:}", v));
    EXPECT_NO_THROW(fmtlog("v{:rc}", v));
    EXPECT_NO_THROW(fmtlog("v{:s}", v));
    EXPECT_NO_THROW(fmtlog("v{:s3}", v));

    std::array<double, 8> a = {9, 7, 5, 3, 1, -1, -3, -1};
    EXPECT_NO_THROW(fmtlog("default a{:s4}", a));
    EXPECT_NO_THROW(fmtlog("a{:}", a));
    EXPECT_NO_THROW(fmtlog("a{:rc}", a));
    EXPECT_NO_THROW(fmtlog("a{:s}", a));
    EXPECT_NO_THROW(fmtlog("a{:s3}", a));
}

} // namespace