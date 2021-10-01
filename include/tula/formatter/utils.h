#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <string>

#include "../concepts.h"

namespace tula::fmt_utils {

/// @brief Remove space in string.
template <tula::meta::String T> auto remove_space(T &&s_) {
    T s{std::forward<T>(s_)};
    s.erase(std::remove_if(s.begin(), s.end(),
                           [](const auto &c) { return std::isspace(c); }),
            s.end());
    return s;
}

/// @brief Convert unsigned integer type to string with given base.
template <uint8_t base, tula::meta::UnsignedIntegral T>
requires(base >= 2 && base <= 64) auto itoa(T n) noexcept {
    constexpr char digits[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+-";
    std::string a;
    while (n != 0) {
        a += digits[n % base];
        n /= base;
    }
    std::reverse(a.begin(), a.end());
    return a;
};

namespace internal {

/// @brief Parse single charactor format spec.
template <char default_char, char... chars> struct charspec {

    constexpr charspec() noexcept = default;

    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx) noexcept -> decltype(ctx.begin()) {
        auto it = ctx.begin(), end = ctx.end();
        if (it == end) {
            return it;
        }
        // only check the first
        bool valid = false;
        auto spec = *it++;
        for (auto c : m_charset) {
            if (spec == c) {
                valid = true;
                break;
            }
        }
        if (valid) {
            m_value = spec;
        }
        return it;
    }
    constexpr auto operator()() const noexcept { return m_value; }

private:
    char m_value{default_char};
    constexpr static std::array<char, sizeof...(chars) + 1> m_charset{chars...};
};

} // namespace internal

/// @brief Formatter base class that handles single char format spec.
template <char... charset> struct charspec_formatter_base {
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx) noexcept -> decltype(ctx.begin()) {
        return spec_handler.parse(ctx);
    }

protected:
    internal::charspec<charset...> spec_handler{};
};

/// @brief Formatter base class that ignores the format spec.
struct nullspec_formatter_base {
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx) noexcept -> decltype(ctx.begin()) {
        return ctx.begin();
    }
};

} // namespace tula::fmt_utils
