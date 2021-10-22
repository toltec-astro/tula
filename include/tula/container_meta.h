#pragma once

#include "traits.h"
#include <tuple>
#include <variant>

namespace tula::meta {

template <class T, class U>
struct is_one_of;
template <class T, class... Ts>
struct is_one_of<T, std::variant<Ts...>>
    : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};

template <class T, class... Ts>
struct is_one_of<T, std::tuple<Ts...>>
    : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};

template <typename T>
concept TupleLike =
    is_instance<T, std::tuple>::value || is_instance<T, std::variant>::value;

template <typename T, typename U>
concept IsOneOf = TupleLike<U> && is_one_of<T, U>::value;

} // namespace tula::meta
