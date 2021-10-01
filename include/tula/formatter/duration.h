#pragma once

#include "utils.h"
#include <chrono>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

namespace fmt {

template <typename T>
requires tula::meta::is_instance<T, std::chrono::duration>::value

    struct formatter<T>
    : tula::fmt_utils::charspec_formatter_base<'a', 'H', 'M', 'S', 'm', 'u',
                                               'n'> {
    // a: human-readable (default)
    // H: hours
    // M: munites
    // S: seconds
    // m: milliseconds
    // u: microseconds
    // n: nanoseconds
    template <typename FormatContext>
    auto format(const T &t, FormatContext &ctx) -> decltype(ctx.out()) {
        auto it = ctx.out();
        auto spec = spec_handler();
        SPDLOG_TRACE("formatting format");
        switch (spec) {
        case 'a':
            return this->human_time(t, it);
        case 'H':
            return format_to(
                it, "{:g}h",
                std::chrono::duration_cast<
                    std::chrono::duration<double, std::ratio<3600>>>(t)
                    .count());
        case 'M':
            return format_to(
                it, "{:g}m",
                std::chrono::duration_cast<
                    std::chrono::duration<double, std::ratio<60>>>(t)
                    .count());
        case 'S':
            return format_to(
                it, "{:g}s",
                std::chrono::duration_cast<std::chrono::duration<double>>(t)
                    .count());
        case 'm':
            return format_to(it, "{:g}ms",
                             std::chrono::duration_cast<
                                 std::chrono::duration<double, std::milli>>(t)
                                 .count());
        case 'u':
            return format_to(it, "{:g}us",
                             std::chrono::duration_cast<
                                 std::chrono::duration<double, std::micro>>(t)
                                 .count());

        case 'n':
            return format_to(
                it, "{:d}ns",
                std::chrono::duration_cast<std::chrono::nanoseconds>(t)
                    .count());
        }
        return it;
    };

private:
    template <typename Iter>
    auto human_time(T value, Iter &it) -> decltype(auto) {
        using namespace std::chrono;
        auto ns = duration_cast<nanoseconds>(value);
        if (ns.count() == 0) {
            format_to(it, "0ns");
            return it;
        }
        bool found_non_zero = false; // marks leading figure
        auto fmt_value = []<typename U>(auto &it, const auto &v,
                                        const auto &found_non_zero) {
            if constexpr (std::is_same_v<U, years>) {
                format_to(it, "{:d}y", v);
            } else if constexpr (std::is_same_v<U, days>) {
                if (v > 0) {
                    format_to(it, "{:d}d", v);
                }
            }
            if constexpr (std::is_same_v<U, hours>) {
                if (found_non_zero) {
                    format_to(it, "{:02d}h", v);
                } else {
                    format_to(it, "{:d}h", v);
                }
            } else if constexpr (std::is_same_v<U, minutes>) {
                if (found_non_zero) {
                    format_to(it, "{:02d}m", v);
                } else {
                    format_to(it, "{:d}m", v);
                }
            } else {
                //
            }
        };
        auto fmt_part = [&fmt_value]<typename U>(auto &it, auto &ns,
                                                 auto &found_non_zero) {
            const auto v = duration_cast<U>(ns);

            if (v.count() || found_non_zero) {
                fmt_value.template operator()<U>(it, v.count(), found_non_zero);
                found_non_zero = true;
                ns -= v;
            }
        };
        // fmt_part.template operator()<years>(it, ns, found_non_zero);
        // if (found_non_zero && ns.count() == 0) {
        //     return it;
        // }
        fmt_part.template operator()<days>(it, ns, found_non_zero);
        if (found_non_zero && ns.count() == 0) {
            return it;
        }
        if (found_non_zero) {
            format_to(it, " "); // separation between yd and hms
        }
        fmt_part.template operator()<hours>(it, ns, found_non_zero);
        fmt_part.template operator()<minutes>(it, ns, found_non_zero);
        auto v = ns.count();
        // guess format
        if (v >= 1000000000) /* >= 1s */ {
            format_to(it, "{}s", v / 1e9);
        } else if (v >= 1000000) /* >=1ms */
        {
            format_to(it, "{}ms", v / 1e6);
        } else if (v >= 1000) /* >=1us */
        {
            format_to(it, "{}us", v / 1e3);
        } else {
            format_to(it, "{}ns", v);
        }
        return it;
    };
};

} // namespace fmt
