#pragma once

#include "../container.h"
#include "../eigen.h"
#include "../nddata/eigen.h"
#include "../nddata/labelmapper.h"
#include "hdr.h"
#include "tula/meta.h"
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
    for (const auto &col : hdr.cols()) {
        if (pred(col)) {
            colnames.push_back(col.name);
        }
    }
    return colnames;
}

template <typename T>
concept ECSVDataType =
    tula::meta::Arithmetic<T> ||
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
        : ECSVHeaderView{hdr,
                         tula::container_utils::to_stdvec(hdr.colnames())} {}

    /// @brief Create a view that maps a subset of columns
    ECSVHeaderView(const ECSVHeader &hdr, std::vector<label_t> colnames)
        : Base{std::move(colnames)}, m_hdr(hdr) {

        auto base_mapper =
            Base(tula::container_utils::to_stdvec(hdr.colnames()));
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

    [[nodiscard]] auto col(index_t idx) const -> const ECSVColumn & {
        if (idx >= m_view_index.size()) {
            throw std::runtime_error(
                fmt::format("column index {} out of range 0-{}", idx,
                            m_view_index.size() - 1));
        }
        return m_hdr.cols()[this->m_view_index[idx]];
    }
    [[nodiscard]] auto col(const label_t &name) const -> const ECSVColumn & {
        return m_hdr.cols()[this->m_view_index[this->index(name)]];
    }

    [[nodiscard]] auto cols() const -> decltype(auto) {
        return std::ranges::transform_view(m_view_cols,
                                           [](auto ref) { return ref.get(); });
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
        return std::ranges::transform_view(
            cols(), [](const auto &col) { return col.datatype; });
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

    auto operator()(index_t idx) -> value_t & {
        if constexpr (is_eigen_data) {
            return data.coeffRef(idx);
        } else {
            return data[idx];
        }
    }
    auto operator()(index_t idx) const -> const value_t & {
        if constexpr (is_eigen_data) {
            return data.coeffRef(idx);
        } else {
            return data[idx];
        }
    }

    template <typename T>
    auto set_value(index_t idx, const T &value) {
        // this checks the value T for string like, and dispatch coordingly
        if constexpr ((is_eigen_data && !tula::meta::StringLike<T>) ||
                      (!is_eigen_data && tula::meta::StringLike<T>)) {
            this->operator()(idx) = value;
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

    auto operator()(index_t idx) {
        if constexpr (is_eigen_data) {
            auto data_col = data.col(idx);
            return ColDataRef<value_t, decltype(data_col)>{data_col,
                                                           this->col(idx)};
        } else {
            return ColDataRef<value_t, std::vector<value_t> &>{data.at(idx),
                                                               this->col(idx)};
        }
    }

    auto operator()(index_t idx) const {
        if constexpr (is_eigen_data) {
            auto data_col = data.col(idx);
            return ColDataRef<value_t, decltype(data_col)>{data_col,
                                                           this->col(idx)};
        } else {
            return ColDataRef<value_t, const std::vector<value_t> &>{
                data.at(idx), this->col(idx)};
        }
    }

    auto operator()(const label_t &name) {
        return this->operator()(this->index(name));
    }
    auto operator()(const label_t &name) const {
        return this->operator()(this->index(name));
    }

    [[nodiscard]] auto row(index_t idx) {
        this->ensure_row_size_for_index(idx);
        if constexpr (is_eigen_data) {
            return data.row(idx);
        } else {
            // We need to build a view to the idx-th element in each vector
            return std::ranges::transform_view(
                data, [idx = idx](auto &value) -> index_t & {
                    return value.at(idx);
                });
        }
    }

    [[nodiscard]] auto array() const &noexcept -> const auto & { return data; }
    auto array() &&noexcept { return std::move(data); }

    /// @brief Truncate the data to have n rows.
    void truncate(index_t n) {
        if (this->empty()) {
            // no-op if empty
            return;
        }
        SPDLOG_TRACE("trucate data with n_rows={}", n);
        if constexpr (is_eigen_data) {
            SPDLOG_TRACE("prev data_shape {} {}", this->data.rows(),
                         this->data.cols());
            this->data.conservativeResize(n, Eigen::NoChange);
            SPDLOG_TRACE("current data_shape {} {}", this->data.rows(),
                         this->data.cols());
        } else {
            for (auto &value : this->data) {
                value.resize(n);
            }
        }
    }
    auto row_size() const noexcept -> index_t {
        if (this->empty()) {
            return 0;
        }
        if constexpr (is_eigen_data) {
            SPDLOG_TRACE("current data_shape {} {}", this->data.rows(),
                         this->data.cols());
            return this->data.rows();
        } else {
            SPDLOG_TRACE("current data_shape {} {}", this->data[0].size(),
                         this->data.size());
            return tula::meta::size_cast<index_t>(this->data[0].size());
        }
    }

    void ensure_row_size_for_index(index_t idx) {
        if (this->empty()) {
            // no-op if empty
            return;
        }
        auto get_new_size = [](index_t old_size,
                               index_t idx) -> std::optional<index_t> {
            SPDLOG_TRACE("check row size for idx: old_size={} idx={}", old_size,
                         idx);
            // here we always requrest n_rows as multiples of block_size
            if (idx >= old_size) {
                auto new_size = idx + 1;
                auto n_blocks =
                    new_size / block_size + index_t(new_size % block_size > 0);
                SPDLOG_TRACE("computed n_blocks={}", n_blocks);
                if (n_blocks == 0) {
                    // this should never happen
                    SPDLOG_TRACE("n_blocks=0, set n_blocks={}", n_blocks);
                    n_blocks = 1;
                }
                SPDLOG_TRACE("set new size to n_blocks={}, size={}", n_blocks,
                             n_blocks * block_size);
                return n_blocks * block_size;
            } else {
                SPDLOG_TRACE("no new size needed");
            }
            return std::nullopt;
        };
        auto new_size = get_new_size(this->row_size(), idx);
        if (new_size.has_value()) {
            this->truncate(new_size.value());
        }
    }

private:
    data_t data;
    void init_data() {
        if (this->empty()) {
            // do not do any initialization as this does not hold any data.
            return;
        }
        if constexpr (is_eigen_data) {
            // for eigen type, each column is the column, each row is a record
            this->data.resize(block_size, this->size());
        } else {
            this->data.resize(this->size());
            // for std vector type, each inner vector is a column.
            // the elements in the inner vector are records of that column
            for (auto &value : this->data) {
                value.resize(block_size);
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
                    decltype(auto) data = ref.get();
                    return std::tuple{data.cols(), data.indices()};
                },
                m_array_data_refs[i]);
            for (std::size_t j = 0; j < indices.size(); ++j) {
                auto col_idx = indices[j];
                // col_idx: original header index,
                // i: array data refs index
                // j: col index in array data
                m_ref_index[col_idx].emplace_back(i, j);
            }
        }
    }

    template <tula::meta::IsUnary F>
    void visit_col(index_t idx, F &&func) {
        for (auto [i, data_col_idx] : m_ref_index[idx]) {
            std::visit(
                [&func, data_col_idx = data_col_idx](auto ref) -> void {
                    auto &array_data = ref.get();
                    func(array_data(data_col_idx));
                },
                m_array_data_refs[i]);
        }
    }

    template <tula::meta::IsUnary F>
    void visit_col(const label_t &name, F &&func) {
        visit_col(m_hdr_view.index(name), std::forward<F>(func));
    }

    auto colrefs(index_t idx) -> decltype(auto) {
        using colref_t = std::variant<decltype(std::declval<ArrayDataTypes &>()(
            index_t()))...>;
        using colrefs_t = std::vector<colref_t>;
        colrefs_t result;
        visit_col(idx, [&result](auto colref) { result.emplace_back(colref); });
        return result;
    }

    void ensure_row_size_for_index(index_t idx) {
        for (const auto &array_data : m_array_data_refs) {
            std::visit(
                [idx = idx](auto ref) {
                    ref.get().ensure_row_size_for_index(idx);
                },
                array_data);
        }
    }
    void truncate(index_t size) {
        for (const auto &array_data : m_array_data_refs) {
            std::visit([size = size](auto ref) { ref.get().truncate(size); },
                       array_data);
        }
    }

    [[nodiscard]] auto get_ref_index() const -> decltype(auto) {
        return m_ref_index;
    }

    [[nodiscard]] auto header_view() const -> decltype(auto) {
        return m_hdr_view;
    }

private:
    ECSVHeaderView m_hdr_view;
    refs_t m_array_data_refs;
    // This holds the list of index pairs to locate the data col for each
    // hdr col
    std::vector<std::vector<std::pair<index_t, index_t>>> m_ref_index;
};

namespace internal {
template <internal::ECSVDataType... Ts>
struct table_data_traits_impl {
    using value_t = std::tuple<ArrayData<Ts>...>;
    using loader_t = ECSVDataLoader<ArrayData<Ts>...>;
    using supported_dtypes_t = std::tuple<Ts...>;

    template <internal::ECSVDataType T>
    constexpr static auto dtype_index() {
        return tula::meta::index_in_tuple<T, supported_dtypes_t>::value;
    }

    static auto init_value(const ECSVHeader &hdr) {
        return value_t{ArrayData<Ts>(hdr, [](const auto &col) {
            return col.datatype == dtype_str<Ts>();
        })...};
    }

    static auto init_loader(const ECSVHeader &hdr, value_t &value) {
        return ECSVDataLoader(hdr, std::get<ArrayData<Ts>>(value)...);
    }
};
} // namespace internal

struct ECSVTable {
    using index_t = ECSVHeaderView::index_t;
    using label_t = ECSVHeaderView::label_t;
    // bool, int8, int16, int32, int64
    // uint8, uint16, uint32, uint64
    // float16, float32, float64, float128
    // complex64, complex128, complex256
    using table_data_traits = internal::table_data_traits_impl<
        bool, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t,
        uint64_t, float, double, long double, std::complex<float>,
        std::complex<double>, std::complex<long double>, std::string>;
    using table_data_t = table_data_traits::value_t;
    using loader_t = table_data_traits::loader_t;

    ECSVTable(ECSVHeader hdr)
        : m_hdr{std::move(hdr)}, m_data{table_data_traits::init_value(m_hdr)},
          m_loader{table_data_traits::init_loader(m_hdr, m_data)} {};

    auto header() const -> decltype(auto) { return m_hdr; }
    auto header_view() const -> decltype(auto) {
        return m_loader.header_view();
    }
    auto loader() const -> decltype(auto) { return m_loader; }

    template <internal::ECSVDataType T>
    auto array_data() const -> decltype(auto) {
        return std::get<ArrayData<T>>(m_data);
    }
    template <internal::ECSVDataType T>
    auto col(index_t idx) {
        auto [i, j] = m_loader.get_ref_index()[idx].front();
        return this->array_data<T>().array().col(j);
        // auto colref = m_loader.colrefs(idx).front();
        // because there is no duplicate in the loader, we can just get the
        // first col
        // unwrap the variant
        // return std::get<decltype(std::declval<ArrayData<T> &>()(index_t()))>(
        // colref);
    }
    template <internal::ECSVDataType T>
    auto col(const label_t &name) -> decltype(auto) {
        return col<T>(this->header_view().index(name));
    }

    auto cols() const -> std::size_t { return m_hdr.size(); }
    auto rows() const -> std::size_t { return m_current_rows; }
    auto empty() const -> bool { return rows() == 0; }

    template <tula::meta::Iterable It>
    void load_rows(It &rows) {
        if (!empty()) {
            throw std::runtime_error(fmt::format(
                "table already contains data n_rows={}", m_current_rows));
        }
        int row_idx = 0;
        for (const auto &row : rows) {
            // populate data
            m_loader.ensure_row_size_for_index(row_idx);
            auto row_size = row.size();
            if (row_size != this->cols()) {
                throw std::runtime_error(fmt::format(
                    "inconsistent number of fields at row {}: {} != {}",
                    row_idx, row_size, this->cols()));
            }
            for (std::size_t col_idx = 0; col_idx < row_size; ++col_idx) {
                m_loader.visit_col(
                    col_idx, [&row_idx, &col_idx, &row](auto colref) {
                        using value_t = typename decltype(colref)::value_t;
                        value_t value;
                        std::istringstream(row.at(col_idx)) >> value;
                        colref.set_value(row_idx, value);
                    });
            }
            ++row_idx;
        }
        m_loader.truncate(row_idx);
        m_current_rows = row_idx;
    }
    auto info() -> std::string {
        std::stringstream ss;
        ss << fmt::format("ECSVTable n_cols={} n_rows={}\n", this->cols(),
                          this->rows());
        ss << "Data Containers:\n";
        // make a list of column names by type
        std::apply(
            [&ss](auto &...array_data) {
                auto get_array_data_info = [&ss](const auto &array_data) {
                    using array_data_t = TULA_DECAY(array_data);
                    auto n_cols = array_data.size();
                    if (n_cols > 0) {
                        ss << fmt::format(
                            "{:>10s}: n_cols={} {}\n",
                            dtype_str<typename array_data_t::value_t>(),
                            array_data.size(), array_data.colnames());
                    }
                };
                (..., get_array_data_info(array_data));
            },
            m_data);
        return ss.str();
    }

private:
    ECSVHeader m_hdr;
    table_data_t m_data;
    loader_t m_loader;
    std::size_t m_current_rows{0};
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

template <typename... Ts>
struct formatter<tula::ecsv::ECSVDataLoader<Ts...>>
    : tula::fmt_utils::nullspec_formatter_base {
    template <typename FormatContext>
    auto format(const tula::ecsv::ECSVDataLoader<Ts...> &loader,
                FormatContext &ctx) {
        auto it = ctx.out();
        return format_to(it, "ECSVDataLoader(n_cols={})",
                         loader.get_ref_index().size());
    }
};

template <>
struct formatter<tula::ecsv::ECSVTable>
    : tula::fmt_utils::nullspec_formatter_base {
    template <typename FormatContext>
    auto format(const tula::ecsv::ECSVTable &tbl, FormatContext &ctx) {
        auto it = ctx.out();
        return format_to(it, "ECSVTable(n_cols={})", tbl.header().size());
    }
};
} // namespace fmt
