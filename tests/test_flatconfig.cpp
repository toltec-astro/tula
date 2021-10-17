#include <gtest/gtest.h>

#include "test_common.h"
#include <tula/config/flatconfig.h>
#include <tula/formatter/container.h>

namespace {

using namespace tula::testing;

TEST(flatconfig, get) {

    using tula::config::FlatConfig;
    constexpr auto undef = tula::config::FlatConfig::undef;

    auto config = FlatConfig{
        {"a", 1}, {"b", undef},
        {"c", true}, {"d", "abc"}
    };

    fmtlog("config: {}", config.pformat());

    EXPECT_TRUE(config.has("a"));
    EXPECT_TRUE(config.has("b"));
    EXPECT_FALSE(config.has("e"));
    EXPECT_TRUE(config.is_set("a"));
    EXPECT_FALSE(config.is_set("b"));
    EXPECT_EQ(config.get_typed<int>("a"), 1);
    EXPECT_EQ(config.get_typed<std::monostate>("b"), std::monostate{});
    EXPECT_TRUE(config.get_typed<bool>("c"));

    EXPECT_EQ(config.get_typed<std::optional<int>>("a").value(), 1);
    EXPECT_EQ(config.get_typed<std::optional<bool>>("b"), std::nullopt);
    EXPECT_EQ(config.get_typed<std::optional<bool>>("c"), std::optional<bool>(true));
    // upate
    config.get_typed<int>("a") = -1;
    config.set("b", "a string");
    config.set("c", config.at("d"));
    config.set("d", undef);
    fmtlog("config: {}", config.pformat());
}

} // namespace
