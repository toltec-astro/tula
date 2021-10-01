#include <gtest/gtest.h>

#include "../common.h"
#include <tula/formatter/duration.h>

namespace {

using namespace tula::testing;

TEST(test_formatter, duration) {
    using namespace std::chrono;
    EXPECT_EQ(fmtlog("{}", days{3}), "3d");
    EXPECT_EQ(fmtlog("{}", days{366}), "366d");
    EXPECT_EQ(fmtlog("{}", milliseconds{100000}), "1m40s");
    EXPECT_EQ(fmtlog("{}", milliseconds{10000}), "10s");
    EXPECT_EQ(fmtlog("{}", milliseconds{1000}), "1s");
    EXPECT_EQ(fmtlog("{}", milliseconds{100}), "100ms");
    EXPECT_EQ(fmtlog("{}", nanoseconds{100111}), "100.111us");
    EXPECT_EQ(fmtlog("{}", nanoseconds{1111}), "1.111us");
    EXPECT_EQ(fmtlog("{}", nanoseconds{111}), "111ns");
}

} // namespace