#pragma once
#include "utils.h"
#include <fmt/format.h>

namespace tula::fmt_utils {

/// @brief Convert pointer to the memory address
template <typename T> struct ptr {
    using value_t = std::uintptr_t;
    const value_t value;
    ptr(const T *p) noexcept : value(reinterpret_cast<value_t>(p)) {}
    ptr(std::shared_ptr<T> p) noexcept
        : value(reinterpret_cast<value_t>(p.get())) {}
};

} // namespace tula::fmt_utils

namespace fmt {

template <typename T>
struct formatter<tula::fmt_utils::ptr<T>>
    : tula::fmt_utils::charspec_formatter_base<'z', 'x', 'y'> {
    // x: base16, i.e., hex
    // y: base32
    // z: base62 (default)
    template <typename FormatContext>
    auto format(const tula::fmt_utils::ptr<T> &ptr, FormatContext &ctx)
        -> decltype(ctx.out()) {
        auto it = ctx.out();
        auto spec = spec_handler();
        switch (spec) {
        case 'x':
            return format_to(it, "{:x}", ptr.value);
        case 'y':
            return format_to(it, "{}", tula::fmt_utils::itoa<32>(ptr.value));
        case 'z':
            return format_to(it, "{}", tula::fmt_utils::itoa<62>(ptr.value));
        }
        return it;
    }
};

} // namespace fmt
