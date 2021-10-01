#include <gtest/gtest.h>

#include "../common.h"
#include <tula/formatter/ptr.h>

namespace {

using namespace tula::testing;

TEST(test_formatter, ptr) {
    int a = 1;
    EXPECT_NO_THROW(fmtlog("a={}", a));
    EXPECT_NO_THROW(fmtlog("*a@{}", fmt::ptr(&a)));
    EXPECT_NO_THROW(fmtlog("*a@{:x}", tula::fmt_utils::ptr(&a)));
    EXPECT_NO_THROW(fmtlog("*a@{:y}", tula::fmt_utils::ptr(&a)));
    EXPECT_NO_THROW(fmtlog("*a@{:z}", tula::fmt_utils::ptr(&a)));

    auto a1 = std::shared_ptr<int>(new int(2));
    EXPECT_NO_THROW(fmtlog("shared a={}", *a1));
    EXPECT_NO_THROW(fmtlog("shared *a@{}", fmt::ptr(a1)));
    EXPECT_NO_THROW(fmtlog("shared *a@{:x}", tula::fmt_utils::ptr(a1)));
    EXPECT_NO_THROW(fmtlog("shared *a@{:y}", tula::fmt_utils::ptr(a1)));
    EXPECT_NO_THROW(fmtlog("shared *a@{:z}", tula::fmt_utils::ptr(a1)));
}

} // namespace