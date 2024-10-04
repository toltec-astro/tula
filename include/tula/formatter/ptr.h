#pragma once
#include "utils.h"
#include <fmt/core.h>

namespace tula::fmt_utils {

/// @brief Convert pointer to the memory address
template <typename T>
struct ptr {
    using value_t = std::uintptr_t;
    ptr(const T *p) noexcept : m_value(reinterpret_cast<value_t>(p)) {}
    ptr(std::shared_ptr<T> p) noexcept
        : m_value(reinterpret_cast<value_t>(p.get())) {}
    auto value() const noexcept -> const auto & { return m_value; }

private:
    const value_t m_value;
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
            return fmt::format_to(it, "{:x}", ptr.value());
        case 'y': {
            constexpr auto base = 32;
            return fmt::format_to(it, "{}",
                             tula::fmt_utils::itoa<base>(ptr.value()));
        }
        case 'z': {
            constexpr auto base = 62;
            return fmt::format_to(it, "{}",
                             tula::fmt_utils::itoa<base>(ptr.value()));
        }
        }
        return it;
    }
};

} // namespace fmt
