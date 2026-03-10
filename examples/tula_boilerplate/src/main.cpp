/**
 * tula_example/src/main.cpp
 *
 * Minimum working example demonstrating tula v3 modules:
 *   - logging (spdlog + fmt)
 *   - Eigen3 (linear algebra)
 *   - Yaml (yaml-cpp configuration)
 *   - Enum (meta_enum + bitmask)
 *   - Grppi (parallel patterns)
 */

#include <tula/logging.h>
#include <tula/eigen.h>
#include <tula/formatter/matrix.h>
#include <tula/config/yamlconfig.h>
#include <tula/enum.h>
#include <tula/formatter/enum.h>
#include <tula/grppi.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <vector>
#include <numeric>

// Enum definitions (must be at namespace scope)
TULA_ENUM(Color, int, Red, Green, Blue);
TULA_ENUM(Permission, int, Read = 1 << 0, Write = 1 << 1, Execute = 1 << 2);
TULA_BITFLAG_MAX_ELEMENT(Permission, Execute);

int main() {
    tula::logging::init();

    SPDLOG_INFO("=== tula_example ===");

    // 1. Eigen3
    {
        SPDLOG_INFO("--- Eigen3 ---");
        Eigen::MatrixXd m(3, 3);
        m << 1, 2, 3, 4, 5, 6, 7, 8, 9;
        SPDLOG_INFO("matrix:\n{}", m);
        Eigen::VectorXd v(3);
        v << 1, 0, 0;
        Eigen::VectorXd result = m * v;
        SPDLOG_INFO("m * [1,0,0]^T = {}", result.transpose());
    }

    // 2. YAML
    {
        SPDLOG_INFO("--- Yaml ---");
        constexpr auto yaml_str = R"(
name: tula_example
settings:
  threshold: 0.5
)";
        using namespace tula::config;
        auto config = YamlConfig(YAML::Load(yaml_str));
        auto name = config.get_str(std::tuple{"name"});
        auto threshold = config.get_typed<double>(std::tuple{"settings", "threshold"});
        SPDLOG_INFO("name={}, threshold={}", name, threshold);
    }

    // 3. Enum
    {
        SPDLOG_INFO("--- Enum ---");
        auto c = Color::Green;
        SPDLOG_INFO("color={:s}  long={:l}", c, c);
        SPDLOG_INFO("Color members: {}", Color_meta::members);
    }

    // 4. Bitmask
    {
        SPDLOG_INFO("--- Bitmask ---");
        auto perms = bitmask::bitmask<Permission>{Permission::Read | Permission::Write};
        SPDLOG_INFO("permissions={}", perms);
        SPDLOG_INFO("permissions bits={:d}", perms);
    }

    // 5. Grppi
    {
        SPDLOG_INFO("--- Grppi ---");
        using namespace tula::grppi_utils;
        SPDLOG_INFO("ExMode members: {}", ExMode_meta::members);
        auto ex = dyn_ex(ExMode::seq);  // get sequential execution object
        std::vector<int> input = {1, 2, 3, 4, 5};
        std::vector<int> squares(input.size());
        grppi::map(ex, input.begin(), input.end(), squares.begin(),
                   [](int x) { return x * x; });
        SPDLOG_INFO("input={}  squares={}", input, squares);
    }

    SPDLOG_INFO("=== done ===");
    return 0;
}
