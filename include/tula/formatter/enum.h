#pragma once
#include "../concepts.h"
#include "../enum.h"
#include "container.h"
#include <bit>
#include <fmt/format.h>
#include <to_string.hpp>

namespace tula::fmt_utils {

namespace internal {

// https://stackoverflow.com/a/65440575
template <unsigned... Len>
constexpr auto fmt_str_cat(const char (&...strings)[Len]) {
    constexpr unsigned N = (... + Len) - sizeof...(Len);
    std::array<char, N + 1> result{};
    result.c[N] = '\0';
    char *dst = result.c;
    for (const char *src : {strings...}) {
        for (; *src != '\0'; src++, dst++) {
            *dst = *src;
        }
    }
}

} // namespace internal

/*
/// @brief Format int type as bits with set width
template <typename FormatContextOut>
auto format_bits(FormatContextOut &it, const T &value, ) -> decltype(auto) {
    if (value > ((1 << width) - 1)) {
        return fmt::format_to(it, "b{:b}", value);
    }
    return fmt::format_to(it, fmt::runtime(fmt::format("b{{:{}b}}", width)),
                          value);
}
*/

/// @brief Format int type as bits with set width
template <tula::meta::Integral T, typename FormatContextOut>
auto format_bits(FormatContextOut &it, const T &value, std::size_t width = 0)
    -> decltype(auto) {
    if (value > ((1 << width) - 1)) {
        return fmt::format_to(it, "b{:b}", value);
    }
    return fmt::format_to(it, fmt::runtime(fmt::format("b{{:0{}b}}", width)),
                          value);
}

/// @brief Format enum type as bits with its used bit width
template <tula::meta::EnumClass T, typename FormatContextOut>
auto format_bits(FormatContextOut &it, const T &value) -> decltype(auto) {
    using UT = std::underlying_type_t<T>;
    return format_bits(it, static_cast<UT>(value), enum_utils::bitwidth_v<T>);
}

/// @brief Format enum value meta.
// d: the bits
// s: the name
// l: the name and value
template <typename T, typename FormatContextOut>
auto format_enum_value_meta(FormatContextOut &it, char spec, const T &meta)
    -> decltype(auto) {
    switch (spec) {
    case 'd': {
        format_bits(it, meta.value);
        return it;
    }
    case 's': {
        // nameonly
        fmt::format_to(it, "{}", meta.name);
        return it;
    }
    case 'l': {
        // format using the string
        auto str = fmt_utils::remove_space(std::string(meta.string));
        // remove up to = sign
        str.erase(str.begin(), std::find(str.begin(), str.end(), '='));
        // without explicit def
        if (str.empty()) {
            it = fmt::format_to(it, "{}(", meta.name);
            it = fmt_utils::format_bits(it, meta.value);
            fmt::format_to(it, ")");
            return it;
        }
        // with explicit def
        str.erase(str.begin());  // remove the = sign
        fmt::format_to(it, "{}({})", meta.name, str);
        return it;
    }
    default: {
        return it;
    }
    }
}

/// @brief Format enum value using the enum value meta.
template <typename T, typename FormatContextOut>
auto format_enum_with_meta(FormatContextOut &it, char spec, const T &value) {
    using meta_t = decltype(enum_meta_type(meta_enum::type_t<T>{}));
    // get metadata of enum member
    auto meta = meta_t::from_value(value); // optional<meta>
    if (meta) {
        return format_enum_value_meta(it, spec, meta.value());
    }
    // fall back to enum class name + bits value
    it = fmt::format_to(it, "{}::", meta_t::name);
    return format_bits(it, value);
}

/// @brief Format bitmask using the underlaying enum value meta.
// s: the name
// l: the name and value
template <typename T, typename FormatContextOut>
auto format_bitmask_with_meta(FormatContextOut &it, char spec,
                              const bitmask::bitmask<T> &bm) {
    using meta_t = decltype(enum_meta_type(meta_enum::type_t<T>{}));
    // get metadata of enum member
    auto meta = meta_t::from_value(static_cast<T>(bm)); // optional<meta>
    if (meta) {
        return format_enum_value_meta(it, spec, meta.value());
    }
    // composed type, decompose
    auto bit_mask = (bm.mask_value >> 1) + 1;
    it = fmt::format_to(it, "(");
    auto bits = bm.bits();
    bool sep = false; // one time flag
    if (bits > 0) {
        while (bit_mask) {
            if (auto bit = bit_mask & bm.bits(); bit) {
                auto meta = meta_t::from_value(static_cast<T>(bit)).value();
                it = fmt::format_to(it, "{}{}", sep ? "|" : "", meta.name);
                sep = true;
            }
            bit_mask >>= 1;
        }
    }
    switch (spec) {
    case 'l': {
        if (bits > 0) {
            it = fmt::format_to(it, ",");
        }
        it = format_bits(it, bits, std::bit_width(bm.mask_value));
        [[fallthrough]];
    }
    default: {
        return fmt::format_to(it, ")");
    }
    }
}

} // namespace tula::fmt_utils

namespace fmt {

/// @brief Formatter for enums with meta
template <tula::enum_utils::EnumWithMeta T>
struct formatter<T> : tula::fmt_utils::charspec_formatter_base<'s', 'd', 'l'>
// d: the bit value
// s: the name (default)
// l: the name and value
{

    template <typename FormatContext>
    auto format(const T &value, FormatContext &ctx) -> decltype(ctx.out()) {
        auto it = ctx.out();
        auto spec = spec_handler();
        if (spec == 'd') {
            return tula::fmt_utils::format_bits(it, value);
        }
        // format enum with meta
        return tula::fmt_utils::format_enum_with_meta(it, spec, value);
    }
};

/// @brief Formatter for bitmasks
template <typename T>
struct formatter<bitmask::bitmask<T>>
    : tula::fmt_utils::charspec_formatter_base<'l', 'd', 's'>
// d: the bit value
// s: the name
// l: the name and value (default)
{
    template <typename FormatContext>
    auto format(const bitmask::bitmask<T> &bm, FormatContext &ctx)
        -> decltype(ctx.out()) {
        auto it = ctx.out();
        auto spec = spec_handler();
        if (spec == 'd') {
            auto w = std::bit_width(
                tula::meta::size_cast<std::size_t>(bm.mask_value));
            return tula::fmt_utils::format_bits(it, bm.bits(), w);
        }
        if constexpr (tula::enum_utils::EnumWithMeta<T>) {
            return tula::fmt_utils::format_bitmask_with_meta(it, spec, bm);
        } else {
            // fallback to format value
            it = format_to(it, "bits(");
            it = tula::fmt_utils::format_bits(it, bm.bits());
            return format_to(it, ")");
        }
    }
};

/// @brief Formatter for enum meta
template <typename EnumType, typename UnderlyingType, std::size_t size>
struct formatter<meta_enum::MetaEnum<EnumType, UnderlyingType, size>>
    : tula::fmt_utils::charspec_formatter_base<'l', 's'> {
    //  'l': name{str...}
    //  's': {str...}

    template <typename FormatContext>
    auto format(const meta_enum::MetaEnum<EnumType, UnderlyingType, size> &meta,
                FormatContext &ctx) -> decltype(ctx.out()) {
        auto it = ctx.out();
        auto spec = spec_handler();
        auto str = tula::fmt_utils::remove_space(std::string(meta.string));
        if (str.empty()) {
            return format_to(it, "{}", meta.name);
        }
        switch (spec) {
        case 'l': {
            return format_to(it, "{}{{{}}}", meta.name, str);
        }
        case 's': {
            return format_to(it, "{{{}}}", str);
        }
        default: {
            return it;
        }
        }
        return it;
    }
};

/// @brief Formatter for enum value meta
template <typename EnumType>
struct formatter<meta_enum::MetaEnumMember<EnumType>>
    : tula::fmt_utils::charspec_formatter_base<'l', 'd', 's'>
// d: the bit value
// s: the name
// l: the name and value string (default)
{
    template <typename FormatContext>
    auto format(const meta_enum::MetaEnumMember<EnumType> &meta,
                FormatContext &ctx) -> decltype(ctx.out()) {
        auto it = ctx.out();
        auto spec = this->spec_handler();
        return tula::fmt_utils::format_enum_value_meta(it, spec, meta);
        //         if constexpr (bitmask::bitmask<
        //                           EnumType>::mask_value > 0) {
        //             SPDLOG_TRACE("has value mask {}", meta.name);
        //             return formatter<bitmask::bitmask<EnumType>>::format(
        //                 bitmask::bitmask<EnumType>{meta.value}, ctx);
        //         } else {
        //             SPDLOG_TRACE("not have value mask {}", meta.name);
        //             // not a bit mask
        //             auto it = ctx.out();
        //             auto spec = this->spec_handler();
        //             if (spec == 'd') {
        //                 return fmt_utils::format_bits(it, meta.value);
        //             }
        //             switch (spec) {
        //             case 's': {
        //                 return format_to(it, "{}", meta.name);
        //             }
        //             case 'l': {
        //                 // format using the string
        //                 auto str =
        //                 fmt_utils::remove_space(std::string(meta.string));
        //                 str.erase(str.begin(),
        //                           ++std::find(str.begin(), str.end(), '='));
        //                 // without explicit def
        //                 if (str.empty()) {
        //                     it = format_to(it, "{}(", meta.name);
        //                     it = fmt_utils::format_bits(it, meta.value);
        //                     return format_to(it, ")");
        //                 }
        //                 return format_to(it, "{}({})", meta.name, str);
        //             }
        //             default: {
        //                 return it;
        //             }
        //             }
        //         }
    }
};

} // namespace fmt
