#pragma once

#include "meta.h"
#include <optional>

namespace tula::meta {

template <auto v, typename T>
struct case_t {
    static constexpr auto value = v;
    using type = T;
    using value_type = decltype(v);
};

template <bool b, typename T>
using cond_t = case_t<b, T>;

namespace internal {

template <typename Cond, typename... Rest>
struct select_impl
    : std::enable_if_t<
          std::is_same_v<typename Cond::value_type, bool>,
          std::conditional_t<Cond::value, Cond, select_impl<Rest...>>> {};

template <typename T>
struct select_impl<T> {
    using type = T; // else clause
};

template <bool b, typename T>
struct select_impl<cond_t<b, T>> {
    // last cond, enforce true
    static_assert(b, "ALL OF THE CASES ARE FALSE BUT NO DEFAULT IS GIVEN");
    using type = T;
};

template <auto v, typename T>
struct case_to_cond {
    using type = T;
};
template <auto v, auto vt, typename T>
struct case_to_cond<v, case_t<vt, T>> {
    using type = cond_t<v == vt, T>;
};

template <auto v, typename T>
using case_to_cond_t = typename case_to_cond<v, T>::type;

} // namespace internal

template <typename Case, typename... Rest>
using select_t = typename internal::select_impl<Case, Rest...>::type;

template <auto v, typename Case, typename... Rest>
using switch_t = select_t<internal::case_to_cond_t<v, Case>,
                          internal::case_to_cond_t<v, Rest>...>;

template <auto... vs>
struct cases;

template <auto v>
using scalar_t = std::integral_constant<decltype(v), v>;

namespace internal {

template <typename T>
struct switch_invoke_impl;

template <auto v0_, auto... vs_>
struct switch_invoke_impl<cases<v0_, vs_...>> {
    using v0 = scalar_t<v0_>;
    using vs = std::conditional_t<sizeof...(vs_) == 0, void, cases<vs_...>>;
    template <typename Func, typename... Args>
    using rt0 = std::invoke_result_t<Func, v0, Args...>;

    template <typename Func, typename... Args>
    using return_type = std::conditional_t<
        std::is_same_v<rt0<Func, Args...>, void>, void,
        std::conditional_t<(std::is_same_v<rt0<Func, Args...>,
                                           std::invoke_result_t<
                                               Func, scalar_t<vs_>, Args...>> &&
                            ...),
                           std::optional<rt0<Func, Args...>>, void>>;
};

} // namespace internal

template <typename cases, typename Func, typename T, typename... Args>
auto switch_invoke(Func &&f, T v, Args &&...args) ->
    typename internal::switch_invoke_impl<cases>::template return_type<
        Func, Args...> {
    using impl = internal::switch_invoke_impl<cases>;
    using v0 = typename impl::v0;
    constexpr auto return_void =
        std::is_same_v<void, typename impl::template rt0<Func, Args...>>;
    if (v == v0::value) {
        if constexpr (return_void) {
            std::forward<Func>(f)(v0{}, std::forward<Args>(args)...);
            return;
        } else {
            return std::forward<Func>(f)(v0{}, std::forward<Args>(args)...);
        }
    }
    if constexpr (std::is_same_v<typename impl::vs, void>) {
        // all checked, no match
        if constexpr (return_void) {
            return;
        } else {
            return std::nullopt;
        }
    } else {
        // check next
        return switch_invoke<typename impl::vs>(std::forward<Func>(f), v,
                                                args...);
    }
}

} // namespace tula::meta
