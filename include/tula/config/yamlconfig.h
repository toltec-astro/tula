#pragma once
#include "../filesystem.h"
#include "../formatter/container.h"
#include "../logging.h"
#include "../meta.h"
#include "tula/traits.h"
#include <tuple>
#include <type_traits>
#include <yaml-cpp/yaml.h>

namespace tula::config {

namespace internal {

template <typename U>
concept YamlNodeKeyType = requires(U &&u) {
    YAML::Node()[std::forward<U>(u)];
};

// https://stackoverflow.com/a/41337824/1824372
inline const YAML::Node &cnode(const YAML::Node &n) { return n; }

inline YAML::Node merge_nodes(YAML::Node a, YAML::Node b) {
    if (!b.IsMap()) {
        // If b is not a map, merge result is b, unless b is null
        return b.IsNull() ? a : b;
    }
    if (!a.IsMap()) {
        // If a is not a map, merge result is b
        return b;
    }
    if (!b.size()) {
        // If a is a map, and b is an empty map, return a
        return a;
    }
    // Create a new map 'c' with the same mappings as a, merged with b
    auto c = YAML::Node(YAML::NodeType::Map);
    for (auto n : a) {
        if (n.first.IsScalar()) {
            const std::string &key = n.first.Scalar();
            auto t = YAML::Node(cnode(b)[key]);
            if (t) {
                c[n.first] = merge_nodes(n.second, t);
                continue;
            }
        }
        c[n.first] = n.second;
    }
    // Add the mappings from 'b' not already in 'c'
    for (auto n : b) {
        if (!n.first.IsScalar() || !cnode(c)[n.first.Scalar()]) {
            c[n.first] = n.second;
        }
    }
    return c;
}

} // namespace internal

/**
 * @brief The YamlConfig struct
 * This is a thin wrapper around YAML::Node.
 */
struct YamlConfig {

    inline static const auto undef{YAML::Node(YAML::NodeType::Undefined)};

    template <typename U>
    using is_valid_key_t = std::conditional_t<internal::YamlNodeKeyType<U>,
                                              std::true_type, std::false_type>;

    using value_t = YAML::Node;
    using storage_t = YAML::Node;
    YamlConfig() = default;
    explicit YamlConfig(const storage_t &node) : m_node(node) {}
    template <typename Path>
    explicit YamlConfig(const storage_t &node, Path filepath)
        : m_node(node), m_filepath{std::move(filepath)} {}

    template <typename... Keys>
    auto get_node(Keys &&...keys) -> decltype(auto) {
        return _get_node_impl(*this, std::forward<Keys>(keys)...);
    }
    template <typename... Keys>
    auto get_node(Keys &&...keys) const -> decltype(auto) {
        return _get_node_impl(*this, std::forward<Keys>(keys)...);
    }

    template <typename key_t, typename value_t>
    void set(key_t &&key, value_t &&value) {
        get_node(std::forward<decltype(key)>(key)) =
            std::forward<decltype(value)>(value);
    }

    template <typename key_t, typename value_t>
    void append(key_t &&key, value_t &&value) {
        get_node(std::forward<decltype(key)>(key))
            .push_back(std::forward<decltype(value)>(value));
    }

    template <typename key_t>
    auto has(key_t &&key) const -> bool {
        bool node_defined;
        try {
            decltype(auto) node = get_node(std::forward<key_t>(key));
            node_defined = node.IsDefined();
        } catch (const YAML::InvalidNode &) {
            node_defined = false;
        }
        return node_defined;
]    }

    template <typename key_t>
    auto has_list(key_t &&key) const -> bool {
        decltype(auto) node = get_node(std::forward<key_t>(key));
        return node.IsDefined() && node.IsSequence();
    }

    template <typename T, typename key_t>
    auto has_typed(key_t &&key) const -> bool {
        if (!has(key)) {
            return false;
        }
        try {
            get_typed<T>(key);
        } catch (const YAML::BadConversion &) {
            return false;
        }
        return true;
    }

    template <typename T, typename key_t>
    auto get_typed(key_t &&key) const {
        return get_node(std::forward<key_t>(key)).template as<T>();
    }
    template <typename T, typename key_t>
    auto get_typed(key_t &&key, T &&defval) const {
        decltype(auto) node = get_node(std::forward<key_t>(key));
        if (node.IsDefined() && !node.IsNull()) {
            return node.template as<T>();
        }
        return std::forward<T>(defval);
    }

    template <typename... Args>
    auto get_str(Args &&...args) const {
        return get_typed<std::string>(std::forward<decltype(args)>(args)...);
    }

    template <typename... Args>
    auto get_filepath(Args &&...args) const {
        decltype(auto) s =
            get_typed<std::string>(std::forward<decltype(args)>(args)...);
        if (m_filepath.has_value()) {
            return (m_filepath.value().parent_path() / s).string();
        }
        return s;
    }

    template <typename key_t>
    auto get_config(key_t &&key) {
        return YamlConfig{get_node(std::forward<key_t>(key)), this->filepath()};
    }

    auto filepath() const -> const std::optional<std::filesystem::path> & {
        return m_filepath;
    }

    ///@brief Create instance from Yaml filepath.
    static auto from_filepath(const std::string &filepath) {
        return YamlConfig(YAML::LoadFile(filepath), filepath);
    }
    ///@brief Create instance from Yaml string.
    static auto from_str(const std::string &s) {
        return YamlConfig(YAML::Load(s));
    }

    auto to_str() const -> std::string {
        return fmt::format("{}", YAML::Dump(m_node));
    }
    auto pformat() const -> std::string { return to_str(); }

    template <typename OStream>
    friend auto operator<<(OStream &os, const YamlConfig &config)
        -> decltype(auto) {
        auto pformat_opt_filepath =
            [](auto &&opt_filepath) -> std::optional<std::string> {
            if (!opt_filepath.has_value()) {
                return std::nullopt;
            }
            return opt_filepath.value().string();
        };
        return os << fmt::format("{}\n<config filepath: {}>", config.pformat(),
                                 pformat_opt_filepath(config.filepath()));
    }
    friend YamlConfig merge(const YamlConfig &left, const YamlConfig &right);

private:
    storage_t m_node{};
    std::optional<std::filesystem::path> m_filepath{std::nullopt};

    static auto _multiget_node_impl(auto &&node, auto &&k, auto &&...rest)
        -> decltype(auto) {
        decltype(auto) r =
            std::forward<decltype(node)>(node)[std::forward<decltype(k)>(k)];
        if constexpr (sizeof...(rest) == 0) {
            return r;
        } else {
            return _multiget_node_impl(r,
                                       std::forward<decltype(rest)>(rest)...);
        }
    }

    template <typename Self, typename... Keys>
    static auto _get_node_impl(Self &self, Keys &&...keys) -> decltype(auto) {
        if constexpr (sizeof...(keys) == 0) {
            return self.m_node;
        } else {
            return std::apply(
                [&self](auto &&...args) mutable {
                    return _multiget_node_impl(
                        self.m_node, std::forward<decltype(args)>(args)...);
                },
                tula::meta::flatten(std::forward<Keys>(keys)...));
        }
    }
};

inline YamlConfig merge(const YamlConfig &left, const YamlConfig &right) {
    return YamlConfig(internal::merge_nodes(left.m_node, right.m_node),
                      left.filepath());
}

} // namespace tula::config

namespace fmt {

template <>
struct formatter<tula::config::YamlConfig>
    : tula::fmt_utils::nullspec_formatter_base {
    template <typename FormatContext>
    auto format(const tula::config::YamlConfig &config, FormatContext &ctx) {
        auto it = ctx.out();
        auto pformat_opt_filepath =
            [](auto &&opt_filepath) -> std::optional<std::string> {
            if (!opt_filepath.has_value()) {
                return std::nullopt;
            }
            return opt_filepath.value().string();
        };
        return format_to(it, "{}\n<config filepath: {}>", config.pformat(),
                         pformat_opt_filepath(config.filepath()));
    }
};

} // namespace fmt
