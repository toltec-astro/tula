#pragma once
#include "meta.h"
#include <Eigen/Core>
#include <fmt/format.h>
#include <type_traits>

namespace Eigen {

using VectorXI = Eigen::Matrix<Eigen::Index, Eigen::Dynamic, 1>;
using MatrixXI = Eigen::Matrix<Eigen::Index, Eigen::Dynamic, Eigen::Dynamic>;
using VectorXb = Eigen::Matrix<bool, Eigen::Dynamic, 1>;
using MatrixXb = Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>;

} // namespace Eigen

namespace tula::eigen_utils {

namespace internal {
// Eigen CRTP does not work for vector block
template <typename T>
struct is_vector_block : std::false_type {};
template <typename VectorType, int Size>
struct is_vector_block<Eigen::VectorBlock<VectorType, Size>> : std::true_type {
};

} // namespace internal

/// [traits]
template <typename T>
inline constexpr bool is_vblock_v =
    internal::is_vector_block<std::decay_t<T>>::value;

template <typename T>
inline constexpr bool is_eigen_v =
    std::is_base_of_v<Eigen::EigenBase<std::decay_t<T>>, std::decay_t<T>> ||
    is_vblock_v<std::decay_t<T>>;

template <typename T>
inline constexpr bool is_dense_v =
    std::is_base_of_v<Eigen::DenseBase<std::decay_t<T>>, std::decay_t<T>>;

/// True if type manages its own data, e.g. MatrixXd, etc.
template <typename T>
inline constexpr bool is_plain_v =
    std::is_base_of_v<Eigen::PlainObjectBase<std::decay_t<T>>, std::decay_t<T>>;

/// [concepts]
template <typename T>
concept IsVBlock = is_vblock_v<T>;

template <typename T>
concept IsEigen = is_eigen_v<T>;

template <typename T>
concept IsDense = is_dense_v<T>;

template <typename T>
concept IsPlain = is_plain_v<T>;

template <IsEigen T>
struct type_traits {
    using Derived = typename std::decay_t<T>;
    constexpr static Eigen::StorageOptions order =
        Derived::IsRowMajor ? Eigen::RowMajor : Eigen::ColMajor;
    // related types
    using Vector = std::conditional_t<
        order == Eigen::RowMajor,
        Eigen::Matrix<typename Derived::Scalar, 1, Derived::ColsAtCompileTime,
                      Eigen::RowMajor>,
        Eigen::Matrix<typename Derived::Scalar, Derived::RowsAtCompileTime, 1,
                      Eigen::ColMajor>>;
    using Matrix =
        Eigen::Matrix<typename Derived::Scalar, Derived::RowsAtCompileTime,
                      Derived::ColsAtCompileTime, order>;
    using VectorMap = Eigen::Map<Vector>;
    using MatrixMap = Eigen::Map<Matrix>;
};

/// @brief Return true if the underlying storage is contiguous.
template <typename Derived>
auto is_contiguous(const Eigen::DenseBase<Derived> &m) -> bool {
    // SPDLOG_TRACE(
    //     "size={} outerstride={} innerstride={} outersize={} innersize={}",
    //     m.size(), m.outerStride(), m.innerStride(), m.outerSize(),
    //     m.innerSize());
    if (m.innerStride() != 1) {
        return false;
    }
    return (m.size() <= m.innerSize()) || (m.outerStride() == m.innerSize());
}

/**
 * @brief Create std::vector from data held by Eigen types. Copies the data.
 * @param m The Eigen type.
 * @tparam order The storage order to follow when copying the data.
 * Default is the same as input.
 */
template <typename Derived,
          Eigen::StorageOptions order = type_traits<Derived>::order>
auto to_stdvec(const Eigen::DenseBase<Derived> &m) {
    using Eigen::Dynamic;
    using Scalar = typename Derived::Scalar;
    std::vector<Scalar> vec(m.size());
    Eigen::Map<Eigen::Matrix<Scalar, Dynamic, Dynamic, order>>(
        vec.data(), m.rows(), m.cols()) = m;
    return vec;
}

/**
 * @brief Create std::vector from data held by Eigen types. Copies the data.
 * @param m The Eigen type.
 * @param order The storage order to follow when copying the data.
 */
template <typename Derived>
auto to_stdvec(const Eigen::DenseBase<Derived> &m,
               Eigen::StorageOptions order) {
    using Eigen::ColMajor;
    using Eigen::RowMajor;
    if (order & RowMajor) {
        return to_stdvec<Derived, RowMajor>(m.derived());
    }
    return to_stdvec<Derived, ColMajor>(m.derived());
}

/**
 * @brief Create Eigen::Map from std::vector.
 * @tparam order The storage order to follow when mapping the data.
 * Default is Eigen::ColMajor.
 */
template <Eigen::StorageOptions order = Eigen::ColMajor, typename Scalar,
          typename... Rest>
auto as_eigen(std::vector<Scalar, Rest...> &v) {
    return Eigen::Map<Eigen::Matrix<Scalar, Eigen::Dynamic, 1, order>>(
        v.data(), v.size());
}

/**
 * @brief Create Eigen::Map from std::vector with shape
 * @tparam order The storage order to follow when mapping the data.
 * Default is Eigen::ColMajor.
 */
template <Eigen::StorageOptions order = Eigen::ColMajor, typename Scalar,
          typename... Rest>
auto as_eigen(std::vector<Scalar, Rest...> &v, Eigen::Index nrows,
              Eigen::Index ncols) {
    using Eigen::Dynamic;
    assert(nrows * ncols == v.size());
    return Eigen::Map<Eigen::Matrix<Scalar, Dynamic, Dynamic, order>>(
        v.data(), nrows, ncols);
}

/**
 * @brief Create Eigen::Map from std::vector.
 * @tparam order The storage order to follow when mapping the data.
 * Default is Eigen::ColMajor.
 */
template <Eigen::StorageOptions order = Eigen::ColMajor, typename Scalar,
          typename... Rest>
auto as_eigen(const std::vector<Scalar, Rest...> &v) {
    return Eigen::Map<const Eigen::Matrix<Scalar, Eigen::Dynamic, 1, order>>(
        v.data(), v.size());
}

/**
 * @brief Create Eigen::Matrix from std::vector of std::pair.
 */
template <typename Scalar, typename... Rest>
auto to_eigen(const std::vector<std::pair<Scalar, Scalar>, Rest...> &v) {
    Eigen::Matrix<Scalar, 2, Eigen::Dynamic> m(2, v.size());
    for (Eigen::Index i = 0; i < m.cols(); ++i) {
        m.coeffRef(0, i) = v[tula::meta::size_cast<std::size_t>(i)].first;
        m.coeffRef(1, i) = v[tula::meta::size_cast<std::size_t>(i)].second;
    }
    return m;
}

} // namespace tula::eigen_utils
