#pragma once

#include "fmt/core.h"
#include "formatter/utils.h"

#if defined(__GNUC__) && !defined(__clang__)
#if __GNUC__ < 10
#include <experimental/filesystem>
namespace std {
namespace filesystem = ::std::experimental::filesystem;
} // namespace std
#else
#include <filesystem>
#endif
#else
#include <filesystem>
#endif

namespace fmt {

template <>
struct formatter<std::filesystem::path>
    : tula::fmt_utils::nullspec_formatter_base {
    template <typename FormatContext>
    auto format(const std::filesystem::path &p, FormatContext &ctx) {
        auto it = ctx.out();
        return format_to(it, "{}", p.string());
    }
};

}  // namespace fmt
