#include <gtest/gtest.h>

#include "../test_common.h"
#include <tula/formatter/container.h>

#include <array>
#include <list>
#include <optional>
#include <queue>
#include <set>
#include <tuple>
#include <unordered_map>
#include <variant>

namespace {

using namespace tula::testing;

// NOLINTNEXTLINE
TEST(formatter, std_container) {
    EXPECT_NO_THROW(fmtlog("l{}", std::list{1, 2}));
    EXPECT_NO_THROW(fmtlog("p{}", std::pair{1, 2}));
    EXPECT_NO_THROW(fmtlog("a{}", std::array{1, 2}));
    EXPECT_NO_THROW(fmtlog("t{}", std::tuple{"a", true}));
    EXPECT_NO_THROW(fmtlog("q{}", std::deque{1, 2}));
    EXPECT_NO_THROW(fmtlog("s{}", std::set{"a", "b"}));
    EXPECT_NO_THROW(fmtlog("o={}", std::optional{"a"}));
    EXPECT_NO_THROW(fmtlog("nullopt={}", std::nullopt));
    EXPECT_NO_THROW(fmtlog("monostate={}", std::monostate{}));
    EXPECT_NO_THROW(fmtlog(
        "m{}", std::unordered_map<std::string, int>{{"a", 1}, {"b", 2}}));
    EXPECT_NO_THROW(
        fmtlog("m{}", std::unordered_map<std::string, std::optional<int>>{
                          {"a", 1}, {"b", std::nullopt}}));

    std::variant<std::monostate, std::string, int, double> v;
    std::vector<decltype(v)> vs{1, "2", 3.0, std::monostate{}};
    EXPECT_NO_THROW(fmtlog("vs={}", vs));
}

// NOLINTNEXTLINE
TEST(formatter, variant) {
    std::variant<bool, int, double, const char *, std::string> v;
    using namespace std::literals;
    EXPECT_EQ(fmtlog("v={}", v = true), "v=true (bool)");
    EXPECT_EQ(fmtlog("v={}", v = -1), "v=-1 (int)");
    EXPECT_EQ(fmtlog("v={}", v = 2.), "v=2 (doub)");
    EXPECT_EQ(fmtlog("v={}", v = "v"), "v=\"v\" (str)");
    EXPECT_EQ(fmtlog("v={}", v = "v"s), "v=\"v\" (str)");
}

} // namespace
