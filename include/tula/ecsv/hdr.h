#pragma once

#include "../formatter/utils.h"
#include "core.h"
#include <fmt/core.h>
#include <ranges>

namespace tula::ecsv {

namespace internal {

template <typename T, typename key_t>
auto get_optional_from_node(const YAML::Node &node, key_t &&key,
                            std::optional<T> default_value = std::nullopt)
    -> std::optional<T> {
    decltype(auto) v = node[std::forward<key_t>(key)];
    if (v.IsDefined()) {
        return v.template as<T>();
    }
    return default_value;
};

} // namespace internal

/**
 * @brief The class to hold ECSV column info.
 */
struct ECSVColumn {
    // https://github.com/astropy/astropy-APEs/blob/main/APE6.rst
    std::string name{};
    std::string datatype{};
    std::optional<std::string> subtype{};
    std::optional<std::string> unit{};
    std::optional<std::string> format{};
    std::optional<std::string> description{};
    YAML::Node meta{};
};

/**
 * @brief The class to hold info in ECSV haeder.
 */
struct ECSVHeader {

    /// @brief Create ECSV header from YAML node.
    static auto
    from_node(const YAML::Node &ecsv_header,
              std::optional<std::string> csv_header = std::nullopt) {
        // check required items in node
        if (!ecsv_header[spec::k_datatype.data()]) {
            throw ParseError("Missing datatype in header YAML");
        }
        // optional items in node
        YAML::Node meta{};
        if (decltype(auto) n = ecsv_header[spec::k_meta.data()]; n) {
            meta.reset(n);
        }
        auto delimiter =
            internal::get_optional_from_node<char>(
                ecsv_header, spec::k_delimiter.data(), spec::ECSV_DELIM_CHAR)
                .value();
        auto schema = internal::get_optional_from_node<std::string>(
            ecsv_header, spec ::k_schema.data());
        auto spec_version = internal::get_optional_from_node<std::string>(
            ecsv_header, "_ecsv_spec_version");
        // collect columns
        std::vector<ECSVColumn> cols;
        for (const auto &n : ecsv_header[spec::k_datatype.data()]) {
            cols.push_back({n[spec::k_name.data()].as<std::string>(),
                            n[spec::k_datatype.data()].as<std::string>(),
                            internal::get_optional_from_node<std::string>(
                                n, spec::k_subtype.data()),
                            internal::get_optional_from_node<std::string>(
                                n, spec::k_unit.data()),
                            internal::get_optional_from_node<std::string>(
                                n, spec::k_format.data()),
                            internal::get_optional_from_node<std::string>(
                                n, spec::k_description.data())});
        }
        // SPDLOG_INFO("cols:{}", cols);
        // when csv_header is provided, we can check the csv header for
        // consistency
        if (csv_header.has_value()) {
            // break the line to get colnames
            std::vector<std::string> csv_colnames{};
            std::string colname{};
            for (char &it : csv_header.value()) {
                if (delimiter != it) {
                    // not a delim, so append to colname
                    colname += it;
                    continue;
                }
                // found delim
                if (colname.empty()) {
                    // keep finding if nothing in colname
                    continue;
                }
                // got something in colname
                csv_colnames.push_back(colname);
                colname.clear(); // reset for the next
            }
            // get anything left in colname
            if (!colname.empty()) {
                csv_colnames.push_back(colname);
            }
            // check the size of colnames with ecsv header cols
            if (csv_colnames.size() != cols.size()) {
                throw ParseError(
                    fmt::format("Mismatch number of columns in YAML header "
                                "({}) and the CSV header ({}).",
                                cols.size(), csv_colnames.size()));
            }
            for (std::size_t i = 0; i < csv_colnames.size(); ++i) {
                if (csv_colnames[i] != cols[i].name) {
                    throw ParseError(
                        fmt::format("Mismatch column name at index {} in YAML "
                                    "header ({}) and the CSV header ({}).",
                                    i, cols[i].name, csv_colnames[i]));
                }
            }
        }
        // initialize and return
        return ECSVHeader(std::move(cols), meta, delimiter, std::move(schema),
                          std::move(spec_version));
    }

    /// @brief Create ECSV header from stream
    template <typename IStream>
    static auto read(IStream &is, std::vector<std::string> *lines = nullptr) {
        return std::apply(ECSVHeader::from_node, parse_header(is, lines));
    }

    /// @brief Create ECSV header from parts.
    ECSVHeader(std::vector<ECSVColumn> cols, const YAML::Node &meta,
               char delimiter, std::optional<std::string> schema,
               std::optional<std::string> spec_version)
        : m_cols{std::move(cols)}, m_meta{meta},
          m_delimiter{delimiter}, m_schema{std::move(schema)},
          m_spec_version{std::move(spec_version)
                             .value_or(std::string(spec::ECSV_VERSION))} {}

    auto cols() const noexcept -> const auto & { return this->m_cols; }
    auto meta() const noexcept -> const auto & { return this->m_meta; }
    auto delimiter() const noexcept -> const auto & {
        return this->m_delimiter;
    }
    auto schema() const noexcept -> const auto & { return this->m_schema; }
    auto spec_version() const noexcept -> const auto & {
        return this->m_spec_version;
    }

    auto colnames() const noexcept -> decltype(auto) {
        return std::ranges::transform_view(m_cols, &ECSVColumn::name);
    }

    auto datatypes() const noexcept -> decltype(auto) {
        return std::ranges::transform_view(m_cols, &ECSVColumn::datatype);
    }

    auto size() const noexcept { return cols().size(); }

private:
    std::vector<ECSVColumn> m_cols{};
    YAML::Node m_meta{};
    char m_delimiter{spec::ECSV_DELIM_CHAR};
    std::optional<std::string> m_schema{};
    std::string m_spec_version{spec::ECSV_VERSION};
};

} // namespace tula::ecsv

// formatter support
namespace fmt {
template <>
struct formatter<tula::ecsv::ECSVColumn>
    : tula::fmt_utils::charspec_formatter_base<'s', 'l'> {
    // s: short form as <name>(<datatype>)
    // l: long form with full info
    template <typename FormatContext>
    auto format(const tula::ecsv::ECSVColumn &col, FormatContext &ctx) {
        auto it = ctx.out();
        auto spec = spec_handler();
        switch (spec) {
        case 's': {
            return format_to(it, "{}({})", col.name, col.datatype);
        }
        case 'l': {
            return format_to(
                it, "ECSVColumn(name={}, dtype={}, unit={}, description={})",
                col.name, col.datatype, col.unit, col.description);
        }
        }
        return it;
    }
};
template <>
struct formatter<tula::ecsv::ECSVHeader>
    : tula::fmt_utils::nullspec_formatter_base {
    template <typename FormatContext>
    auto format(const tula::ecsv::ECSVHeader &hdr, FormatContext &ctx) {
        auto it = ctx.out();
        return format_to(it, "ECSVHeader(ncols={})", hdr.cols().size());
    }
};
} // namespace fmt
