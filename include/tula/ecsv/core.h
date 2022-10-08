#pragma once

#include "../meta.h"
#include <algorithm>
#include <cassert>
#include <complex>
#include <map>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
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
                if (vt value{}; YAML::convert<vt>::decode(n, value)) {
                    result = value;
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
        if (T value{}; convert<T>::decode(node, value)) {
            rhs = std::move(value);
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
auto startswith(const T &value, const U &prefix) -> bool {
    if (prefix.size() > value.size()) {
        return false;
    }
    return std::equal(prefix.begin(), prefix.end(), value.begin());
}

} // namespace internal

namespace spec {
inline namespace v0 {
static constexpr std::string_view ECSV_VERSION = "0.9";
static constexpr char ECSV_DELIM_CHAR = ' ';
static constexpr std::string_view ECSV_HEADER_PREFIX = "# ";
static constexpr std::string_view ECSV_VERSION_LINE_REGEX = "^# %ECSV (.+)";
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

/// @brief Read stream line by line and parse as ECSV header
/// @param is Input stream to process.
/// @param lines Optional output to capture the read lines.
/// It returns the tuple of the parsed ECSV YAML header node and
/// The CSV header line.
template <typename IStream>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto parse_header(IStream &is, std::vector<std::string> *lines = nullptr) {
    std::regex re_ecsv_version{std::string{spec::ECSV_VERSION_LINE_REGEX}};

    std::stringstream ss_header; // stream to hold all header info.

    std::size_t li = 0; // running line number
    std::string ln{};   // running line content
    std::string ecsv_spec_version{};
    std::optional<std::string>
        csv_header{}; // csv header, immediately after the ECSV header
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
        if (li == 0) {
            std::smatch sm;
            if (std::regex_match(ln, sm, re_ecsv_version)) {
                // update the version number
                ecsv_spec_version = sm[1];
            } else {
                throw ParseError("no ECSV version line found");
            }
            ++li;
            continue;
        }
        // l > 0
        if (ln == "#") {
            // this is an empty line, just ignore
            ++li;
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
            ++li;
            continue;
        }
        // else
        // this line is not the yaml header and is expect to be the csv header
        {
            csv_header = ln;
            ++li;
            break;
        }
    }
    assert(lines ? (li == lines->size()) : true); // sanity check
    // Create yaml node
    auto node = YAML::Load(ss_header);
    // we update some additional info in to the node for internal usage
    node["_ecsv_spec_version"] = ecsv_spec_version;
    return std::tuple{node, csv_header};
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
    } else if constexpr (std::is_same_v<T, std::string>) {
        return "string";
    } else {
        static_assert(tula::meta::always_false<T>,
                      "TULA ECSV IS NOT IMPLEMENTED FOR THIS SCALAR TYPE");
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
    YAML::Node node{};
    node.SetStyle(YAML::EmitterStyle::Flow);
    std::string k_name{spec::k_name};
    std::string k_datatype{spec::k_datatype};
    node[k_name] = name;
    node[k_datatype] = dtype_str<T>();
    return node;
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
        for (const auto &colname : colnames) {
            header[k_datatype].push_back(make_column_node<T>(colname));
        }
    } else {
        // types are differnet
        tula::meta::apply_const_sequence(
            [&](auto &&index_) {
                constexpr auto index = std::decay_t<decltype(index_)>::value;
                // data type
                using U = std::tuple_element_t<index, std::tuple<T, Ts...>>;
                header[k_datatype].push_back(
                    (make_column_node<U>(colnames[index])));
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
    YAML::Node node;
    using key_t = typename std::decay_t<Map>::key_type;
    using value_t = typename std::decay_t<Map>::mapped_type;
    for (const auto &item : std::forward<Map>(map)) {
        // handle value types
        if constexpr (tula::meta::is_instance<value_t, std::variant>::value) {
            std::visit(
                [&](const auto &value) {
                    using vt = std::decay_t<decltype(value)>;
                    if constexpr (std::is_same_v<vt, std::monostate>) {
                        node.push_back(YAML::Null);
                    } else {
                        node.push_back(std::map{std::pair{item.first, value}});
                    };
                },
                item.second);
        } else {
            node.push_back(std::map{std::pair{item.first, item.second}});
        }
    };
    node.SetTag(std::string{spec::ECSV_META_TAG});
    return node;
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
        for (const auto &value : meta) {
            assert(value.IsMap() && value.size() == 1);
            const auto it = value.begin();
            if (auto opt_item = decode_item(*it); opt_item) {
                result[opt_item.value().first] = opt_item.value().second;
            } else if (rest != nullptr) {
                rest_.push_back(value);
            }
        }
    } else if (meta.IsMap()) {
        for (const auto &value : meta) {
            if (auto opt_item = decode_item(value); opt_item) {
                result[opt_item.value().first] = opt_item.value().second;
            } else if (rest != nullptr) {
                rest_.operator[](value.first) = value.second;
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
