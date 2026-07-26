#include <tula/config/core.h>
#include <tula/config/yamlconfig.h>

#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace {

template <typename Derived>
using ConfigValidator =
    tula::config::ConfigValidatorMixin<Derived, tula::config::YamlConfig>;

struct Dataset : ConfigValidator<Dataset> {
    using Base = ConfigValidator<Dataset>;

    explicit Dataset(config_t config, bool check = true)
        : Base{std::move(config), check},
          m_path(this->config().get_str("path"))
    {
    }

    static auto check_config(config_t &config) -> std::optional<std::string>
    {
        return config.has_typed<std::string>("path")
                   ? std::nullopt
                   : std::optional<std::string>{"missing string path"};
    }

    [[nodiscard]] auto path() const -> const std::string & { return m_path; }

  private:
    std::string m_path;
};

} // namespace

int main()
{
    using tula::config::YamlConfig;

    auto config = YamlConfig::from_str(R"(
runtime:
  workers: 8
inputs:
  - name: toltec0
    path: data/toltec0.nc
enabled: true
)");
    if (!config.has("inputs") || !config.has(std::tuple{"inputs", 0}) ||
        config.has(std::tuple{"inputs", "bad"})) {
        return 1;
    }
    if (config.get_typed<int>(std::tuple{"runtime", "workers"}) != 8 ||
        config.get_str(std::tuple{"inputs", 0, "name"}) != "toltec0") {
        return 2;
    }

    const auto dataset =
        Dataset::from_config(config.get_config(std::tuple{"inputs", 0}));
    if (dataset.path() != "data/toltec0.nc") {
        return 3;
    }

    const auto merged = merge(
        config,
        YamlConfig::from_str("runtime: {workers: 4}\nextra: present\n"));
    if (merged.get_typed<int>(std::tuple{"runtime", "workers"}) != 4 ||
        merged.get_str("extra") != "present") {
        return 4;
    }

    try {
        static_cast<void>(Dataset::from_config(
            YamlConfig::from_str("other: 42\n")));
        return 5;
    } catch (const std::runtime_error &) {
    }
    return fmt::format("{}", merged).find("workers") == std::string::npos ? 6
                                                                          : 0;
}
