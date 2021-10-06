#pragma once

#include <Eigen/Core>
#include <optional>
#include <string>

namespace tula::nddata {

namespace internal {

// Some of the implementation related types
template <typename T>
struct impl_traits {
    using index_t = std::size_t;
};

} // namespace internal

/**
 * @brief  A CRTP base class provides a common interface for NDData
 *
 * @tparam Derived The subclass.
 */
template <typename Derived>
struct NDData {

    using index_t = typename internal::impl_traits<Derived>::index_t;
    using physical_type_t = std::optional<std::string>;
    using unit_t = std::optional<std::string>;
    using label_t = std::string;

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
