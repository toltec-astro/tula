#include <gtest/gtest.h>

#include "../test_common.h"
#include <tula/formatter/ptr.h>

#include <memory>

namespace {

using namespace tula::testing;

// NOLINTNEXTLINE
TEST(formatter, ptr) {
    int a = 1;
    EXPECT_NO_THROW(fmtlog("a={}", a));
    EXPECT_NO_THROW(fmtlog("*a@{}", fmt::ptr(&a)));
    EXPECT_NO_THROW(fmtlog("*a@{:x}", tula::fmt_utils::ptr(&a)));
    EXPECT_NO_THROW(fmtlog("*a@{:y}", tula::fmt_utils::ptr(&a)));
    EXPECT_NO_THROW(fmtlog("*a@{:z}", tula::fmt_utils::ptr(&a)));

    auto a1 = std::make_shared<int>(2);
    EXPECT_NO_THROW(fmtlog("shared a={}", *a1));
    EXPECT_NO_THROW(fmtlog("shared *a@{}", fmt::ptr(a1)));
    EXPECT_NO_THROW(fmtlog("shared *a@{:x}", tula::fmt_utils::ptr(a1)));
    EXPECT_NO_THROW(fmtlog("shared *a@{:y}", tula::fmt_utils::ptr(a1)));
    EXPECT_NO_THROW(fmtlog("shared *a@{:z}", tula::fmt_utils::ptr(a1)));
}

} // namespace
