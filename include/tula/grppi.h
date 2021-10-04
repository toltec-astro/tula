#pragma once

#include "enum.h"
#include "formatter/enum.h"
#include "logging.h"
#include "meta.h"
#include <grppi/dyn/dynamic_execution.h>
#include <grppi/grppi.h>
#include <numeric>

namespace tula::grppi_utils {

/**
 * @brief Utility for GRPPI.
 */

// clang-format off
// NOLINTNEXTLINE
TULA_BITFLAG(ExMode, int, 0xFFFF,
         seq      = 1 << 0,
         thr      = 1 << 1,
         omp      = 1 << 2,
         tbb      = 1 << 3,
         ff       = 1 << 4,
         par      = thr | omp | tbb | ff
         );
// clang-format on

namespace internal {

template <ExMode... modes>
struct modes_impl {
private:
    /// @brief All supported GRPPI execution modes.
    constexpr static auto m_supported = []() noexcept {
        using T = std::underlying_type_t<ExMode>;
        T m{0};
        using namespace grppi;
        if constexpr (is_supported<sequential_execution>()) {
            m |= static_cast<T>(ExMode::seq);
        }
        if constexpr (is_supported<parallel_execution_native>()) {
            m |= static_cast<T>(ExMode::thr);
        }
        if constexpr (is_supported<parallel_execution_omp>()) {
            m |= static_cast<T>(ExMode::omp);
        }
        if constexpr (is_supported<parallel_execution_tbb>()) {
            m |= static_cast<T>(ExMode::tbb);
        }
        if constexpr (is_supported<parallel_execution_ff>()) {
            m |= static_cast<T>(ExMode::ff);
        }
        return static_cast<ExMode>(m);
    }();
    template <ExMode mode>
    constexpr static auto get_supported_filter() noexcept {
        if constexpr (m_supported & mode) {
            return std::make_tuple(mode);
        } else {
            return std::make_tuple();
        }
    }
    constexpr static auto get_supported() noexcept {
        auto s = std::tuple_cat(get_supported_filter<modes>()...);
        static_assert(
            std::tuple_size<decltype(s)>::value > 0,
            "GRPPI_UTILTS NEED TO SPECIFY AT LEAST ONE SUPPORTED EXMODE");
        return s;
    }

public:
    constexpr static auto supported = tula::meta::t2a(get_supported());
};

} // namespace internal

/*
 * @brief A utility class to manage GRPPI execution modes.
 * @tparam modes The modes to make available, sorted from high priority to low.
 * If not set, a default order is used: {omp, thr, tbb, ff, seq}.
 */
template <ExMode... modes>
struct ExConfig {

private:
    using modes_impl = std::conditional_t<
        (sizeof...(modes) > 0), internal::modes_impl<modes...>,
        internal::modes_impl<ExMode::omp, ExMode::thr, ExMode::tbb, ExMode::ff,
                             ExMode::seq>>;

public:
    /// @brief The supported ex mode names.
    static auto mode_names_supported() noexcept {
        std::vector<std::string> supported_names;
        supported_names.reserve(modes_impl::supported.size());
        for (const auto &mode : modes_impl::supported) {
            supported_names.emplace_back(ExMode_meta::to_name(mode));
        }
        return supported_names;
    }

    /// @brief The enabled ex modes.
    static auto modes_enabled() noexcept {
        // gcc does not support reduce
        return std::accumulate(modes_impl::supported.begin(),
                               modes_impl::supported.end(),
                               bitmask::bitmask<ExMode>{}, std::bit_or<>{});
    }

    /// @brief Return the default excution mode from a set of modes.
    static auto default_mode(bitmask::bitmask<ExMode> modes_) {
        for (const auto &m : modes_impl::supported) {
            if (m & modes_) {
                return m;
            }
        }
        throw std::runtime_error(fmt::format(
            "No available GRPPI execution mode in given modeset {:s}", modes_));
    }

    /// @brief Return the default GRPPI execution mode from all supported modes.
    static auto default_mode() noexcept { return modes_impl::supported[0]; }

    /// @brief Return the default GRPPI execution mode name;
    template <typename... Args>
    static auto default_mode_name(Args... args) {
        return ExMode_meta::to_name(default_mode(args...));
    }

    /// @brief Return the GRPPI execution mode for given name.
    static auto get_mode(std::string_view name) {
        auto mode_meta = ExMode_meta::from_name(name);
        if (!mode_meta) {
            throw std::runtime_error(fmt::format(
                R"("{:s}" is not a valid GRPPI execution mode)", name));
        }
        auto mode = mode_meta.value().value;
        if (mode & modes_enabled()) {
            return mode;
        }
        throw std::runtime_error(fmt::format(
            R"(GRPPI execution mode "{:s}" is not supported.)", mode));
    }

    /// @brief Returns the GRPPI execution object of \p mode.
    /// Mode with higher prority is used if multiple modes are set.
    template <typename... Args>
    static auto dyn_ex(bitmask::bitmask<ExMode> modes_, Args &&...args)
        -> grppi::dynamic_execution {
        using namespace grppi;
        if (!(modes_enabled() & modes_)) {
            throw std::runtime_error(fmt::format(
                "No supported GRPPI execution mode found in {:s}", modes_));
        }
        auto m = default_mode(modes_);
        SPDLOG_TRACE("create dynamic execution {} for mode {:s}", m);
        switch (m) {
        case ExMode::seq: {
            return sequential_execution(std::forward<Args>(args)...);
        }
        case ExMode::thr: {
            return parallel_execution_native(std::forward<Args>(args)...);
        }
        case ExMode::omp: {
            return parallel_execution_omp(std::forward<Args>(args)...);
        }
        case ExMode::tbb: {
            return parallel_execution_tbb(std::forward<Args>(args)...);
        }
        case ExMode::ff: {
            return parallel_execution_ff(std::forward<Args>(args)...);
        }
        default:
            throw std::runtime_error(
                fmt::format("Unknown grppi execution mode {:s}", m));
        }
    }
    /// @brief Returns the GRPPI execution object of mode \p name.
    template <typename... Args>
    static auto dyn_ex(std::string_view name, Args &&...args)
        -> grppi::dynamic_execution {
        return dyn_ex(get_mode(name), std::forward<Args>(args)...);
    }
    /// @brief Returns the GRPPI execution object of default mode.
    static auto dyn_ex() -> grppi::dynamic_execution {
        return dyn_ex(default_mode());
    }
};

/// @brief The default ExConfig class with all supported modes enabled.
/// @see \ref ExConfig.
using ex_config = ExConfig<>;

/// @brief Return the GRPPI execution mode from \p name.
inline auto get_mode(std::string_view name) -> ExMode {
    return ex_config::get_mode(name);
}

/// @brief Return the default GRPPI executionm mode from a modeset or all
/// supported.
template <typename... Args>
auto default_mode(Args... args) {
    return ex_config::default_mode(std::forward<Args>(args)...);
}

/// @brief Return the name of the default GRPPI mode.
template <typename... Args>
auto default_mode_name(Args... args) {
    return ex_config::default_mode_name(std::forward<Args>(args)...);
}

/// @brief Return the GRPPI execution object of \p mode.
/// @see \ref ExConfig::dyn_ex
template <typename... Args>
auto dyn_ex(Args... args) {
    return ex_config::dyn_ex(std::forward<Args>(args)...);
}

} // namespace tula::grppi_utils
