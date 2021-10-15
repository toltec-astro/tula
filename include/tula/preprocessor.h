#pragma once

/// @brief Some preprocessor tricks.

/**
 * Overload macro with number of arguments.
 *
 * Credit: https://stackoverflow.com/a/45600545/1824372
 */
#define TULA_DETAILS_BUGFX(x) x
#define TULA_DETAILS_NARG2(...)                                                \
    TULA_DETAILS_BUGFX(TULA_DETAILS_NARG1(__VA_ARGS__, TULA_DETAILS_RSEQN()))
#define TULA_DETAILS_NARG1(...)                                                \
    TULA_DETAILS_BUGFX(TULA_DETAILS_ARGSN(__VA_ARGS__))
#define TULA_DETAILS_ARGSN(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define TULA_DETAILS_RSEQN() 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define TULA_DETAILS_FUNC2(name, n) name##n
#define TULA_DETAILS_FUNC1(name, n) TULA_DETAILS_FUNC2(name, n)
#define TULA_GET_MACRO_NARG_OVERLOAD(func, ...)                                \
    TULA_DETAILS_FUNC1(func,                                                   \
                       TULA_DETAILS_BUGFX(TULA_DETAILS_NARG2(__VA_ARGS__)))    \
    (__VA_ARGS__)

/**
 * TULA_ANONYMOUS_VAR(str) introduces an identifier starting with
 * str and ending with a number that varies with the line.
 *
 * Credit: https://github.com/facebook/folly/blob/master/folly/Preprocessor.h
 */
#ifndef TULA_ANONYMOUS_VAR
#define FB_CONCATENATE_IMPL(s1, s2) s1##s2
#define FB_CONCATENATE(s1, s2) FB_CONCATENATE_IMPL(s1, s2)
#ifdef __COUNTER__
#define TULA_ANONYMOUS_VAR(str) FB_CONCATENATE(str, __COUNTER__)
#else
#define TULA_ANONYMOUS_VAR(str) FB_CONCATENATE(str, __LINE__)
#endif

/**
 * TULA_X introduces an identifier starting with "_"
 * that varies with the line.
 */
#define TULA_X TULA_ANONYMOUS_VAR(_)
#endif
