#pragma once

#include <tuple>
#include <utility>

#include "concepts.h"
#include "traits.h"

/// @brief Some meta programing related functions and tooling.

namespace tula::meta {

namespace internal {

template <typename... Ts>
auto fwd_capture_impl(Ts &&...xs) noexcept {
    return std::make_tuple(std::forward<decltype(xs)>(xs)...);
}

} // namespace internal

/**
 * @brief Cast numerical type with bound check in debug mode.
 */
template <Integral T, Integral U>
auto size_cast(U value) noexcept -> T {
    assert(value == static_cast<U>(static_cast<T>(value)));
    return static_cast<T>(value);
}

/// @brief Call function for each item in sequence.
template <typename T, class Func, T... Is,
          template <typename TT, TT...> typename S>
constexpr auto apply_sequence(Func &&f, S<T, Is...> /*unused*/)
    -> decltype(auto) {
    return std::forward<Func>(f)(Is...);
}

/// @brief Call function for each item in sequency, wrapped in integral
/// constant.
template <typename T, class Func, T... Is,
          template <typename TT, TT...> typename S>
constexpr auto apply_const_sequence(Func &&f, S<T, Is...> /*unused*/)
    -> decltype(auto) {
    return std::forward<Func>(f)(std::integral_constant<T, Is>{}...);
}

template <typename tuple_t>
constexpr auto t2a(tuple_t &&tuple) {
    constexpr auto get_array = [](auto &&...x) {
        return std::array{std::forward<decltype(x)>(x)...};
    };
    return std::apply(get_array, std::forward<tuple_t>(tuple));
}

} // namespace tula::meta

/** Some commonly used constructs
 */

#define BOOLT(...) std::integral_constant<bool, __VA_ARGS__>

#define TULA_DECAY(x) std::decay_t<decltype(x)>

#define TULA_SIZET(...) tula::meta::size_cast<std::size_t>(__VA_ARGS__)

#define TULA_FWD(...) std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

#define TULA_FWD_CAPTURE(...)                                                  \
    TULA_GET_MACRO_NARG_OVERLOAD(TULA_FWD_CAPTURE, __VA_ARGS__)
#define TULA_FWD_CAPTURE1(x1)                                                  \
    tula::meta::internal::fwd_capture_impl(TULA_FWD(x1))
#define TULA_FWD_CAPTURE2(x1, x2)                                              \
    tula::meta::internal::fwd_capture_impl(TULA_FWD(x1), TULA_FWD(x2))
#define TULA_FWD_CAPTURE3(x1, x2, x3)                                          \
    tula::meta::internal::fwd_capture_impl(TULA_FWD(x1), TULA_FWD(x2),         \
                                           TULA_FWD(x3))
#define TULA_FWD_CAPTURE4(x1, x2, x3, x4)                                      \
    tula::meta::internal::fwd_capture_impl(TULA_FWD(x1), TULA_FWD(x2),         \
                                           TULA_FWD(x3), TULA_FWD(x4))
#define TULA_FWD_CAPTURE5(x1, x2, x3, x4, x5)                                  \
    tula::meta::internal::fwd_capture_impl(                                    \
        TULA_FWD(x1), TULA_FWD(x2), TULA_FWD(x3), TULA_FWD(x4), TULA_FWD(x5))

#define TULA_LIFT(...) GET_MACRO_NARG_OVERLOAD(TULA_LIFT, __VA_ARGS__)
#define TULA_LIFT1(f)                                                          \
    [](auto &&...xs) noexcept(                                                 \
        noexcept(f(TULA_FWD(xs)...))) -> decltype(f(TULA_FWD(xs)...)) {        \
        return f(TULA_FWD(xs)...);                                             \
    }
#define TULA_LIFT2(o, f)                                                       \
    [&(o)](auto &&...xs) noexcept(noexcept((o).f(TULA_FWD(xs)...)))            \
        -> decltype((o).f(TULA_FWD(xs)...)) { return (o).f(TULA_FWD(xs)...); }

#define define_has_member_traits(class_name, member_name)                      \
    class has_##member_name {                                                  \
        typedef char yes_type;                                                 \
        typedef long no_type;                                                  \
        template <typename U>                                                  \
        static yes_type test(decltype(&U::member_name));                       \
        template <typename U>                                                  \
        static no_type test(...);                                              \
                                                                               \
    public:                                                                    \
        static constexpr bool value =                                          \
            sizeof(test<class_name>(0)) == sizeof(yes_type);                   \
    }
