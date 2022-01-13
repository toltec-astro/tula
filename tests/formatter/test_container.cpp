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

namespace tula::testing {

// define a custom type with formatting

struct Item {

    // template <typename OStream>
    // friend auto operator<<(OStream &os, const Item &item) -> decltype(auto) {
    //     return os << fmt::format("Item");
    // }
};

} // namespace tula::testing

namespace fmt {

template <>
struct formatter<tula::testing::Item, char, void>
    : tula::fmt_utils::nullspec_formatter_base {
    template <typename FormatContext>
    auto format(const tula::testing::Item &item, FormatContext &ctx) noexcept
        -> decltype(ctx.out()) {
        return format_to(ctx.out(), "Item");
    }
};

} // namespace fmt

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

TEST(formatter, std_container_with_custom_item) {
    auto item1 = Item();
    auto item2 = Item();
    EXPECT_NO_THROW(fmtlog("l{}", std::list{item1, item2}));
    EXPECT_NO_THROW(fmtlog("p{}", std::pair{item1, item2}));
    EXPECT_NO_THROW(fmtlog("a{}", std::array{item1, item2}));
    EXPECT_NO_THROW(fmtlog("t{}", std::tuple{"a", true, item1}));
    EXPECT_NO_THROW(fmtlog("o={}", std::optional{item1}));
    EXPECT_NO_THROW(fmtlog("m{}", std::unordered_map<std::string, Item>{
                                      {"a", item1}, {"b", item2}}));
    EXPECT_NO_THROW(
        fmtlog("m{}", std::unordered_map<std::string, std::optional<Item>>{
                          {"a", item1}, {"b", std::nullopt}}));
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
