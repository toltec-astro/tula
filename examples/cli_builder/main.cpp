#include <tula/cli.h>
#include <tula/config/flatconfig.h>
#include <tula/grppi.h>
#include <tula/logging.h>
#include <tula_config/config.h>
#include <tula_config/gitversion.h>

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
auto parse_args(int argc, char *argv[]) {

    using namespace tula::cli::clipp_builder;
    using config_mapper_t = ConfigMapper<tula::config::FlatConfig>;
    // CLI config container, this is consumed and discarded.
    config_mapper_t c{};
    // Runtime config container, this is returned.
    config_mapper_t r{};

    auto ver_str = fmt::format("{} {} ({})", TULA_PROJECT_NAME,
                               TULA_GIT_VERSION, TULA_BUILD_TIMESTAMP);
    auto screen = tula::cli::screen{"cli_builder", TULA_PROJECT_NAME, ver_str,
                                    TULA_PROJECT_DESCRIPTION};

    constexpr auto level_names = tula::logging::active_level_names;
    auto default_level_name = []() {
        auto v = spdlog::level::debug;
        if (v < tula::logging::active_level) {
            v = tula::logging::active_level;
        }
        return tula::logging::get_level_name(v);
    }();
    using ex_config = tula::grppi_utils::ex_config;
    // clang-format off
    auto cli = (
    //=======================================================================//
    c(p(        "h", "help"), "Print help information and exit."),
    c(p(          "version"), "Print version information and exit."),
    //=======================================================================//
    r( (        "config_file"), "The path of input config file.",
                              opt_str()),
    //=======================================================================//
            "common options" % g(
    c(p(   "l", "log_level"), "Set the log level.",
                              default_level_name, list(level_names)),
    r(p(             "plot"), "Make diagnostic plot."),
    r(p(     "plot_backend"), "Matplotlib backend to use",
                              "default", str()),
    r(p(          "grppiex"), "GRPPI execution policy",
                              ex_config::default_mode(),
                              list(ex_config::mode_names_supported())))
    //=======================================================================//
    );
    // clang-format on
    screen.parse(cli, argc, argv);
    // handle CLI config
    auto cc = std::move(c).config(); // this discards c
    SPDLOG_TRACE("cc: {}", cc.pformat());
    if (cc.get_typed<bool>("help")) {
        screen.manpage(cli);
        std::exit(EXIT_SUCCESS);
    } else if (cc.get_typed<bool>("version")) {
        screen.version();
        std::exit(EXIT_SUCCESS);
    }
    {
        auto log_level_str = cc.get_str("log_level");
        auto log_level = spdlog::level::from_str(log_level_str);
        SPDLOG_DEBUG("reconfigure logger to level={}", log_level_str);
        spdlog::set_level(log_level);
    }
    // pass on the runtime config
    return std::move(r).config();
}

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
int main(int argc, char *argv[]) {
    tula::logging::init(spdlog::level::trace);
    try {
        auto rc = parse_args(argc, argv);
        SPDLOG_TRACE("rc: {}", rc.pformat());
        return EXIT_SUCCESS;
    } catch (std::exception const &e) {
        SPDLOG_ERROR("abort: {}", e.what());
        return EXIT_FAILURE;
    }
}
