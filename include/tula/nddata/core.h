#pragma once

// #include "tula/formatter/utils.h"
// #include <fmt/core.h>
#include <optional>
#include <string>

namespace tula::nddata {

template <typename T>
struct type_traits {
    using index_t = std::size_t;
    using physical_type_t = std::optional<std::string>;
    using unit_t = std::optional<std::string>;
    using label_t = std::string;
};

/**
 * @brief  A CRTP base class provides a common interface for NDData
 *
 * @tparam Derived The subclass.
 */
template <typename Derived>
struct NDData {

    using index_t = typename type_traits<Derived>::index_t;
    using physical_type_t = typename type_traits<Derived>::physical_type_t;
    using unit_t = typename type_traits<Derived>::unit_t;
    using label_t = typename type_traits<Derived>::label_t;

    auto derived() const noexcept -> const auto & {
        return static_cast<const Derived &>(*this);
    }
    auto derived() noexcept -> auto & { return static_cast<Derived &>(*this); }

    template <typename U = Derived>
    requires requires(U &&u) { u.data; }
    auto operator()() const noexcept -> const auto & { return derived().data; }

    template <typename U = Derived>
    requires requires(U &&u) { u.data; }
    auto operator()() noexcept -> auto & { return derived().data; }
};

} // namespace tula::nddata

/*
namespace fmt {

template <typename Derived>
struct formatter<tula::nddata::NDData<Derived>>
    : tula::fmt_utils::nullspec_formatter_base {

    template <typename FormatContext>
    auto format(const tula::nddata::NDData<Derived> &data_,
                FormatContext &ctx) {
        auto it = ctx.out();
        decltype(auto) data = data_.derived();
        format_to(it, "NDData{}", data());
    }
};

} // namespace fmt
*/
