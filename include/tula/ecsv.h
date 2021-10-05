#pragma once

#include "formatter/container.h"
#include "formatter/utils.h"
#include "logging.h"
#include "meta.h"
#include <algorithm>
#include <cassert>
#include <complex>
#include <ranges>
#include <regex>
#include <sstream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>
#include <yaml-cpp/yaml.h>

namespace YAML {

template <typename... Ts>
struct convert<std::variant<Ts...>> {

    using variant_t = std::variant<Ts...>;

    /// @brief Recursively try convert node to variant alternative *in order*.
    /// The order of types matters here, for example,
    /// variant<string, int> will capture all values as string because
    /// in YAML/Json everything is string.
    /// for variant<int, string>, all nodes convertable to int are converted.
    template <std::size_t i = 0>
    static auto _to_variant(const YAML::Node &n) -> std::optional<variant_t> {
        if constexpr (i < std::variant_size_v<variant_t>) {
            using vt = std::variant_alternative_t<i, variant_t>;
            std::optional<vt> result{};
            if constexpr (std::is_same_v<vt, std::monostate>) {
                // capture null with this
                if (n.IsNull()) {
                    result = std::monostate{};
                }
            } else {
                if (vt v{}; YAML::convert<vt>::decode(n, v)) {
                    result = v;
                }
            }
            return result.has_value() ? std::move(result.value())
                                      : _to_variant<i + 1>(n);
        }
        return std::nullopt;
    }

    static auto decode(const Node &node, variant_t &rhs) -> bool {
        // capture null with monostate
        if (node.IsNull()) {
            if constexpr (std::bool_constant<(
                              std::is_same_v<std::monostate, Ts> ||
                              ...)>::value) {
                rhs = std::monostate{};
                return true;
            } else {
                return false;
            }
        }
        if (auto opt_result = _to_variant(node); opt_result.has_value()) {
            rhs = std::move(opt_result.value());
            return true;
        }
        return false;
    }
};

template <typename T>
struct convert<std::optional<T>> {
    using optional_t = std::optional<T>;

    /// @brief Decode YAML node to optional.
    /// Note that in YAML/Json everything is string.
    /// optional<string> will capture all node.
    static auto decode(const Node &node, optional_t &rhs) -> bool {
        if (node.IsNull()) {
            rhs = std::nullopt;
            return true;
        }
        if (T v{}; convert<T>::decode(node, v)) {
            rhs = std::move(v);
            return true;
        }
        return false;
    }
};

} // namespace YAML

namespace tula::ecsv {

namespace internal {

/// @brief Returns true if \p v ends with \p ending.
template <tula::meta::SizedIterable T, tula::meta::SizedIterable U>
auto startswith(const T &v, const U &prefix) -> bool {
    if (prefix.size() > v.size()) {
        return false;
    }
    return std::equal(prefix.begin(), prefix.end(), v.begin());
}

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

namespace spec {
inline namespace v0 {
static constexpr std::string_view ECSV_VERSION = "0.9";
static constexpr char ECSV_DELIM_CHAR = ' ';
static constexpr std::string_view ECSV_HEADER_PREFIX = "# ";
static constexpr std::string_view ECSV_VERSION_LINE_REGEX = "^# %ECSV .+";
static constexpr std::string_view ECSV_VERSION_LINE_PREFIX = "%ECSV ";
static constexpr std::string_view ECSV_META_TAG = "tag:yaml.org,2002:omap";

static constexpr std::string_view k_delimiter = "delimiter";
static constexpr std::string_view k_schema = "schema";
static constexpr std::string_view k_datatype = "datatype";
static constexpr std::string_view k_meta = "meta";
static constexpr std::string_view k_name = "name";
static constexpr std::string_view k_subtype = "subtype";
static constexpr std::string_view k_unit = "unit";
static constexpr std::string_view k_format = "format";
static constexpr std::string_view k_description = "description";

template <typename OStream>
void dump_yaml_preamble(OStream &os) {
    os << ECSV_VERSION_LINE_PREFIX << ECSV_VERSION << "\n---\n";
}

template <typename OStream>
void dump_yaml_header(OStream &os, const YAML::Node &node) {
    std::stringstream buf;
    dump_yaml_preamble(buf);
    // dump the node to tmp buff
    buf << node;
    // process each line to prepend the header prefix
    std::string ln;
    while (std::getline(buf, ln)) {
        os << spec::ECSV_HEADER_PREFIX << ln << "\n";
    }
}
} // namespace v0
} // namespace spec

/// @brief Throw when there is an error when parse ECSV.
struct ParseError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// @brief Throw when there is an error when dump ECSV.
struct DumpError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

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
        return ECSVHeader(std::move(cols), meta, delimiter, schema);
    }

    /// @brief Create ECSV header from parts.
    ECSVHeader(std::vector<ECSVColumn> cols, const YAML::Node &meta,
               char delimiter, std::optional<std::string> schema)
        : m_cols{std::move(cols)}, m_meta{meta}, m_delimiter{delimiter},
          m_schema{std::move(schema)}, m_col_index{make_col_index(m_cols)} {}

    auto cols() const noexcept -> const auto & { return this->m_cols; }
    auto meta() const noexcept -> const auto & { return this->m_meta; }
    auto delimiter() const noexcept -> const auto & {
        return this->m_delimiter;
    }
    auto schema() const noexcept -> const auto & { return this->m_schema; }

    /// @brief Return the column at index \p i.
    auto col(std::size_t i) const -> const auto & { return this->m_cols.at(i); }

    /// @brief Return the column of \p name.
    auto col(const std::string &s) const -> const auto & {
        return this->m_cols[this->m_col_index.at(s)];
    }

    auto colnames() const -> decltype(auto) {
        return std::ranges::transform_view(m_cols, &ECSVColumn::name);
    }

    auto datatypes() const -> decltype(auto) {
        return std::ranges::transform_view(m_cols, &ECSVColumn::datatype);
    }

private:
    std::vector<ECSVColumn> m_cols{};
    YAML::Node m_meta{};
    char m_delimiter{spec::ECSV_DELIM_CHAR};
    std::optional<std::string> m_schema{};

    // index columns with names;
    using col_index_t = std::unordered_map<std::string, std::size_t>;
    col_index_t m_col_index;
    static auto make_col_index(const std::vector<ECSVColumn> &cols)
        -> col_index_t {
        col_index_t col_index;
        for (std::size_t i = 0; i < cols.size(); ++i) {
            col_index.emplace(cols[i].name, i);
        }
        return col_index;
    }
};

/// @brief Read stream line by line and parse as ECSV header
/// @param is Input stream to process.
/// @param lines Optional output to capture the read lines.
template <typename IStream>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto parse_header(IStream &is, std::vector<std::string> *lines = nullptr) {
    std::regex re_ecsv_version{std::string{spec::ECSV_VERSION_LINE_REGEX}};

    std::stringstream ss_header; // stream to hold all header info.

    std::size_t l = 0;        // running line number
    std::string ln{};         // running line content
    std::string csv_header{}; // csv header, immediately after the ECSV header
    while (std::getline(is, ln)) {
        // SPDLOG_TRACE("current line: {}", ln);
        if (lines) {
            lines->push_back(ln);
        }
        // ignore any leading spaces
        ln.erase(ln.begin(), std::find_if(ln.begin(), ln.end(), [](auto ch) {
                     return !std::isspace(ch);
                 }));
        // check first line.
        if (l == 0) {
            if (std::regex_match(ln, re_ecsv_version)) {
                //
            } else {
                throw ParseError("no ECSV version line found");
            }
            ++l;
            continue;
        }
        // l > 0
        if (ln == "#") {
            // this is an empty line, just ignore
            ++l;
            continue;
        }
        if (internal::startswith(ln, spec::ECSV_HEADER_PREFIX)) {
            // this line is part of the header yaml
            // put the actual yaml part to the stream
            for (auto it = ln.begin() + spec::ECSV_HEADER_PREFIX.size();
                 it != ln.end(); ++it) {
                ss_header << *it;
            }
            ss_header << "\n";
            ++l;
            continue;
        }
        // else
        // this line is not the yaml header and is expect to be the csv header
        {
            csv_header = ln;
            ++l;
            break;
        }
    }
    assert(lines ? (l == lines->size()) : true); // sanity check
    // Create ECSV header and return
    return ECSVHeader::from_node(YAML::Load(ss_header), csv_header);
};

template <typename T>
auto dtype_str() -> std::string {
    // bool, int8, int16, int32, int64
    // uint8, uint16, uint32, uint64
    // float16, float32, float64, float128
    // complex64, complex128, complex256
    if constexpr (std::is_same_v<T, bool>) {
        return "bool";
    } else if constexpr (std::is_same_v<T, int8_t>) {
        return "int8";
    } else if constexpr (std::is_same_v<T, int16_t>) {
        return "int16";
    } else if constexpr (std::is_same_v<T, int32_t>) {
        return "int32";
    } else if constexpr (std::is_same_v<T, int64_t>) {
        return "int64";
    } else if constexpr (std::is_same_v<T, uint8_t>) {
        return "uint8";
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        return "uint16";
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        return "uint32";
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        return "uint64";
    } else if constexpr (std::is_same_v<T, float>) {
        return "float32";
    } else if constexpr (std::is_same_v<T, double>) {
        return "float64";
    } else if constexpr (std::is_same_v<T, long double>) {
        return "float128";
    } else if constexpr (std::is_same_v<T, std::complex<float>>) {
        return "complex64";
    } else if constexpr (std::is_same_v<T, std::complex<double>>) {
        return "complex128";
    } else if constexpr (std::is_same_v<T, std::complex<long double>>) {
        return "complex256";
    } else {
        static_assert(tula::meta::always_false<T>,
                      "ECSV NOT IMPLEMENTED FOR THIS SCALAR TYPE");
    }
}

/// @brief Check if the declared data types are uniform across all columns and
/// is T.
/// @tparam T The desired data type.
template <typename T, typename DataTypes>
auto check_uniform_dtype(DataTypes &&dtypes) -> bool {
    if (std::adjacent_find(dtypes.begin(), dtypes.end(),
                           std::not_equal_to<>()) == dtypes.end()) {
        // all dtypes are the same
        return dtypes.front() == dtype_str<T>();
    }
    return false;
}

/// @brief Return a YAML node representing a column.
template <typename T>
auto make_column_node(const std::string &name) {
    YAML::Node n{};
    n.SetStyle(YAML::EmitterStyle::Flow);
    std::string k_name{spec::k_name};
    std::string k_datatype{spec::k_datatype};
    n[k_name] = name;
    n[k_datatype] = dtype_str<T>();
    return n;
}

/**
 * @brief Dump ECSV header to stream.
 *
 * @tparam OStream The output stream type
 * @tparam T  The column type for all columns if \p Ts is empty.
 * @tparam Ts  When not empty, use column types as (T, Ts...). The number of
 * types must match with the size of \p colnames.
 * @param os  The output stream.
 * @param colnames The column names to use.
 * @param meta The metadata to include in the header
 */
template <typename OStream, typename T, typename... Ts>
void dump_header(OStream &os, const std::vector<std::string> &colnames,
                 const YAML::Node &meta) {
    YAML::Node header;
    header.SetStyle(YAML::EmitterStyle::Block);
    constexpr auto ntypes = sizeof...(Ts) + 1; // number of types include T
    if (ntypes > 1 && (ntypes != colnames.size())) {
        throw DumpError("mismatch number of types with colnames.");
    }
    std::string k_datatype{spec::k_datatype};
    std::string k_meta{spec::k_meta};
    if constexpr (ntypes == 1) {
        // all types are T. uniform.
        for (const auto &c : colnames) {
            header[k_datatype].push_back(make_column_node<T>(c));
        }
    } else {
        // types are differnet
        tula::meta::apply_const_sequence(
            [&](auto &&i_) {
                constexpr auto i = std::decay_t<decltype(i_)>::value;
                // data type
                using U = std::tuple_element_t<i, std::tuple<T, Ts...>>;
                header[k_datatype].push_back(
                    (make_column_node<U>(colnames[i])));
            },
            std::make_index_sequence<ntypes>{});
    }
    if (!meta.IsNull()) {
        // The YAML::Node does not support move constructor so
        // we pass by const ref to save from making an extra copy.
        // later we should use pass by value and move semantics.
        header[k_meta] = meta;
    }
    spec::dump_yaml_header(os, header);
}

/// @brief Create meta node from map-like data structure.
template <typename Map>
auto map_to_meta(Map &&map) {
    YAML::Node n;
    using key_t = typename std::decay_t<Map>::key_type;
    using value_t = typename std::decay_t<Map>::mapped_type;
    for (const auto &item : std::forward<Map>(map)) {
        // handle value types
        if constexpr (tula::meta::is_instance<value_t, std::variant>::value) {
            std::visit(
                [&](const auto &v) {
                    using vt = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<vt, std::monostate>) {
                        n.push_back(YAML::Null);
                    } else {
                        n.push_back(std::map{std::pair{item.first, v}});
                    };
                },
                item.second);
        } else {
            n.push_back(std::map{std::pair{item.first, item.second}});
        }
    };
    n.SetTag(std::string{spec::ECSV_META_TAG});
    return n;
}

/// @brief Create map from meta, if viable.
/// @param meta The meta.
/// @param rest A node to capture any uncaptured entries in meta.
/// The map key type must be int-like or string, and the mapped type can be
/// int-like, floating point, string, std::optional, or std::variant
template <typename key_t, typename value_t>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto meta_to_map(const YAML::Node &meta, YAML::Node *rest = nullptr)
    -> std::map<key_t, value_t> {

    using item_t = std::pair<key_t, value_t>;
    auto decode_item = [](const auto &n) -> std::optional<item_t> {
        item_t item{};
        if (!YAML::convert<key_t>::decode(n.first, item.first)) {
            return std::nullopt;
        }
        if (!YAML::convert<value_t>::decode(n.second, item.second)) {
            return std::nullopt;
        }
        return std::move(item);
    };
    std::map<key_t, value_t> result;
    YAML::Node rest_{}; // this is to collect any uncaptured item
    rest_.SetTag(meta.Tag());
    if (meta.IsSequence() && (meta.Tag() == spec::ECSV_META_TAG)) {
        // ordered map
        for (const auto &v : meta) {
            assert(v.IsMap() && v.size() == 1);
            const auto it = v.begin();
            if (auto opt_item = decode_item(*it); opt_item) {
                result[opt_item.value().first] = opt_item.value().second;
            } else if (rest != nullptr) {
                rest_.push_back(v);
            }
        }
    } else if (meta.IsMap()) {
        for (const auto &v : meta) {
            if (auto opt_item = decode_item(v); opt_item) {
                result[opt_item.value().first] = opt_item.value().second;
            } else if (rest != nullptr) {
                rest_.operator[](v.first) = v.second;
            }
        }
    } else {
        if (rest != nullptr) {
            rest_.reset(meta);
        }
    }
    // update to the input rest param.
    if (rest != nullptr) {
        rest->reset(rest_);
    }
    return result;
}

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
