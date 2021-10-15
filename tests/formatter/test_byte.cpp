#include <gtest/gtest.h>

#include "../test_common.h"
#include <tula/formatter/byte.h>

namespace {

using namespace tula::testing;

// NOLINTNEXTLINE
TEST(formatter, byte) {
    std::byte b{42}; // NOLINT
    EXPECT_NO_THROW(fmtlog("b={}", b));
    EXPECT_NO_THROW(fmtlog("b={:x}", b));
}

} // namespace
