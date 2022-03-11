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
    void parse(Cli &&cli, int argc, char *argv[], Func &&...func) const {
        if (auto res = clipp::parse(argc, argv, cli); res) {
            (std::forward<decltype(func)>(func)(std::forward<Cli>(cli),
                                                std::move(res)),
             ...);
        } else {
            usage(cli);
            error(res);
            std::exit(EXIT_FAILURE);
        }
    }

    template <typename Res>
    void error(const Res &res) const {
        if (res.any_error()) {
            fmt::print("{}: error parse arguments:\n", prog);
        }
        std::stringstream ss;
        clipp::debug::print(ss, res);
        fmt::print("{}", ss.str());
    }
    void description() const { fmt::print("{}: {}\n", name, desc); }

    void version() const { fmt::print("{}\n", vers); }

    template <typename Cli>
    void usage(const Cli &cli) const {
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
    void help(const Cli &cli) const {
        usage(cli);
        fmt::print("\n{}\n", clipp::documentation(cli, docfmt).str());
    }
    template <typename Cli>
    void manpage(const Cli &cli) const {
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
        std::move(labels.back()), std::forward<Param>(param), std::forward<Doc>(doc));
};

// A helper to normalize value to vspec value types
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

// A helper class to hold additional information of a value factory. When
// called, it will return the clipp::values create by value_factory
template <typename value_t_, bool is_optional_, bool is_repeated_,
          typename value_factory_t>
struct valspec_t {
    valspec_t(value_factory_t value_factory_)
        : value_factory{std::move(value_factory_)} {}
    value_factory_t value_factory;
    static constexpr auto is_optional = is_optional_;
    static constexpr auto is_repeated = is_repeated_;
    using value_t = value_t_;
    using defval_t =
        std::conditional_t<is_repeated_, std::vector<value_t>, value_t>;

    template <typename Conf, typename key_t>
    auto operator()(Conf &conf, key_t key) const {
        return value_factory(conf, std::move(key));
    }
};

template <typename value_t, auto... options, typename... Args>
auto make_valspec(Args &&...args) {
    return valspec_t<value_t, options..., Args...>(std::forward<Args>(args)...);
}


template <
    typename value_t,
    bool is_repeated,
    typename clipp_value_func_t, typename ...Args>
auto make_clipp_value_factory(
            clipp_value_func_t clipp_value_func,
            std::string default_metavar,
            Args&& ...args
            ) {
    return [clipp_value_func = std::move(clipp_value_func), default_metavar = std::move(default_metavar), args=std::tuple{std::forward<Args>(args)...}](
               auto &conf, auto key) {
        auto _make_args = [&default_metavar, &args] () -> decltype(auto){
            using args_t = decltype(args);
            // this is to add the default metavar to the args passed
            // to clipp value func.
            if constexpr (std::tuple_size_v<args_t> == 0) {
                // no args
                return std::tuple{default_metavar};
            } else if constexpr (std::tuple_size_v<args_t> == 1) {
                if constexpr (tula::meta::StringLike<typename std::tuple_element_t<0, args_t>>)
                                        {
                        // one arg, string, so no need to set
                    return args;
                } else {
                    // one arg, not string, so append string meta var
                    return std::tuple_cat(args, std::tuple{default_metavar});
                }
            } else {
                // multiple args, use as is
                return args;
            }
        };
        return std::apply(
            clipp_value_func, _make_args()).call(
            [&conf, key = std::move(key)](auto &&value) {
                value_t v{};
                clipp::set(v)(std::forward<decltype(value)>(value));
                if constexpr (is_repeated) {
                    // initialize if not exists for list
                    if (!conf.has(key)) {
                        conf.set(key, std::vector<value_t>{});
                    }
                    conf.append(key, v);
                } else {
                    // just set the value
                    conf.set(key, v);
                }
            });
    };
}

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
            [this](auto key, auto &&param, auto &&doc) {
                // initialize as false
                this->config().set(key, false);
                return std::forward<decltype(param)>(param).call(
                           // in params set the flag to true
                           [this, key=std::move(key)]() { this->config().set(key, true); }) %
                       doc;
            });
    }

    /// @brief Positional argument according to valspec
    template <typename Doc, typename VSpec>
    auto operator()(std::string key, Doc &&doc, VSpec &&valspec) {
        // initialize as undef
        this->config().set(key, config_t::undef);
        // config is set in value.call
        return valspec(this->config(), std::move(key)) % std::forward<Doc>(doc);
    };

    /// @brief Option with value, according to valspec, defualt value inferred.
    template <typename Param, typename Doc, typename VSpec> requires (!tula::meta::StringLike<Param>)
    auto operator()(Param &&param, Doc &&doc, VSpec &&valspec) {
        return operator()(std::forward<Param>(param), std::forward<Doc>(doc),
                          typename VSpec::defval_t{},
                          std::forward<VSpec>(valspec));
    }

    /// @brief Option with value, according to valspec, default value specified
    template <typename Param, typename Doc, typename V, typename VSpec>
    auto operator()(Param &&param, Doc &&doc, V &&defval, VSpec &&valspec) {
        return internal::arg(
            std::forward<Param>(param), std::forward<Doc>(doc),
            [this, defval_ = std::forward<V>(defval),
             valspec = std::forward<VSpec>(valspec)](const auto &key,
                                                     auto param, auto &&doc_) {
                // normalize value to default value type for valspec
                using defval_t = typename VSpec::defval_t;
                defval_t defval{internal::normalize_value(defval_)};
                std::string doc{std::forward<Doc>(doc_)};
                // setup option param
                if constexpr (VSpec::is_optional) {
                    // initialize to undef if value is optional
                    // and set to default var in param callback
                    // so when the param is not specified
                    // it remains undef.
                    this->config().set(key, config_t::undef);
                    param = param.call(
                        [this, key = key, defvar = defval]() {
                            this->config().set(key, std::move(defvar));
                        });
                } else {
                    // initialize to default value directly
                    this->config().set(key, defval);
                    // and set the value in value.call
                }
                // append the doc with default value if defined
                if constexpr (!std::is_same_v<V, undef>) {
                    doc = fmt::format("{}. Default is {}", doc, defval);
                }
                return (param & valspec(this->config(), key)) % doc;
            });
    }

private:
    config_t m_config;
};

template <typename... configs_t>
struct config_parser {
    using mappers_t = std::tuple<ConfigMapper<configs_t>...>;

    template <typename F>
    auto operator()(F &&cli_builder, const screen &s, int argc,
                    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
                    char *argv[]) {
        auto cli = std::apply(std::forward<F>(cli_builder), m_mappers);
        s.parse(cli, argc, argv);
        return std::tuple_cat(
            std::tuple{cli},
            std::apply(
                [](auto &&...x) {
                    return std::tuple(std::forward<decltype(x)>(x).config()...);
                },
                std::move(m_mappers)));
    }

private:
    mappers_t m_mappers{};
};

// the non-human-readable cli builder symbols:
//       g            : group(entries...)
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
inline constexpr auto str = [](auto && ...args) {
    using value_t = std::string;
    constexpr auto is_optional = false;
    constexpr auto is_repeated = false;
    return internal::make_valspec<value_t, is_optional, is_repeated>(
        internal::make_clipp_value_factory<value_t, is_repeated>(
            TULA_LIFT(clipp::value), "arg", std::forward<decltype(args)>(args)...));
};

inline constexpr auto opt_str = [](auto && ...args) {
    using value_t = std::string;
    constexpr auto is_optional = true;
    constexpr auto is_repeated = false;
    return internal::make_valspec<value_t, is_optional, is_repeated>(
        internal::make_clipp_value_factory<value_t, is_repeated>(
            TULA_LIFT(clipp::opt_value), "arg", std::forward<decltype(args)>(args)...));
};

inline constexpr auto strs = [](auto && ...args) {
    using value_t = std::string;
    constexpr auto is_optional = false;
    constexpr auto is_repeated = true;
    return internal::make_valspec<value_t, is_optional, is_repeated>(
        internal::make_clipp_value_factory<value_t, is_repeated>(
            TULA_LIFT(clipp::values), "arg", std::forward<decltype(args)>(args)...));
};

inline constexpr auto opt_strs = [](auto && ...args) {
    using value_t = std::string;
    constexpr auto is_optional = true;
    constexpr auto is_repeated = true;
    return internal::make_valspec<value_t, is_optional, is_repeated>(
        internal::make_clipp_value_factory<value_t, is_repeated>(
            TULA_LIFT(clipp::opt_values), "arg", std::forward<decltype(args)>(args)...));
};


inline constexpr auto int_ = [](std::string metavar = "num") {
    using value_t = int;
    constexpr auto is_optional = false;
    constexpr auto is_repeated = false;
    return internal::make_valspec<value_t, is_optional, is_repeated>(
        internal::make_clipp_value_factory<value_t, is_repeated>(
            TULA_LIFT(clipp::integer), std::move(metavar)));
};

inline constexpr auto opt_int = [](std::string metavar = "num") {
    using value_t = int;
    constexpr auto is_optional = true;
    constexpr auto is_repeated = false;
    return internal::make_valspec<value_t, is_optional, is_repeated>(
        internal::make_clipp_value_factory<value_t, is_repeated>(
            TULA_LIFT(clipp::opt_integer), std::move(metavar)));
};

inline constexpr auto ints = [](std::string metavar = "num") {
    using value_t = int;
    constexpr auto is_optional = false;
    constexpr auto is_repeated = true;
    return internal::make_valspec<value_t, is_optional, is_repeated>(
        internal::make_clipp_value_factory<value_t, is_repeated>(
            TULA_LIFT(clipp::integers), std::move(metavar)));
};

inline constexpr auto opt_ints = [](std::string metavar = "num") {
    using value_t = int;
    constexpr auto is_optional = true;
    constexpr auto is_repeated = true;
    return internal::make_valspec<value_t, is_optional, is_repeated>(
        internal::make_clipp_value_factory<value_t, is_repeated>(
            TULA_LIFT(clipp::opt_integers), std::move(metavar)));
};

inline constexpr auto doub = [](std::string metavar = "value") {
    using value_t = double;
    constexpr auto is_optional = false;
    constexpr auto is_repeated = false;
    return internal::make_valspec<value_t, is_optional, is_repeated>(
        internal::make_clipp_value_factory<value_t, is_repeated>(
            TULA_LIFT(clipp::number), std::move(metavar)));
};

inline constexpr auto opt_doub = [](std::string metavar = "value") {
    using value_t = double;
    constexpr auto is_optional = true;
    constexpr auto is_repeated = false;
    return internal::make_valspec<value_t, is_optional, is_repeated>(
        internal::make_clipp_value_factory<value_t, is_repeated>(
            TULA_LIFT(clipp::opt_number), std::move(metavar)));
};

inline constexpr auto doubs = [](std::string metavar = "value") {
    using value_t = double;
    constexpr auto is_optional = false;
    constexpr auto is_repeated = true;
    return internal::make_valspec<value_t, is_optional, is_repeated>(
        internal::make_clipp_value_factory<value_t, is_repeated>(
            TULA_LIFT(clipp::numbers), std::move(metavar)));
};

inline constexpr auto opt_doubs = [](std::string metavar = "value") {
    using value_t = double;
    constexpr auto is_optional = true;
    constexpr auto is_repeated = true;
    return internal::make_valspec<value_t, is_optional, is_repeated>(
        internal::make_clipp_value_factory<value_t, is_repeated>(
            TULA_LIFT(clipp::opt_numbers), std::move(metavar)));
};

inline constexpr auto list = [](auto &&choices_) {
    // figure out the choices names, if they are from enum
    auto choices = [](auto &&c) {
        using T = std::decay_t<decltype(choices_)>;
        if constexpr (std::is_enum_v<T>) {
            return enum_utils::names<T>();
        } else {
            return std::forward<decltype(c)>(c);
        }
    }(std::forward<decltype(choices_)>(choices_));
    using value_t = std::string;
    constexpr auto is_optional = false;
    constexpr auto is_repeated = false;
    return internal::make_valspec<value_t, is_optional, is_repeated>(
        [choices=std::move(choices)](auto &conf, auto key) {
            clipp::group g;
            for (const auto &x : choices) {
                std::ostringstream oss;
                oss << x;
                g.push_back(clipp::command(oss.str()).call(
                    [ &conf, key=key, x=x](){ conf.set(key, x); }));
            }
            g.exclusive(true);
            return g;
        });
};

namespace m = clipp::match;

} // namespace clipp_builder
} // namespace tula::cli
