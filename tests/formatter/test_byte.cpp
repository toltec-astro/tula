#include <gtest/gtest.h>

#include "../common.h"
#include <tula/formatter/byte.h>

namespace {

using namespace tula::testing;

TEST(test_formatter, byte) {
    std::byte b{42};
    EXPECT_NO_THROW(fmtlog("b={}", b));
    EXPECT_NO_THROW(fmtlog("b={:x}", b));
}

} // namespace