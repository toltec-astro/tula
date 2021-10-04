#pragma once

#include "bits.h"
#include "concepts.h"
#include "meta.h"
#include <bitmask/bitmask.hpp>
#include <meta_enum/meta_enum.hpp>
#include <variant>

namespace meta_enum::internal {

// Allow composition of members in the definition
template <typename T>
constexpr auto operator|(const IntWrapper<T> &lhs, const IntWrapper<T> &rhs)
    -> IntWrapper<T> {
    return IntWrapper<T>(lhs.value | rhs.value);
}

} // namespace meta_enum::internal

// Define a macro with meta struct using the instrument in meta_enum
#define TULA_ENUM_IMPL(Type, UnderlyingType, ...)                              \
    struct Type##_meta {                                                       \
        constexpr static auto internal_size = []() constexpr noexcept {        \
            using IntWrapperType =                                             \
                meta_enum::internal::IntWrapper<UnderlyingType>;               \
            IntWrapperType __VA_ARGS__; /* NOLINT */                           \
            return std::initializer_list<IntWrapperType>{__VA_ARGS__}.size();  \
        };                                                                     \
        constexpr static auto meta = meta_enum::internal::parseMetaEnum<       \
            Type, UnderlyingType, internal_size()>(#Type, #__VA_ARGS__, []() { \
            using IntWrapperType =                                             \
                meta_enum::internal::IntWrapper<UnderlyingType>;               \
            IntWrapperType __VA_ARGS__; /* NOLINT */                           \
            return meta_enum::internal::resolveEnumValuesArray<                \
                Type, UnderlyingType, internal_size()>({__VA_ARGS__});         \
        }());                                                                  \
        [[maybe_unused]] constexpr static auto to_name = [](Type e) noexcept { \
            for (const auto &member : meta.members) {                          \
                if (member.value == e)                                         \
                    return member.name;                                        \
            }                                                                  \
            return std::string_view("__INVALID__");                            \
        };                                                                     \
        [[maybe_unused]] constexpr static auto from_name =                     \
            [](std::string_view s) noexcept                                    \
            -> std::optional<meta_enum::MetaEnumMember<Type>> {                \
            for (const auto &member : meta.members) {                          \
                if (member.name == s)                                          \
                    return member;                                             \
            }                                                                  \
            return std::nullopt;                                               \
        };                                                                     \
        [[maybe_unused]] constexpr static auto from_value =                    \
            [](Type v) noexcept                                                \
            -> std::optional<meta_enum::MetaEnumMember<Type>> {                \
            for (const auto &member : meta.members) {                          \
                if (member.value == v)                                         \
                    return member;                                             \
            }                                                                  \
            return std::nullopt;                                               \
        };                                                                     \
        [[maybe_unused]] constexpr static auto from_index =                    \
            [](std::size_t i) noexcept {                                       \
                std::optional<meta_enum::MetaEnumMember<Type>> result;         \
                if (i < meta.members.size())                                   \
                    result = meta.members[i];                                  \
                return result;                                                 \
            };                                                                 \
        constexpr static auto &name = meta.name;                               \
        constexpr static auto &string = meta.string;                           \
        constexpr static auto &members = meta.members;                         \
    }

namespace tula::enum_utils {

// Use function traits to allow traits in different namespaces

template <typename T>
struct type_t {};

/// @brief Traits to test if enum has meta
template <typename T>
auto enum_has_meta(type_t<T>) -> std::false_type;

/// @brief Traits to obtain the meta type of enum
template <typename T>
void enum_meta_type(type_t<T>);

template <typename T>
concept EnumWithMeta = tula::meta::EnumClass<T> && requires {
    { enum_has_meta(type_t<T>{}) } -> std::same_as<std::true_type>;
};

template <typename T>
concept BitFlag = tula::meta::EnumClass<T> && requires {
    typename bitmask::bitmask<T>::mask_value;
} &&(bitmask::bitmask<T>::mask_value > 0);

template <typename T>
concept BitFlagWithMeta = EnumWithMeta<T> && BitFlag<T>;

namespace internal {

    /// @brief Return the mask value ( values | ...) of enum type.
    template <tula::meta::EnumClass T>
    constexpr auto get_mask_value() noexcept {
        using U = std::underlying_type_t<T>;
        U mask_value{0};
        if constexpr (EnumWithMeta<T>) {
            // Build value mask using meta members
            using meta_t = decltype(enum_meta_type(type_t<T>{}));
            for (const auto &meta : meta_t::members) {
                mask_value |= static_cast<U>(meta.value);
            }
        } else if constexpr (BitFlag<T>) {
            mask_value = bitmask::bitmask<T>::mask_value;
        } else {
            // No mask defined for T
        }
        // This is 0 otherwise
        return mask_value;
    };

    /// @brief Return the number of set bits for enum
    template <tula::meta::EnumClass T>
    constexpr auto bitcount(T v) noexcept {
        return bits_utils::count(static_cast<std::underlying_type_t<T>>(v));
    }

} // namespace internal

/// @brief The mask value ( values | ...) of enum type.
template <tula::meta::EnumClass T>
inline constexpr auto bitmask_v = internal::get_mask_value<T>();

/// @brief The number of used bits in enum type.
template <tula::meta::EnumClass T>
inline constexpr auto bitwidth_v = bits_utils::fls(bitmask_v<T>);

/// @brief Check if enum value has multiple bits set.
template <tula::meta::EnumClass auto v>
inline constexpr auto is_compound_v = (internal::bitcount(v) > 1);

/// @brief Decompose enum value to an array of enum values
template <tula::meta::EnumClass auto v>
constexpr auto decompose() noexcept {
    using T = decltype(v);
    using U = std::underlying_type_t<T>;
    return tula::meta::apply_sequence(
        [](auto &&...i) { return std::array{static_cast<T>(i)...}; },
        bits_utils::decompose<static_cast<U>(v)>());
}

/// @brief Convert enum type to a variant type.
template <tula::meta::EnumClass auto v,
          template <decltype(v), typename...> class TT>
constexpr auto enum_to_variant() noexcept {
    // decompose
    using T = decltype(v);
    using U = std::underlying_type_t<T>;
    return tula::meta::apply_const_sequence(
        [](auto... i) {
            return std::variant<TT<static_cast<T>(decltype(i)::value)>...>{};
        },
        bits_utils::decompose<static_cast<U>(v)>());
}

template <tula::meta::EnumClass auto v,
          template <decltype(v), typename...> class TT>
using enum_to_variant_t = decltype(enum_to_variant<v, TT>());

/// @brief Return the enum member values as array.
template <tula::enum_utils::EnumWithMeta T>
constexpr auto values() noexcept {
    using meta_t = decltype(enum_meta_type(type_t<T>{}));
    constexpr std::size_t n = meta_t::members.size();
    std::array<T, n> values_;
    for (std::size_t i = 0; i < n; ++i) {
        values_[i] = meta_t::members[i].value;
    }
    return values_;
}

/// @brief Return the enum member names
template <tula::enum_utils::EnumWithMeta T>
constexpr auto names() noexcept {
    using meta_t = decltype(enum_meta_type(type_t<T>{}));
    constexpr std::size_t n = meta_t::members.size();
    std::array<std::string_view, n> names_;
    for (std::size_t i = 0; i < n; ++i) {
        names_[i] = meta_t::members[i].name;
    }
    return names_;
}

/// @brief Return the enum name for value.
template <tula::enum_utils::EnumWithMeta T>
constexpr auto name(T v) {
    using meta_t = decltype(enum_meta_type(type_t<T>{}));
    auto opt_member = meta_t::from_value(v);
    if (opt_member.has_value()) {
        return opt_member.value().name;
    }
    return "(undef)";
}

} // namespace tula::enum_utils

/// @brief Make available the enum traits classes.
#define TULA_ENUM_REGISTER(Type)                                               \
    constexpr auto enum_meta_type(enum_utils::type_t<Type>)->Type##_meta;      \
    constexpr auto enum_has_meta(enum_utils::type_t<Type>)->std::true_type

/// @brief Define enum in-line, has to be registered separately.
#define TULA_ENUM_DECL(Type, UnderlyingType, ...)                              \
    enum class Type : UnderlyingType { __VA_ARGS__ };                          \
    TULA_ENUM_IMPL(Type, UnderlyingType, __VA_ARGS__)

/// @brief Define enum out of line. The enum is registered automatically.
#define TULA_ENUM(Type, UnderlyingType, ...)                                   \
    TULA_ENUM_DECL(Type, UnderlyingType, __VA_ARGS__);                         \
    TULA_ENUM_REGISTER(Type)

/// @brief Return the metadata of the enum.
#define TULA_ENUM_META(Type) Type##_meta::meta

/// @brief Set enum as bitflag with mask value.
#define TULA_BITFLAG_VALUE_MASK(Type, ValueMask)                               \
    /* NOLINTBEGIN */ BITMASK_DEFINE_VALUE_MASK(Type,                          \
                                                ValueMask) /* NOLINTEND */     \
    static_assert(true)

/// @brief Define bitmask element for bit flag.
#define TULA_BITFLAG_MAX_ELEMENT(...)                                          \
    /* NOLINTBEGIN */ BITMASK_DEFINE_MAX_ELEMENT(__VA_ARGS__) /* NOLINTEND */  \
    static_assert(true)

/// @brief Define enum as bitflag.
#define TULA_BITFLAG(Type, UnderlyingType, ValueMask, ...)                     \
    TULA_ENUM(Type, UnderlyingType, __VA_ARGS__);                              \
    TULA_BITFLAG_VALUE_MASK(Type, ValueMask)
