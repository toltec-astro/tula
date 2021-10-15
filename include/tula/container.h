#pragma once
#include "concepts.h"
#include "eigen.h"
#include "logging.h"
#include <iterator>
#include <regex>

namespace tula::container_utils {

template <typename T>
concept Populatable =
    std::input_or_output_iterator<T> || tula::eigen_utils::IsEigen<T>;

/// @brief Populate data using data in existing container.
/// Optioanally, when \p func is provided, the elements are mapped using
/// \p func.
template <tula::meta::Iterable In, Populatable Out, typename... F>
void populate(In &&in, Out &out, F &&...func) {
    if constexpr (tula::meta::Sized<Out>) {
        if constexpr (requires { out.resize(in.size()); }) {
            // resize if available
            out.resize(in.size());
        }
        // check size match
        assert(out.size() == in.size());
    }
    // figure out output for transform
    auto out_iter = [&]() {
        if constexpr (eigen_utils::IsEigen<Out>) {
            // eigen type
            return out.reshaped().begin();
        } else if constexpr (std::input_or_output_iterator<Out>) {
            return out;
        } else {
            static_assert(tula::meta::always_false<Out>,
                          "NOT ABLE TO GET OUTPUT ITERATOR");
        }
    }();
    // to handle custom transform function
    auto run_with_iter = [&](const auto &begin, const auto &end) {
        // no func provided, run a copy with implicit conversion
        if constexpr ((sizeof...(F)) == 0) {
            // SPDLOG_TRACE("use implicit conversion");
            std::copy(begin, end, out_iter);
            // run transform with func
        } else {
            std::transform(begin, end, out_iter,
                           std::forward<decltype(func)>(func)...);
        };
    };
    // handle l- and r- values differently for input
    if constexpr (std::is_lvalue_reference_v<In>) {
        // run with normal iter
        SPDLOG_TRACE("use copy iterator");
        run_with_iter(in.begin(), in.end());
    } else {
        // run with move iter
        SPDLOG_TRACE("use move iterator");
        run_with_iter(std::make_move_iterator(in.begin()),
                      std::make_move_iterator(in.end()));
    }
}

/// @brief Create data from data in existing container. Makes use
/// of \ref populate.
template <tula::meta::SizedIterable Out, typename In, typename... F>
requires requires { std::is_default_constructible_v<Out>; }
auto create(In &&in, F &&...func) -> Out {
    Out out;
    if constexpr (tula::meta::is_instance<Out, std::vector>::value) {
        // reserve for vector
        // SPDLOG_TRACE("reserve vector{}", in.size());
        out.reserve(in.size());
    }
    if constexpr (tula::eigen_utils::IsPlain<Out>) {
        SPDLOG_TRACE("create eigen");
        populate(FWD(in), out, FWD(func)...);
        return out;
    } else if constexpr (tula::meta::Iterable<Out>) {
        SPDLOG_TRACE("create stl");
        // populate with iterator
        auto out_iter = [&]() {
            if constexpr (requires {
                              out.push_back(
                                  std::declval<typename Out::value_type>());
                          }) {
                SPDLOG_TRACE("use back_inserter");
                return std::back_inserter(out);
            } else if constexpr (requires {
                                     out.insert(std::declval<
                                                typename Out::value_type>());
                                 }) {
                SPDLOG_TRACE("use inserter");
                return std::inserter(out, out.end());
            } else {
                static_assert(meta::always_false<Out>,
                              "NOT ABLE TO GET OUTPUT ITERATOR");
            }
        }();
        populate(std::forward<In>(in), out_iter,
                 std::forward<decltype(func)>(func)...);
        return out;
    } else {
        static_assert(meta::always_false<Out>,
                      "NOT KNOW HOW TO POPULATE OUT TYPE");
    }
}

/// @brief Returns true if \p v ends with \p ending.
template <tula::meta::SizedIterable T, tula::meta::SizedIterable U>
auto startswith(const T &v, const U &prefix) noexcept -> bool {
    if (prefix.size() > v.size()) {
        return false;
    }
    return std::equal(prefix.begin(), prefix.end(), v.begin());
}

/// @brief Returns true if \p v ends with \p ending.
template <tula::meta::SizedIterable T, tula::meta::SizedIterable U>
auto endswith(const T &v, const U &ending) noexcept -> bool {
    if (ending.size() > v.size()) {
        return false;
    }
    return std::equal(ending.rbegin(), ending.rend(), v.rbegin());
}

/// @brief Rearrange nested vector to a flat one *in place*.
template <typename T>
void ravel(std::vector<std::vector<T>> &v) noexcept {
    std::size_t total_size = 0;
    for (const auto &sub : v) {
        total_size += sub.size(); // I wish there was a transform_accumulate
    }
    std::vector<T> result;
    result.reserve(total_size);
    for (const auto &sub : v) {
        result.insert(result.end(), std::make_move_iterator(sub.begin()),
                      std::make_move_iterator(sub.end()));
    }
    v = std::move(result);
}

/// @brief Returns the index of element in vector.
template <typename T>
auto indexof(const std::vector<T> &vec, const T &v) noexcept
    -> std::optional<typename std::vector<T>::difference_type> {
    auto it = std::find(vec.begin(), vec.end(), v);
    if (it == vec.end()) {
        return std::nullopt;
    }
    return std::distance(vec.begin(), it);
}

/// @brief Create [index, element] like enumerate for container.
template <tula::meta::Iterable T>
auto unordered_enumerate(const T &v) noexcept
    -> std::unordered_map<std::size_t, typename T::value_type> {
    std::unordered_map<std::size_t, typename T::value_type> ret;
    std::size_t i = 0;
    for (auto it = v.begin(); it != v.cend(); ++it) {
        ret.insert({i, *it});
        ++i;
    }
    return ret;
}

/// @brief Create [index, element] like enumerate for container.
template <tula::meta::SizedIterable T>
auto enumerate(const T &v) noexcept
    -> std::vector<std::pair<std::size_t, typename T::value_type>> {
    std::vector<std::pair<std::size_t, typename T::value_type>> ret;
    for (std::size_t i = 0; i < v.size(); ++i) {
        ret.emplace_back(std::piecewise_construct, std::forward_as_tuple(i),
                         std::forward_as_tuple(v[i]));
    }
    return ret;
}

/// @brief Create index sequence for container.
template <tula::meta::Integral Index>
auto index(Index size) noexcept {
    std::vector<Index> ret(size);
    eigen_utils::as_eigen(ret).setLinSpaced(size, 0, size - 1);
    return ret;
}

/// @brief Create index sequence for container.
template <tula::meta::Sized T>
auto index(const T &v) noexcept {
    return index(v.size());
}

template <typename T>
using Slice = std::tuple<std::optional<T>, std::optional<T>, std::optional<T>>;
using IndexSlice = Slice<Eigen::Index>;

/// @brief Parse python-like slice string.
template <typename T = Eigen::Index>
auto parse_slice(const std::string &slice_str) {
    Slice<T> result;
    auto &[start, stop, step] = result;
    std::size_t istart{0};
    std::size_t istop{0};
    std::size_t istep{0};
    std::string value_pattern;
    if constexpr (std::is_integral_v<T>) {
        value_pattern = "[-+]?[0-9]+";
        istart = 1;
        istop = 3;
        istep = 4;
    } else {
        value_pattern = "[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?";
        istart = 1;
        istop = 4;
        istep = 6; // NOLINT(readability-magic-numbers)
    }
    std::string pattern = fmt::format("^({})?(:)?({})?:?({})?$", value_pattern,
                                      value_pattern, value_pattern);
    std::regex re_slice{pattern};
    SPDLOG_TRACE("checking slice str {} with {}", slice_str, pattern);
    std::smatch match;
    T var;
    if (std::regex_match(slice_str, match, re_slice)) {
        if (match[istart].matched) {
            std::istringstream iss;
            iss.str(match[istart].str());
            iss >> var;
            start = var;
        }
        if (match[istop].matched) {
            std::istringstream iss;
            iss.str(match[istop].str());
            iss >> var;
            stop = var;
        }
        if (match[istep].matched) {
            std::istringstream iss;
            iss.str(match[istep].str());
            iss >> var;
            step = var;
        }
    }
    SPDLOG_TRACE("parsed slice {}", result);
    return result;
}

// start stop step len
template <typename T>
using BoundedSlice = std::tuple<T, T, T, T>;

/// @brief Convert slice to indices
template <tula::meta::Integral T, tula::meta::Integral N>
auto to_indices(Slice<T> slice, N n) noexcept {
    BoundedSlice<T> result{
        std::get<0>(slice).value_or(0),
        std::get<1>(slice).value_or(n),
        std::get<2>(slice).value_or(1),
        0,
    };
    auto &[start, stop, step, size] = result;
    if (start < 0) {
        start += n;
    }
    if (stop < 0) {
        start += n;
    }
    if (stop >= n) {
        stop = n;
    }
    size = (stop - start) / step + (((stop - start) % step) ? 1 : 0);
    return result;
}

// std::vector<T>&& src - src MUST be an rvalue reference
// std::vector<T> src - src MUST NOT, but MAY be an rvalue reference
template <typename T>
void append(std::vector<T> source, std::vector<T> &destination) noexcept {
    if (destination.empty()) {
        destination = std::move(source);
    } else {
        destination.insert(std::end(destination),
                           std::make_move_iterator(std::begin(source)),
                           std::make_move_iterator(std::end(source)));
    }
}

} // namespace tula::container_utils

namespace fmt {

template <typename T, typename Char>
struct formatter<tula::container_utils::Slice<T>, Char>
    : tula::fmt_utils::nullspec_formatter_base {

    template <typename FormatContext>
    auto format(const tula::container_utils::Slice<T> &slice,
                FormatContext &ctx) {
        auto it = ctx.out();
        const auto &[start, stop, step] = slice;
        return format_to(it, "[{}:{}:{}]", start, stop, step);
    }
};

template <typename T, typename Char>
struct formatter<tula::container_utils::BoundedSlice<T>, Char>
    : tula::fmt_utils::nullspec_formatter_base {

    template <typename FormatContext>
    auto format(const tula::container_utils::BoundedSlice<T> &slice,
                FormatContext &ctx) {
        auto it = ctx.out();
        const auto &[start, stop, step, size] = slice;
        return format_to(it, "[{}:{}:{}]({})", start, stop, step, size);
    }
};

} // namespace fmt
