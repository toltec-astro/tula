#pragma once

#include "utils.h"
#include <cstddef> // std::to_integer
#include <fmt/core.h>

namespace fmt {

template <>
struct formatter<std::byte>
    : tula::fmt_utils::charspec_formatter_base<'i', 'x'> {
    // x: base16, i.e., hex
    // i: int
    template <typename FormatContext>
    auto format(const std::byte &byte, FormatContext &ctx) const
        -> decltype(ctx.out()) {
        auto it = ctx.out();
        auto spec = spec_handler();
        auto value = std::to_integer<int>(byte);
        switch (spec) {
        case 'x':
            return fmt::format_to(it, "0x{:x}", value);
        case 'i':
            return fmt::format_to(it, "{}", value);
        }
        return it;
    }
};

} // namespace fmt
