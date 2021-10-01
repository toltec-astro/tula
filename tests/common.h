#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>
#include <tula/logging.h>

namespace tula::testing {

inline auto logger() {
    constexpr auto name = "tula::testing";
    auto logger_ = spdlog::get(name);
    if (logger_) {
        return logger_;
    }
    return spdlog::stdout_color_mt(name);
}

template <typename FmtStr, typename... Args>
std::string fmtlog(FmtStr &&fmt_str, Args &&...args) {
    auto s = fmt::format(fmt::runtime(fmt_str), args...);
    logger()->log(tula::logging::active_level, s);
    return s;
};

} // namespace tula::testing
