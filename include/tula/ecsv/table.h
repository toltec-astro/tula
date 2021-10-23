#pragma once

#include "../nddata/eigen.h"
#include "../nddata/labelmapper.h"
#include "../container.h"
#include "hdr.h"
#include "../eigen.h"
#include <functional>
#include <ranges>
#include <stdexcept>

namespace tula::ecsv {
struct ECSVHeaderView;
} // namespace tula::ecsv

namespace tula::nddata {

template <>
struct type_traits<tula::ecsv::ECSVHeaderView> {
    using index_t = std::size_t;
    using label_t = std::string;
};

} // namespace tula::nddata
namespace tula::ecsv {

namespace internal {

template <typename Pred>
requires tula::meta::Invocable<Pred, const ECSVColumn &>
auto get_colnames_filtered(const ECSVHeader &hdr, Pred &&pred) {
    using label_t = tula::nddata::type_traits<ECSVHeaderView>::label_t;
    std::vector<label_t> colnames;
    for (const auto &c : hdr.cols()) {
        if (pred(c)) {
            colnames.push_back(c.name);
        }
    }
    return colnames;
}

template <typename T>
concept ECSVDataType = tula::meta::Arithmetic<T> ||
    tula::meta::is_instance<T, std::complex>::value || tula::meta::String<T>;

constexpr std::size_t array_data_block_size = 1024;

template <ECSVDataType T, auto order = Eigen::ColMajor>
using eigen_array_data_t = Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic,
                                        order, Eigen::Dynamic, Eigen::Dynamic>;

template <ECSVDataType T>
using std_array_data_t = std::vector<std::vector<T>>;

template <ECSVDataType T>
constexpr static bool use_eigen_array_data =
    !tula::meta::is_instance<T, std::basic_string>::value;

template <ECSVDataType T, auto order = Eigen::ColMajor>
using array_data_t =
    std::conditional_t<use_eigen_array_data<T>, eigen_array_data_t<T, order>,
                       std_array_data_t<T>>;

} // namespace internal

/**
 * @brief The class to help navigate ECSV header.
 *
 */
struct ECSVHeaderView : tula::nddata::LabelMapper<ECSVHeaderView> {
    using Base = tula::nddata::LabelMapper<ECSVHeaderView>;
    using index_t = Base::index_t;
    using label_t = Base::label_t;
    using labels_t = Base::labels_t;

    ECSVHeaderView(const ECSVHeader &hdr)
        : ECSVHeaderView{hdr, tula::container_utils::to_stdvec(hdr.colnames())} {}

    /// @brief Create a view that maps a subset of columns
    ECSVHeaderView(const ECSVHeader &hdr, std::vector<label_t> colnames)
        : Base{std::move(colnames)}, m_hdr(hdr) {

        auto base_mapper = Base(tula::container_utils::to_stdvec(hdr.colnames()));
        for (const auto &view_colname : this->labels()) {
            m_view_index.push_back(base_mapper.index(view_colname));
            m_view_cols.push_back(
                std::cref(hdr.cols()[base_mapper.index(view_colname)]));
        }
    }

    template <typename Pred>
    requires tula::meta::Invocable<Pred, const ECSVColumn &>
    ECSVHeaderView(const ECSVHeader &hdr, Pred &&pred)
        : ECSVHeaderView{hdr, internal::get_colnames_filtered(
                                  hdr, std::forward<Pred>(pred))} {}

    [[nodiscard]] auto col(index_t i) const -> const ECSVColumn & {
        if (i >= m_view_index.size()) {
            throw std::runtime_error(
                fmt::format("column index {} out of range 0-{}", i,
                            m_view_index.size() - 1));
        }
        return m_hdr.cols()[this->m_view_index[i]];
    }
    [[nodiscard]] auto col(const label_t &name) const -> const ECSVColumn & {
        return m_hdr.cols()[this->m_view_index[this->index(name)]];
    }

    [[nodiscard]] auto cols() const -> decltype(auto) {
        return std::ranges::transform_view(
            m_view_cols, [] (auto ref) {return ref.get();});
    }

    /// @brief Return the list of indices of columns in the original header.
    [[nodiscard]] auto indices() const -> decltype(auto) {
        return this->m_view_index;
    }

    [[nodiscard]] auto colnames() const noexcept -> decltype(auto) {
        return this->labels();
        // return std::ranges::transform_view(cols(), &ECSVColumn::name);
    }
    [[nodiscard]] auto datatypes() const noexcept -> decltype(auto) {
        return std::ranges::transform_view(cols(), &ECSVColumn::datatype);
    }

private:
    const ECSVHeader &m_hdr;
    using view_index_t = std::vector<index_t>;
    view_index_t m_view_index;

    using view_cols_t = std::vector<std::reference_wrapper<const ECSVColumn>>;
    view_cols_t m_view_cols;
};

template <internal::ECSVDataType value_t_, typename col_data_t_>
struct ColDataRef {
    using index_t = ECSVHeaderView::index_t;
    using value_t = value_t_;
    using col_data_t = col_data_t_;
    constexpr static auto is_eigen_data =
        internal::use_eigen_array_data<value_t>;

    col_data_t data;
    const ECSVColumn &col;
    auto operator()(index_t i) -> value_t & {
        if constexpr (is_eigen_data) {
            return data.coeffRef(i);
        } else {
            return data[i];
        }
    }
    auto operator()(index_t i) const -> const value_t & {
        if constexpr (is_eigen_data) {
            return data.coeffRef(i);
        } else {
            return data[i];
        }
    }

    template <typename T>
    auto set_value(index_t i, const T &v) {
        // this checks the value T for string like, and dispatch coordingly
        if constexpr ((is_eigen_data && !tula::meta::StringLike<T>) ||
                      (!is_eigen_data && tula::meta::StringLike<T>)) {
            this->operator()(i) = v;
        }
    }
};

/// @brief ECSV data object.
template <internal::ECSVDataType T,
          std::size_t block_size_ = internal::array_data_block_size,
          Eigen::StorageOptions order = Eigen::ColMajor>
struct ArrayData : ECSVHeaderView {
    using Base = ECSVHeaderView;
    using index_t = Base::index_t;
    using label_t = Base::label_t;
    using labels_t = Base::labels_t;
    using value_t = T;
    using data_t = internal::array_data_t<T, order>;
    constexpr static auto is_eigen_data =
        internal::use_eigen_array_data<value_t>;
    constexpr static auto block_size = block_size_;

    ArrayData(ECSVHeaderView hdr_view) : Base{std::move(hdr_view)} {
        this->init_data();
    }
    ArrayData(const ECSVHeader &hdr, std::vector<label_t> colnames)
        : Base{hdr, std::move(colnames)} {
        this->init_data();
    }

    ArrayData(const ECSVHeader &hdr)
        : ArrayData{hdr, tula::container_utils::to_stdvec(hdr.colnames())} {}

    template <typename Pred>
    requires tula::meta::Invocable<Pred, const ECSVColumn &>
    ArrayData(const ECSVHeader &hdr, Pred &&pred)
        : ArrayData{hdr, internal::get_colnames_filtered(
                             hdr, std::forward<Pred>(pred))} {}

    auto operator()(index_t i) {
        if constexpr (is_eigen_data) {
            auto c = data.col(i);
            return ColDataRef<value_t, decltype(c)>{c, this->col(i)};
        } else {
            return ColDataRef<value_t, std::vector<value_t> &>{data.at(i),
                                                               this->col(i)};
        }
    }

    auto operator()(index_t i) const {
        if constexpr (is_eigen_data) {
            auto c = data.col(i);
            return ColDataRef<value_t, decltype(c)>{c};
        } else {
            return ColDataRef<value_t, const std::vector<value_t> &>{
                data.at(i)};
        }
    }

    auto operator()(const label_t &name) {
        return this->operator()(this->index(name));
    }
    auto operator()(const label_t &name) const {
        return this->operator()(this->index(name));
    }

    [[nodiscard]] auto row(index_t j) {
        this->ensure_row_size_for_index(j);
        if constexpr (is_eigen_data) {
            return data.row(j);
        } else {
            // We need to build a view to the j th element in each vector
            return std::ranges::transform_view(
                data, [j = j](auto &v) -> index_t & { return v.at(j); });
        }
    }

    auto array() const &noexcept -> const auto & { return data; }
    auto array() &&noexcept { return std::move(data); }

    /// @brief Truncate the data to have n records.
    void truncate(index_t n) {
        if constexpr (is_eigen_data) {
            this->data.conservativeResize(n, Eigen::NoChange);
        } else {
            for (auto &v : this->data) {
                v.resize(n);
            }
        }
    }

    void ensure_row_size_for_index(index_t j) {
        auto get_new_size = [](index_t old_size,
                               index_t j) -> std::optional<index_t> {
            if (j >= old_size) {
                index_t n_blocks{0};
                if (old_size == 0) {
                    n_blocks = 1;
                } else {
                    n_blocks = j / old_size + index_t(j % old_size > 0);
                    // when j = 0 this happens
                    if (n_blocks == 0) {
                        n_blocks = 1;
                    }
                }
                return n_blocks * block_size;
            }
            return std::nullopt;
        };
        std::optional<index_t> new_size{std::nullopt};
        if constexpr (is_eigen_data) {
            new_size = get_new_size(this->data.rows(), j);

        } else {
            new_size = get_new_size(
                tula::meta::size_cast<index_t>(this->data[0].size()), j);
        }
        if (new_size.has_value()) {
            this->truncate(new_size.value());
        }
    }

private:
    data_t data;
    void init_data() {
        if constexpr (is_eigen_data) {
            // for eigen type, each column is the column, each row is a record
            this->data.resize(block_size, this->size());
        } else {
            this->data.resize(this->size());
            // for std vector type, each inner vector is a column.
            // the elements in the inner vector are records of that column
            for (auto &v : this->data) {
                v.resize(block_size);
            }
        }
    }
};

template <typename... ArrayDataTypes>
struct ECSVDataLoader {
    using index_t = ECSVHeaderView::index_t;
    using label_t = ECSVHeaderView::label_t;
    using ref_t = std::variant<std::reference_wrapper<ArrayDataTypes>...>;
    using refs_t = std::vector<ref_t>;
    ECSVDataLoader(const ECSVHeader &hdr, ArrayDataTypes &...array_data_)
        : m_hdr_view{hdr}, m_array_data_refs{std::ref(array_data_)...} {
        m_ref_index.resize(m_hdr_view.size());
        for (std::size_t i = 0; i < m_array_data_refs.size(); ++i) {
            auto [cols, indices] = std::visit(
                [](auto ref) {
                    decltype(auto) d = ref.get();
                    return std::tuple{d.cols(), d.indices()};
                },
                m_array_data_refs[i]);
            for (std::size_t j = 0; j < indices.size(); ++j) {
                auto k = indices[j];
                // k: original header index,
                // i: array data refs index
                // j: col index in array data
                m_ref_index[k].emplace_back(i, j);
            }
        }
    }

    template <tula::meta::IsUnary F>
    void visit_col(index_t k, F &&f) {
        for (auto [i, j] : m_ref_index[k]) {
            std::visit(
                [&f, j = j](auto ref) -> void {
                    auto &array_data = ref.get();
                    f(array_data(j));
                },
                m_array_data_refs[i]);
        }
    }

    template <tula::meta::IsUnary F>
    void visit_col(const label_t &name, F &&f) {
        visit_col(m_hdr_view.index(name), std::forward<F>(f));
    }

    void ensure_row_size_for_index(index_t j) {
        for (const auto &array_data : m_array_data_refs) {
            std::visit(
                [j = j](auto ref) { ref.get().ensure_row_size_for_index(j); },
                array_data);
        }
    }
    void truncate(index_t n) {
        for (const auto &array_data : m_array_data_refs) {
            std::visit([n = n](auto ref) { ref.get().truncate(n); },
                       array_data);
        }
    }

    auto get_ref_index() const -> decltype(auto) { return m_ref_index; }

private:
    ECSVHeaderView m_hdr_view;
    refs_t m_array_data_refs;
    // This holds the list of index pairs to locate the data col for each
    // hdr col
    std::vector<std::vector<std::pair<index_t, index_t>>> m_ref_index;
};

} // namespace tula::ecsv

namespace fmt {

template <>
struct formatter<tula::ecsv::ECSVHeaderView>
    : tula::fmt_utils::nullspec_formatter_base {
    template <typename FormatContext>
    auto format(const tula::ecsv::ECSVHeaderView &hdr, FormatContext &ctx) {
        auto it = ctx.out();
        return format_to(it, "ECSVHeaderView(ncols={})", hdr.size());
    }
};

template <typename T, auto... vs>
struct formatter<tula::ecsv::ArrayData<T, vs...>>
    : tula::fmt_utils::nullspec_formatter_base {
    template <typename FormatContext>
    auto format(const tula::ecsv::ArrayData<T, vs...> &array_data,
                FormatContext &ctx) {
        auto it = ctx.out();
        return format_to(it, "ECSVArrayData(ncols={})", array_data.size());
    }
};

template <typename... Ts>
struct formatter<tula::ecsv::ColDataRef<Ts...>>
    : tula::fmt_utils::nullspec_formatter_base {
    template <typename FormatContext>
    auto format(const tula::ecsv::ColDataRef<Ts...> &colref,
                FormatContext &ctx) {
        auto it = ctx.out();
        return format_to(it, "ECSVColRef(name={})", colref.col.name);
    }
};
} // namespace fmt
