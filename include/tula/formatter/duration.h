#pragma once

#include "utils.h"
#include <chrono>
#include <fmt/core.h>

namespace fmt {

// template <typename T>
// requires tula::meta::is_instance<T, std::chrono::duration>::value

//    struct formatter<T>
template <typename Rep, typename Period>
struct formatter<std::chrono::duration<Rep, Period>, char>

    : tula::fmt_utils::charspec_formatter_base<'a', 'H', 'M', 'S', 'm', 'u',
                                               'n'> {
    // a: human-readable (default)
    // H: hours
    // M: munites
    // S: seconds
    // m: milliseconds
    // u: microseconds
    // n: nanoseconds
    using T = std::chrono::duration<Rep, Period>;
    template <typename FormatContext>
    auto format(const T &t, FormatContext &ctx) const -> decltype(ctx.out()) {
        auto it = ctx.out();
        auto spec = spec_handler();
        constexpr auto h_to_s = 3600;
        constexpr auto m_to_s = 60;
        switch (spec) {
        case 'a':
            return this->human_time(t, it);
        case 'H':
            return fmt::format_to(
                it, "{:g}h",
                std::chrono::duration_cast<
                    std::chrono::duration<double, std::ratio<h_to_s>>>(t)
                    .count());
        case 'M':
            return fmt::format_to(
                it, "{:g}m",
                std::chrono::duration_cast<
                    std::chrono::duration<double, std::ratio<m_to_s>>>(t)
                    .count());
        case 'S':
            return fmt::format_to(
                it, "{:g}s",
                std::chrono::duration_cast<std::chrono::duration<double>>(t)
                    .count());
        case 'm':
            return fmt::format_to(it, "{:g}ms",
                             std::chrono::duration_cast<
                                 std::chrono::duration<double, std::milli>>(t)
                                 .count());
        case 'u':
            return fmt::format_to(it, "{:g}us",
                             std::chrono::duration_cast<
                                 std::chrono::duration<double, std::micro>>(t)
                                 .count());

        case 'n':
            return fmt::format_to(
                it, "{:d}ns",
                std::chrono::duration_cast<std::chrono::nanoseconds>(t)
                    .count());
        }
        return it;
    };

private:
    template <typename Iter>
    auto human_time(T value, Iter &it) const -> decltype(auto) {
        using namespace std::chrono;
        auto ns = duration_cast<nanoseconds>(value);
        if (ns.count() == 0) {
            fmt::format_to(it, "0ns");
            return it;
        }
        bool found_non_zero = false; // marks leading figure
        auto fmt_value = []<typename U>(auto &it, const auto &v,
                                        const auto &found_non_zero) {
            if constexpr (std::is_same_v<U, years>) {
                fmt::format_to(it, "{:d}y", v);
            } else if constexpr (std::is_same_v<U, days>) {
                if (v > 0) {
                    fmt::format_to(it, "{:d}d", v);
                }
            }
            if constexpr (std::is_same_v<U, hours>) {
                if (found_non_zero) {
                    fmt::format_to(it, "{:02d}h", v);
                } else {
                    fmt::format_to(it, "{:d}h", v);
                }
            } else if constexpr (std::is_same_v<U, minutes>) {
                if (found_non_zero) {
                    fmt::format_to(it, "{:02d}m", v);
                } else {
                    fmt::format_to(it, "{:d}m", v);
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
            fmt::format_to(it, " "); // separation between yd and hms
        }
        fmt_part.template operator()<hours>(it, ns, found_non_zero);
        fmt_part.template operator()<minutes>(it, ns, found_non_zero);
        auto v = ns.count();
        // guess format
        constexpr auto s_to_ns = 1000000000;
        constexpr auto ms_to_ns = 1000000;
        constexpr auto us_to_ns = 1000;
        if (v >= s_to_ns) /* >= 1s */ {
            fmt::format_to(it, "{}s", double(v) / s_to_ns);
        } else if (v >= ms_to_ns) /* >=1ms */
        {
            fmt::format_to(it, "{}ms", double(v) / ms_to_ns);
        } else if (v >= us_to_ns) /* >=1us */
        {
            fmt::format_to(it, "{}us", double(v) / us_to_ns);
        } else {
            fmt::format_to(it, "{}ns", v);
        }
        return it;
    };
};

} // namespace fmt
