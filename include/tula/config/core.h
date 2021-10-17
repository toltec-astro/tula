#pragma once
#include "../formatter/container.h"
#include "../logging.h"
#include "../meta.h"

namespace tula::config {

/// @brief A config wrapper with validation.
template <typename Derived, typename Config>
struct ConfigValidatorMixin {
    using config_t = Config;

private:
    config_t m_config{};

public:
    ConfigValidatorMixin() = default;
    template <typename... Args>
    ConfigValidatorMixin(Args &&...args) {
        set_config(std::forward<decltype(args)>(args)...);
    }
    auto config() const noexcept -> const config_t & { return m_config; }

    static void _check_config_impl(config_t &config) {
        // note that the check config may choose to alter the config object
        SPDLOG_TRACE("config check ...");
        // if constexpr (derived_has_check_config::value) {
        if constexpr (requires { Derived::check_config;}) {
            if (auto opt_errors = Derived::check_config(config);
                opt_errors.has_value()) {
                throw std::runtime_error(
                    fmt::format("invalid config:\n{}\nerrors: {}", config,
                                opt_errors.value()));
            }
            SPDLOG_TRACE("config check passed");
        } else {
            SPDLOG_WARN("config check requested but no "
                        "check_config implemented");
        }
    }
    void set_config(config_t config, bool check = true) {
        if (check) {
            _check_config_impl(config);
        } else {
            SPDLOG_TRACE("config check skipped");
        }
        m_config = std::move(config);
    }
    template <typename... Args>
    static auto from_config(Args &&...args) {
        return Derived(std::forward<decltype(args)>(args)...);
    }
};

} // namespace tula::config
