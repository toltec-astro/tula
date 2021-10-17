#pragma once
#include "../formatter/container.h"
#include "../logging.h"
#include "../meta.h"
#include <map>
#include <sstream>
#include <type_traits>

namespace tula::config {

namespace internal {

using flatconfig_value_t =
    std::variant<std::monostate, bool, int, double, std::string>;

} // namespace internal

template <typename T>
concept FlatConfigDataType =
    tula::meta::IsOneOf<T, internal::flatconfig_value_t>;

/**
 * @class FlatConfig
 * @brief A simple config class with one level.
 * @ingroup config
 */
class FlatConfig {

public:
    using key_t = std::string;
    using value_t = internal::flatconfig_value_t;
    using storage_t = std::map<key_t, value_t>;
    using undef_t = std::monostate;
    constexpr static auto undef = undef_t{};

    FlatConfig() = default;
    explicit FlatConfig(storage_t config) : m_config{std::move(config)} {}
    FlatConfig(const std::initializer_list<storage_t::value_type> &config)
        : FlatConfig{storage_t{config}} {}

    [[nodiscard]] inline auto has(const key_t &k) const noexcept -> bool {
        return this->m_config.find(k) != m_config.cend();
    }

    [[nodiscard]] inline auto is_set(const key_t &k) const noexcept -> bool {
        return has(k) && !std::holds_alternative<std::monostate>(this->at(k));
    }
    template <typename F>
    inline auto try_call_with(const key_t &k, F &&f) const {
        SPDLOG_TRACE("try call with config key={} value={}", k, this->at(k));
        bool called = false;
        auto fn = [&](auto &&v) {
            if constexpr (std::is_invocable_v<F, std::decay_t<decltype(v)>>) {
                SPDLOG_TRACE("f({}={}) called", k, this->at(k));
                called = true;
                return std::invoke(FWD(f), FWD(v));
            }
        };
        using RT =
            std::invoke_result_t<decltype(std::visit<decltype(fn), value_t>),
                                 decltype(fn), value_t>;
        if constexpr (std::is_void_v<RT>) {
            std::visit(fn, this->at(k));
            if (!called) {
                SPDLOG_TRACE("f({}) not called", k);
            }
            return called;
        } else {
            auto result = std::optional(std::visit(fn, this->at(k)));
            if (called) {
                return result;
            }
            SPDLOG_TRACE("f({}) not called", k);
            return std::nullopt;
        }
    }
    template <typename F>
    inline auto try_call_if(const key_t &k, F &&f) const {
        SPDLOG_TRACE("try call if config key={} value={}", k, this->at(k));
        using RT = std::invoke_result_t<F>;
        if constexpr (std::is_void_v<RT>) {
            if (!has(k) || !std::holds_alternative<bool>(this->at(k)) ||
                !get_typed<bool>(k)) {
                SPDLOG_TRACE("f({}) not called", k);
                return false;
            }
            SPDLOG_TRACE("f({}={}) called", k, this->at(k));
            std::forward<F>(f)();
            return true;
        } else {
            if (!has(k) || !std::holds_alternative<bool>(this->at(k)) ||
                !get_typed<bool>(k)) {
                SPDLOG_TRACE("f({}) not called", k);
                return std::optional<RT>{};
            }
            SPDLOG_TRACE("f({}={}) called", k, this->at(k));
            return std::forward<F>(f)();
        }
    }

    template <typename Opt>
    requires(!FlatConfigDataType<Opt>) &&
        tula::meta::is_instance<Opt, std::optional>::value &&FlatConfigDataType<
            typename Opt::value_type> [[nodiscard]] auto get_typed(const key_t
                                                                       &key)
            const -> Opt {
        decltype(auto) v = this->at(key);
        using T = typename Opt::value_type;
        SPDLOG_TRACE("get typed config key={} value={}", key, v);
        if (has(key)) {
            if (std::holds_alternative<T>(v)) {
                return get_typed<T>(key);
            }
            if (std::holds_alternative<std::monostate>(v)) {
                return std::nullopt;
            }
            throw std::runtime_error(
                fmt::format("wrong type for config key={} value={}", key, v));
        }
        return std::nullopt;
    }

    template <FlatConfigDataType T>
    [[nodiscard]] auto get_typed(const key_t &key) const -> const T & {
        decltype(auto) v = this->at(key);
        SPDLOG_TRACE("get typed config key={} value={}", key, v);
        return std::get<T>(v);
    }
    template <FlatConfigDataType T>
    [[nodiscard]] auto get_typed(const key_t &key) -> T & {
        decltype(auto) v = this->at(key);
        SPDLOG_TRACE("get typed config key={} value={}", key, v);
        return std::get<T>(v);
    }

    template <typename T>
    auto get_typed(const key_t &key, T &&defval) const noexcept -> T {
        if (has(key)) {
            return get_typed<T>(key);
        }
        return std::forward<T>(defval);
    }

    template <typename T>
    auto get_lexical(const key_t &key) const -> T {
        decltype(auto) v = this->at(key);
        if (std::holds_alternative<std::monostate>(v)) {
            throw std::bad_variant_access();
        }
        std::stringstream ss;
        std::visit(
            [&ss](auto &&arg) {
                if constexpr (!std::is_same_v<std::monostate,
                                              std::decay_t<decltype(arg)>>) {
                    ss << arg;
                }
            },
            v);
        T out;
        ss >> out;
        SPDLOG_TRACE("get lexical config key={} value={} got={}", key, v, out);
        return out;
    }
    template <typename... Args>
    auto get_str(const key_t &key, Args &&...args) const -> std::string {
        return get_lexical<std::string>(key,
                                        std::forward<decltype(args)>(args)...);
    }

    template <typename T>
    void set(const key_t &key, T &&v) noexcept {
        using V = std::decay_t<T>;
        if constexpr (std::is_same_v<V, value_t>) {
            at_or_add(key) = std::forward<T>(v);
        } else if constexpr (tula::meta::CStr<V> ||
                             std::is_same_v<V, std::string_view>) {
            at_or_add(key) =
                value_t{std::in_place_type<std::string>, std::forward<T>(v)};
        } else {
            at_or_add(key) = value_t{std::in_place_type<V>, std::forward<T>(v)};
        }
    }

    [[nodiscard]] auto pformat() const -> std::string {
        if (this->m_config.empty()) {
            return "{}";
        }
        // compute width
        std::size_t key_width = 0;
        auto it = std::max_element(this->m_config.begin(), this->m_config.end(),
                                   [](const auto &a, const auto &b) {
                                       return a.first.size() < b.first.size();
                                   });
        if (it != this->m_config.end()) {
            key_width = it->first.size();
        }
        std::stringstream ss;
        ss << "{";
        for (const auto &p : this->m_config) {
            auto fmt_str = fmt::format("\n {{:>{}s}}: {{}}", key_width);
            ss << fmt::format(fmt::runtime(fmt_str), p.first, p.second);
        }
        ss << "\n}";
        return ss.str();
    }

    [[nodiscard]] auto at(const key_t &key) const -> const value_t & {
        try {
            return this->m_config.at(key);
        } catch (const std::out_of_range &) {
            throw std::out_of_range(fmt::format(
                "invalid key: \"{}\" in config {}", key, pformat()));
        }
    }
    [[nodiscard]] inline auto at(const key_t &key) -> value_t & {
        return const_cast<value_t &>(
            const_cast<const FlatConfig *>(this)->at(key));
    }

    template <typename value_t_in = value_t>
    auto at_or_add(const std::string &key) noexcept -> value_t & {
        if (!has(key)) {
            this->m_config[key] = value_t_in{};
            SPDLOG_TRACE("add config key={}", key);
        }
        return this->at(key);
    }

    auto update(FlatConfig other) noexcept -> FlatConfig & {
        other.m_config.merge(std::move(m_config));
        m_config = std::move(other.m_config);
        return *this;
    }

private:
    storage_t m_config{};
};

} // namespace tula::config
