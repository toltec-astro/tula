#include <gtest/gtest.h>

#include "../common.h"
#include <tula/formatter/matrix.h>

namespace {

using namespace tula::testing;

TEST(test_formatter, matrix) {
    Eigen::MatrixXd m{5, 10};
    m.setConstant(std::nan(""));
    m.reshaped().setLinSpaced(m.size(), 0, m.size() - 1);
    using tula::fmt_utils::pprint;
    EXPECT_NO_THROW(fmtlog("default m{}", pprint(m)));
    EXPECT_NO_THROW(fmtlog("m{:r1c5}", pprint(m)));
    EXPECT_NO_THROW(fmtlog("m{:r1c}", pprint(m)));
    EXPECT_NO_THROW(fmtlog("m{:rc1}", pprint(m)));

    auto c = m.col(0);
    EXPECT_NO_THROW(fmtlog("default c{}", pprint(c)));
    EXPECT_NO_THROW(fmtlog("c{:}", pprint(c)));
    EXPECT_NO_THROW(fmtlog("c{:rc}", pprint(c)));
    EXPECT_NO_THROW(fmtlog("c{:s}", pprint(c)));
    EXPECT_NO_THROW(fmtlog("c{:s3}", pprint(c)));

    std::vector<double> v = {0, 1, 2, 3, 4, 5, 6, 7};
    EXPECT_NO_THROW(fmtlog("default v{:s4}", pprint(v)));
    EXPECT_NO_THROW(fmtlog("v{:}", pprint(v)));
    EXPECT_NO_THROW(fmtlog("v{:rc}", pprint(v)));
    EXPECT_NO_THROW(fmtlog("v{:s}", pprint(v)));
    EXPECT_NO_THROW(fmtlog("v{:s3}", pprint(v)));
}

} // namespace