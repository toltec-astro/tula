#pragma once

#include <concepts>
#include <string>
#include <type_traits>

#include "traits.h"

namespace tula::meta {

// [Type constraints]
template <typename T>
concept Integral = std::is_integral_v<T>;

template <typename T>
concept SignedIntegral = Integral<T> && std::is_signed_v<T>;

template <typename T>
concept UnsignedIntegral = Integral<T> && !SignedIntegral<T>;

template <typename T>
concept IntegralConstant = is_integral_constant<T>::value;

template <typename T>
concept EnumClass = std::is_enum_v<T>;

template <typename T>
concept CStr =
    std::same_as<char,
                 std::remove_cvref_t<std::remove_pointer_t<std::decay_t<T>>>>;

template <typename T>
concept String = is_instance<T, std::basic_string>::value;

template <typename T>
concept StringView = is_instance<T, std::basic_string_view>::value;

template <typename T>
concept StringLike = CStr<T> || String<T> || StringView<T>;

// [Interface constraints]
template <typename T>
concept Sized = requires(T &t) {
    std::size(t);
};

template <typename T>
concept Iterable = requires(T &t) {
    std::begin(t);
    std::end(t);
};

// [Return type constraints]
template <template <typename...> typename traits, typename F, typename... Args>
concept RTHasTraits =
    traits<typename std::invoke_result<F, Args...>::type>::value;

template <template <typename...> typename T, typename F, typename... Args>
concept RTIsInstance =
    is_instance<typename std::invoke_result<F, Args...>::type, T>::value;

template <typename T, typename F, typename... Args>
concept RTIsType =
    std::same_as<typename std::invoke_result<F, Args...>::type, T>;

template <typename F, typename... Args>
concept Invocable = requires(F &&f, Args &&...args) {
    std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
};

template <typename F>
concept IsNullary = Invocable<F>;

} // namespace tula::meta
