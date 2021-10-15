#pragma once

#include "enum.h"
#include "logging.h"
#include "meta.h"
#include <clipp.h>
#include <utility>
#include <variant>

namespace tula::cli {

struct screen {
    std::string prog{"prog"};
    std::string name;
    std::string desc;
    std::string vers;
    screen(std::string prog_, std::string name_, std::string vers_,
           std::string desc_)
        : prog(std::move(prog_)), name(std::move(name_)),
          vers(std::move(vers_)), desc(std::move(desc_)) {}
    constexpr static auto indent = 2;
    constexpr static auto doc_column = 24;
    constexpr static auto last_column = 80;
    clipp::doc_formatting docfmt = clipp::doc_formatting{}
                                       .first_column(indent)
                                       .doc_column(doc_column)
                                       .last_column(last_column)
                                       .indent_size(indent)
                                       .flag_separator(",")
                                       .empty_label("arg");
    template <typename Cli, typename... Func>
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    void parse(Cli &&cli, int argc, char *argv[], Func &&...func) {
        if (auto res = clipp::parse(argc, argv, cli); res) {
            (FWD(func)(std::forward<Cli>(cli), std::move(res)), ...);
        } else {
            usage(cli);
            error(res);
            std::exit(EXIT_FAILURE);
        }
    }

    template <typename Res>
    void error(const Res &res) {
        if (res.any_error()) {
            fmt::print("{}: error parse arguments:\n", prog);
        }
        std::stringstream ss;
        clipp::debug::print(ss, res);
        fmt::print("{}", ss.str());
    }
    void description() { fmt::print("{}: {}\n", name, desc); }

    void version() { fmt::print("{}\n", vers); }

    template <typename Cli>
    void usage(const Cli &cli) {
        std::string heading = "usage: ";
        auto hs = heading.size();
        auto fmt =
            clipp::doc_formatting{docfmt}.first_column(static_cast<int>(hs));
        auto usage_str = clipp::usage_lines(cli, prog, fmt)
                             .str()
                             .substr(hs - 1, std::string::npos);
        fmt::print("usage: {}\n", usage_str);
    }

    template <typename Cli>
    void help(const Cli &cli) {
        usage(cli);
        fmt::print("\n{}\n", clipp::documentation(cli, docfmt).str());
    }
    template <typename Cli>
    void manpage(const Cli &cli) {
        auto indstr = std::string(static_cast<std::size_t>(indent), ' ');
        std::stringstream ss;
        ss << clipp::make_man_page(cli, prog, docfmt)
                  //               .prepend_section(
                  //                   "DESCRIPTION", fmt::format("{}{}",
                  //                   indstr, desc))
                  .prepend_section(
                      "NAME", fmt::format("{}{} - {}", indstr, name, desc));
        fmt::print("{}\n", ss.str());
    }
};

namespace clipp_builder {

inline const auto undefstr = std::string("<undef>");
using undef = std::monostate;

namespace internal {

// factory function to create param with option keys deduced automatically
template <typename Param, typename Doc, typename Setter>
auto arg(Param &&param, Doc &&doc, Setter &&setter) {
    auto labels = param.flags();
    std::transform(
        labels.begin(), labels.end(), labels.begin(),
        [](const auto &s) { return s.substr(s.find_first_not_of('-')); });
    std::sort(labels.begin(), labels.end(),
              [](const auto &l, const auto &r) { return l.size() < r.size(); });
    // make longest flag the key
    return std::forward<Setter>(setter)(
        labels.back(), std::forward<Param>(param), std::forward<Doc>(doc));
};

template <typename value_t, bool is_optional_, typename key_t, typename func_t>
struct valspec_t {
    valspec_t(key_t key_, func_t func_)
        : key{std::move(key_)}, func{std::move(func_)} {}
    key_t key;
    func_t func;
    template <typename Conf, typename ckey_t>
    auto make_value(Conf &conf, const ckey_t &ckey) const {
        if constexpr (std::is_constructible_v<std::string, key_t>) {
            // single value with clipp value factory
            return func(key).call([&conf, ckey = ckey](auto &&value) {
                value_t v{};
                clipp::set(v)(std::forward<decltype(value)>(value));
                conf.set(ckey, v);
            });
        } else {
            // func is value factory
            return func(key, conf.template get_typed<value_t>(ckey));
        }
    }
    static constexpr auto is_optional = is_optional_;
};

template <typename value_t, bool is_optional, typename... Args>
auto make_valspec(Args &&...args) {
    return valspec_t<value_t, is_optional, Args...>(
        std::forward<decltype(args)>(args)...);
}

// normalize value
// const char*, enum -> string
template <typename T>
auto normalize_value(T &&v) {
    using V = std::decay_t<T>;
    if constexpr (std::is_enum_v<V>) {
        return std::string(enum_utils::name(v));
    } else if constexpr (std::is_constructible_v<std::string, V> ||
                         std::is_same_v<std::string_view, V>) {
        return std::string(std::forward<T>(v));
    } else {
        return std::forward<T>(v);
    }
};

} // namespace internal

template <typename ConfigType>
struct ConfigMapper {
    using config_t = ConfigType;
    using value_t = typename config_t::value_t;
    ConfigMapper() = default;
    ConfigMapper(config_t config) : m_config{std::move(config)} {}
    auto config() const &noexcept -> const config_t & { return m_config; }
    auto config() &noexcept -> config_t & { return m_config; }
    auto config() &&noexcept -> config_t && { return std::move(m_config); }

    /// @brief Boolean flag.
    template <typename Param, typename Doc>
    auto operator()(Param &&param, Doc &&doc) {
        return internal::arg(
            std::forward<Param>(param), std::forward<Doc>(doc),
            [this](const auto &key, auto &&param, auto &&doc) {
                // initialize as false
                decltype(auto) c = this->config();
                c.set(key, false);
                return std::forward<decltype(param)>(param).set(
                           c.template get_typed<bool>(key)) %
                       doc;
            });
    }

    /// @brief Positional argument according to valspec
    template <typename Doc, typename VSpec>
    auto operator()(std::string key, Doc &&doc, VSpec &&valspec) {
        decltype(auto) c = this->config();
        c.set(key, undef{});
        return valspec.make_value(c, std::move(key)) % std::forward<Doc>(doc);
    };

    /// @brief Option with value, according to valspec
    template <typename Param, typename Doc, typename V, typename VSpec>
    auto operator()(Param &&param, Doc &&doc, V &&defval, VSpec &&valspec) {
        return internal::arg(
            std::forward<Param>(param), std::forward<Doc>(doc),
            [this, defval_ = std::forward<V>(defval),
             valspec = std::forward<VSpec>(valspec)](const auto &key,
                                                     auto param, auto &&doc_) {
                // normalize value to value type
                value_t defval{internal::normalize_value(defval_)};
                std::string doc{std::forward<Doc>(doc_)};
                decltype(auto) c = this->config();
                // setup option param
                if constexpr (VSpec::is_optional) {
                    // initialize to undef if value is optional
                    // and set to default var in option callback
                    c.set(key, undef{});
                    param = param.call(
                        [this, key = key, defvar = defval]() mutable {
                            this->config().set(key, std::move(defvar));
                        });
                } else {
                    // initialize to default value directly
                    c.set(key, defval);
                }
                // append the doc with default value if defined
                if constexpr (!std::is_same_v<V, undef>) {
                    doc = fmt::format("{}. Default is {}", doc, defval);
                }
                return (param & valspec.make_value(c, key)) % doc;
            });
    }

private:
    config_t m_config;
};

// the non-human-readable cli builder symbols:
//       _             : entry(config, param, doc, defval, valspec)
//       __            : group(entries...)
//       p             : param(labels...)
// xxx, opt_xxx, list  : valspec

template <typename... Args>
auto g(Args &&...args) {
    return clipp::group("", std::forward<decltype(args)>(args)...);
};

template <typename... Args>
auto p(Args &&...args) {
    return clipp::with_prefixes_short_long(
        "-", "--", clipp::option(std::forward<decltype(args)>(args)...));
};

// valspecs
inline constexpr auto str = [](const std::string &k = "arg") {
    return internal::make_valspec<std::string, false>(k,
                                                      TULA_LIFT(clipp::value));
};
inline constexpr auto opt_str = [](const std::string &k = "arg") {
    return internal::make_valspec<std::string, true>(
        k, TULA_LIFT(clipp::opt_value));
};

inline constexpr auto int_ = [](const std::string &k = "num") {
    return internal::make_valspec<int, false>(k, TULA_LIFT(clipp::integer));
};

inline constexpr auto opt_int = [](const std::string &k = "num") {
    return internal::make_valspec<int, true>(k, TULA_LIFT(clipp::opt_integer));
};
inline constexpr auto doub = [](const std::string &k = "value") {
    return internal::make_valspec<double, false>(k, TULA_LIFT(clipp::number));
};

inline constexpr auto list = [](auto &&choices_) {
    using T = std::decay_t<decltype(choices_)>;
    auto choices = [](auto &&c) {
        if constexpr (std::is_enum_v<T>) {
            return enum_utils::names<T>();
        } else {
            return std::forward<decltype(c)>(c);
        }
    }(std::forward<decltype(choices_)>(choices_));
    return internal::make_valspec<std::string, false>(
        std::move(choices), [](auto &&choices, auto &val) {
            clipp::group g;
            for (auto &x : choices) {
                std::ostringstream oss;
                oss << x;
                g.push_back(clipp::command(oss.str()).call(
                    [x = x, &val]() mutable { val = x; }));
            }
            g.exclusive(true);
            return g;
        });
};

} // namespace clipp_builder
} // namespace tula::cli
