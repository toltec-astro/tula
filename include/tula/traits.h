#pragma once

#include <iterator>
#include <type_traits>

/// @brief Some commonly used type traits.

namespace tula::meta {

template <typename... Ts>
constexpr bool always_false = false;

/// @brief Check if type is template instance.
template <typename, template <typename...> typename = std::void_t,
          template <typename...> typename = std::void_t>
struct is_instance : public std::false_type {};

template <typename... Ts, template <typename...> typename T>
struct is_instance<T<Ts...>, T> : public std::true_type {};

template <typename... Ts, template <typename...> typename T,
          template <typename...> typename U, typename... Us>
struct is_instance<T<U<Us...>, Ts...>, T, U> : public std::true_type {};

/// @brief Check if type is integral constant.
template <typename T, typename = void>
struct is_integral_constant : std::false_type {};
template <typename T, T v>
struct is_integral_constant<std::integral_constant<T, v>> : std::true_type {};

template <typename F, typename... Args>
// NOLINTNEXTLINE(readability-named-parameter)
constexpr auto arity(F (*)(Args...)) {
    return sizeof...(Args);
}

} // namespace tula::meta
