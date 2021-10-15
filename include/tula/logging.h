#pragma once

#include "formatter/duration.h"
#include <array>
#include <fmt/format.h>
#include <fmt/ranges.h>

#ifdef SPDLOG_ACTIVE_LEVEL
#undef SPDLOG_ACTIVE_LEVEL
#endif
#ifdef LOGLEVEL
#if LOGLEVEL == Trace
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#elif LOGLEVEL == Debug
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#elif LOGLEVEL == Info
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#elif LOGLEVEL == Warning
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_WARN
#elif LOGLEVEL == Error
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_ERROR
#endif
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

/// @brief Logging utilities.

namespace tula::logging {
/// @brief The log level enum.
using level_enum = spdlog::level::level_enum;

/// @brief The list of log level names.
constexpr auto level_names = std::array SPDLOG_LEVEL_NAMES;

/// @brief The minimum log level activated at compile time.
constexpr auto active_level = static_cast<level_enum>(SPDLOG_ACTIVE_LEVEL);

/// @brief The list of log level names enabled at compile time.
constexpr auto active_level_names = []() {
    constexpr auto n = level_names.size() - SPDLOG_ACTIVE_LEVEL;
    std::array<std::string_view, n> names;
    for (std::size_t i = 0; i < n; ++i) {
        names[i] = level_names[i + SPDLOG_ACTIVE_LEVEL].data();
    }
    return names;
}();

/// @brief Return the level name for given level.
constexpr auto get_level_name = [](level_enum level) {
    return std::string_view(level_names[static_cast<int>(level)].data());
};

/**
 * @brief Initialize the default logger.
 * @param verbose When true, print additional info about the logger settings.
 */
inline void init(level_enum level, bool verbose = true) noexcept {
    if (verbose) {
        if (level < active_level) {
            fmt::print("** logging ** Log level {} is not enabled at compile "
                       "time. Use the minimum level {} instead.\n",
                       get_level_name(level), get_level_name(active_level));
            level = active_level;
        }
        fmt::print(
            "** logging ** Configured with level={}; available levels: {}.\n",
            get_level_name(level), active_level_names);
    }
    spdlog::set_level(level);
}

/// @brief Initialize the default logger with minimum log level enabled.
inline void init(bool verbose = true) { return init(active_level, verbose); }

/// @brief Return the current time.
inline auto now() { return std::chrono::high_resolution_clock::now(); }

/// @brief Return the time elapsed since given time.
inline auto elapsed_since(
    const std::chrono::time_point<std::chrono::high_resolution_clock> &t0) {
    return now() - t0;
}

/// @brief An RAII class to report the lifetime of itself.
struct scoped_timeit {
    scoped_timeit(std::string_view msg_, double *elapsed_msec_ = nullptr)
        : msg(msg_), elapsed_msec(elapsed_msec_) {
        SPDLOG_INFO("**timeit** {}", msg);
    }
    ~scoped_timeit() {
        auto t = elapsed_since(t0);
        constexpr auto s_to_ms = 1e3;
        if (this->elapsed_msec != nullptr) {
            *(this->elapsed_msec) =
                std::chrono::duration_cast<std::chrono::duration<double>>(t)
                    .count() *
                s_to_ms;
        }
        SPDLOG_INFO("**timeit** {} finished in {}", msg, t);
    }
    scoped_timeit(const scoped_timeit &) = delete;
    scoped_timeit(scoped_timeit &&) = delete;
    auto operator=(const scoped_timeit &) -> scoped_timeit & = delete;
    auto operator=(scoped_timeit &&) -> scoped_timeit & = delete;

    std::chrono::time_point<std::chrono::high_resolution_clock> t0{now()};
    std::string_view msg;
    double *elapsed_msec{nullptr};
};

/// @brief An RAII class to alter the default log level for its lifetime.
template <auto level_>
struct scoped_loglevel {
    scoped_loglevel() {
        level = spdlog::default_logger()->level();
        spdlog::set_level(level_);
    }
    ~scoped_loglevel() { spdlog::set_level(level); }
    scoped_loglevel(const scoped_loglevel &) = delete;
    scoped_loglevel(scoped_loglevel &&) = delete;
    auto operator=(const scoped_loglevel &) -> scoped_loglevel & = delete;
    auto operator=(scoped_loglevel &&) -> scoped_loglevel & = delete;

    spdlog::level::level_enum level;
};

} // namespace tula::logging
