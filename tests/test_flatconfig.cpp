#include <tula/config/flatconfig.h>

#include <optional>
#include <stdexcept>
#include <string>

int main()
{
    using tula::config::FlatConfig;

    auto config = FlatConfig{
        {"count", 3},
        {"enabled", true},
        {"label", "detector"},
        {"unset", FlatConfig::undef},
    };
    if (!config.has("count") || config.has("missing") ||
        !config.is_set("enabled") || config.is_set("unset")) {
        return 1;
    }
    if (config.get_typed<int>("count") != 3 ||
        config.get_typed<std::string>("label") != "detector") {
        return 2;
    }
    if (config.get_typed<std::optional<int>>("unset").has_value()) {
        return 3;
    }

    config.get_typed<int>("count") = 4;
    config.set("label", "array");
    config.set("added", 1.5);
    if (config.get_lexical<std::string>("count") != "4" ||
        config.get_typed<std::string>("label") != "array" ||
        config.get_typed<double>("added") != 1.5) {
        return 4;
    }

    bool called = false;
    if (!config.try_call_if("enabled", [&called] { called = true; }) ||
        !called) {
        return 5;
    }
    return config.pformat().find("added") == std::string::npos ? 6 : 0;
}
