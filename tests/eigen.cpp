#include <gtest/gtest.h>

#include "common.h"
#include <tula/eigen.h>
#include <tula/formatter/matrix.h>
#include <tula/logging.h>

namespace {

using namespace tula::testing;

TEST(test_matrix, iter) {
    using tula::fmt_utils::pprint;
    Eigen::MatrixXd m{5, 2};
    m.reshaped().setLinSpaced(10, 0, 9);
    EXPECT_NO_THROW(fmtlog("m{}", pprint(m)));

    Eigen::MatrixXd n{5, 2};
    Eigen::Map<Eigen::VectorXd>(n.data(), n.size()).setLinSpaced(10, 0, 9);

    EXPECT_EQ(m, n);

    auto begin = m.reshaped().begin();
    auto end = m.reshaped().end();
    EXPECT_EQ(*begin, 0);
    EXPECT_EQ(*(--end), 9);
    for (auto it = begin; it != end; ++it) {
        EXPECT_NO_THROW(fmtlog("*it={}", *it));
    }
}
} // namespace