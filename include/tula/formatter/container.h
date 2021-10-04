#pragma once
#include "utils.h"
#include <fmt/ranges.h>
#include <optional>
#include <variant>

namespace fmt {

template <typename T>
struct formatter<std::optional<T>> : formatter<T> {
    template <typename FormatContext>
    auto format(const std::optional<T> &opt, FormatContext &ctx)
        -> decltype(ctx.out()) {
        if (opt) {
            return formatter<T>::format(opt.value(), ctx);
        }
        return format_to(ctx.out(), "(nullopt)");
    }
};

template <>
struct formatter<std::nullopt_t, char, void>
    : tula::fmt_utils::nullspec_formatter_base {
    template <typename FormatContext>
    auto format(const std::nullopt_t & /*unused*/, FormatContext &ctx) noexcept
        -> decltype(ctx.out()) {
        return format_to(ctx.out(), "(nullopt)");
    }
};

template <>
struct formatter<std::monostate, char, void>
    : tula::fmt_utils::nullspec_formatter_base {
    template <typename FormatContext>
    auto format(const std::monostate & /*unused*/, FormatContext &ctx) noexcept
        -> decltype(ctx.out()) {
        return format_to(ctx.out(), "(undef)");
    }
};

/*
template <typename... Ts>
struct formatter<std::unordered_map<Ts...>, char, void> {

    constexpr static auto prefix = '{';
    constexpr static auto postfix = '}';

    template <typename ParseContext>
    FMT_CONSTEXPR auto parse(ParseContext &ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::unordered_map<Ts...> &values, FormatContext &ctx)
        -> decltype(ctx.out()) {
        auto out = detail::copy(prefix, ctx.out());
        size_t i = 0;
        auto it = std::begin(values);
        auto end = std::end(values);
        for (; it != end; ++it) {
            if (i > 0)
                out = detail::write_delimiter(out);
            out = detail::write_range_entry<char>(out, *it);
            ++i;
        }
        return detail::copy(postfix, out);
    }
};
template <typename T, typename U>
struct formatter<std::pair<T, U>, char, void>
    : tula::fmt_utils::nullspec_formatter_base {
    template <typename FormatContext>
    auto format(const std::pair<T, U> &p, FormatContext &ctx)
        -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{{{}: {}}}", p.first, p.second);
    }
};
*/

template <typename T, typename... Rest>
struct formatter<std::variant<T, Rest...>, char, void>
    : tula::fmt_utils::charspec_formatter_base<'l', '0', 's'> {
    // 0: value
    // s: value (t)
    // l: value (type) (default)
    template <typename FormatContext>
    auto format(const std::variant<T, Rest...> &v, FormatContext &ctx)
        -> decltype(ctx.out()) {
        auto it = ctx.out();
        auto spec = spec_handler();
        std::visit(
            [&it, &spec](const auto &v) {
                using V = std::decay_t<decltype(v)>;
                if constexpr (tula::meta::StringLike<V>) {
                    format_to(it, "\"{}\"", v);
                } else {
                    format_to(it, "{}", v);
                }
                if (spec == '0') {
                    return;
                }
                // format with type code
                std::string t{};
                if constexpr (std::is_same_v<V, std::monostate>) {
                    return;
                } else if constexpr (std::is_same_v<V, bool>) {
                    t = "bool";
                } else if constexpr (std::is_same_v<V, int>) {
                    t = "int";
                } else if constexpr (std::is_same_v<V, double>) {
                    t = "doub";
                } else if constexpr (tula::meta::StringLike<V>) {
                    t = "str";
                } else {
                    // fallback to rtti type id
                    t = typeid(V).name();
                }
                switch (spec) {
                case 's': {
                    format_to(it, " ({})", t[0]);
                    return;
                }
                case 'l': {
                    format_to(it, " ({})", t);
                    return;
                }
                }
                return;
            },
            v);
        return it;
    }
};

} // namespace fmt
