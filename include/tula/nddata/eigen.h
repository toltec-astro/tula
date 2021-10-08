#pragma once

#include "../eigen.h"
#include "core.h"
#include <Eigen/Core>
#include <Eigen/src/Core/util/XprHelper.h>
#include <optional>
#include <string>
#include <type_traits>

namespace tula::nddata {

template <tula::eigen_utils::IsPlain PlainObject>
struct EigenData;

template <tula::eigen_utils::IsPlain PlainObject>
struct type_traits<EigenData<PlainObject>> : type_traits<void> {
    using Base = type_traits<void>;
    using index_t = Eigen::Index;
    using physical_type_t = Base::physical_type_t;
    using unit_t = Base::unit_t;
    using label_t = Base::label_t;
};

/**
 * @brief  A NDData class for holding Eigen plain object.
 *
 * @tparam PlainObject The Eigen data.
 *
 * This class owns the data.
 */
template <tula::eigen_utils::IsPlain PlainObject>
struct EigenData : NDData<EigenData<PlainObject>> {
    using Base = NDData<EigenData<PlainObject>>;
    using index_t = typename Base::index_t;

    EigenData() = default;
    EigenData(const PlainObject &data_) : data{data_} {};
    EigenData(PlainObject &&data_) : data{std::move(data_)} {};

    PlainObject data;
};

/**
 * @brief  A NDData class that refer to Eigen object.
 *
 * @tparam PlainObject The Eigen data object.
 *
 * This class does not own the data.
 */
template <tula::eigen_utils::IsEigen T>
struct EigenDataRef : NDData<EigenDataRef<T>> {
    using Base = NDData<EigenDataRef<T>>;
    using index_t = typename Base::index_t;
    using ref_t = typename Eigen::internal::ref_selector<T>::type;
    EigenDataRef() = default;
    EigenDataRef(const T &data_) : data{data_} {};
    EigenDataRef(T &&data_) : data{std::move(data_)} {};

    ref_t data;
};

} // namespace tula::nddata
