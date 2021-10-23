#pragma once

#include "formatter/duration.h"
#include "meta.h"
#include <array>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/ostream.h>
#include <sstream>

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

template <typename Func>
class progressbar {
    static const auto overhead = sizeof " [100%]";

    Func func;
    const std::size_t width;
    const double scale{100};
    std::string message;
    const std::string bar;
    std::atomic<int> counter{0};

    auto barstr(double perc) {
        // clamp prog to valid range [0,1]
        if (perc < 0) {
            perc = 0;
        } else if (perc > 1) {
            perc = 1;
        }
        std::stringstream ss;
        auto barwidth = width - message.size();
        auto offset = width - std::size_t(double(barwidth) * perc);
        ss << message;
        ss.write(bar.data() + offset,
                 tula::meta::size_cast<std::streamsize>(barwidth));
        ss << fmt::format("[{:3.0f}%]", scale * perc);

        return ss.str();
    }

public:
    progressbar(Func func_, std::size_t linewidth, std::string message_,
                const char symbol = '.')
        : func{std::move(func_)}, width{linewidth - overhead},
          message{std::move(message_)}, bar{std::string(width, symbol) +
                                            std::string(width, ' ')} {
        // write(0.0);
    }

    // not copyable or movable
    progressbar(const progressbar &) = delete;
    auto operator=(const progressbar &) -> progressbar & = delete;
    progressbar(progressbar &&) = delete;
    auto operator=(progressbar &&) -> progressbar & = delete;

    ~progressbar() { func(fmt::format("{}\n", barstr(1.0))); }

    auto write(double perc) { func(barstr(perc)); }
    template <typename N1, typename N2>
    auto count(N1 total, N2 stride) {
        if (stride < 1) {
            stride = 1;
        }
        ++counter;
        if (counter % stride == 0) {
            double perc = double(counter) / total;
            // prevent write 1.0 here because this is done by the destructor
            // NOLINTNEXTLINE(readability-magic-numbers)
            if (int(perc * 100) < 100) {
                write(perc);
            }
        }
    }
};

namespace internal {

template <typename Prefunc, typename Postfunc, typename... Pargs>
struct decorated_invoke {
    decorated_invoke(std::tuple<Pargs...> && /*unused*/, Prefunc &&pre_,
                     Postfunc &&post_)
        : pre(std::move(pre_)), post(std::move(post_)) {}
    Prefunc pre;
    Postfunc post;
    template <typename F, typename... Args>
    auto operator()(Pargs &&...pargs, F &&func, Args &&...args) const
        -> decltype(auto) {
        decltype(auto) d = pre(std::forward<decltype(pargs)>(pargs)...);
        if constexpr (std::is_void_v<std::invoke_result_t<F, Args...>>) {
            std::forward<F>(func)(std::forward<decltype(args)>(args)...);
            post(std::forward<decltype(d)>(d));
        } else {
            decltype(auto) ret =
                std::forward<F>(func)(std::forward<decltype(args)>(args)...);
            post(std::forward<decltype(d)>(d));
            return ret;
        }
    }
};

} // namespace internal

inline const auto timeit = internal::decorated_invoke(
    std::tuple<std::string_view>{},
    [](auto msg) {
        SPDLOG_INFO("**timeit** {}", msg);
        // get time before function invocation
        auto start = std::chrono::high_resolution_clock::now();
        return std::tuple{msg, start};
    },
    [](auto &&p) {
        const auto &[msg, start] = std::forward<decltype(p)>(p);
        // get time after function invocation
        const auto &stop = std::chrono::high_resolution_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::duration<double>>(stop -
                                                                      start);
        SPDLOG_INFO("**timeit** {} finished in {}", msg, elapsed);
    });

} // namespace tula::logging
