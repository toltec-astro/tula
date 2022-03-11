#include <thread>
#include <tula/cli.h>
#include <tula/config/flatconfig.h>
#include <tula/config/yamlconfig.h>
#include <tula/grppi.h>
#include <tula/logging.h>
#include <tula_config/config.h>
#include <tula_config/gitversion.h>

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
auto parse_args(int argc, char *argv[]) {
    tula::logging::scoped_timeit TULA_X("parse_args");
    using namespace tula::cli::clipp_builder;

    auto ver_str = fmt::format("{} {} ({})", TULA_PROJECT_NAME,
                               TULA_GIT_VERSION, TULA_BUILD_TIMESTAMP);
    constexpr auto level_names = tula::logging::active_level_names;
    auto default_level_name = []() {
        auto v = spdlog::level::debug;
        if (v < tula::logging::active_level) {
            v = tula::logging::active_level;
        }
        return tula::logging::get_level_name(v);
    }();
    using ex_config = tula::grppi_utils::ex_config;
    auto parse = config_parser<tula::config::YamlConfig, tula::config::FlatConfig>{};
    // using config_mapper_t = ConfigMapper<config_t>;
    // config_mapper_t r{};
    // config_mapper_t c{};
    // clang-format off
    auto screen = tula::cli::screen{
    //=======================================================================//
              "cli_builder" , TULA_PROJECT_NAME, ver_str,
                              TULA_PROJECT_DESCRIPTION};
    // auto [cli, rc, cc] = tula::logging::timeit("parse", parse,
    auto [cli, rc, cc] = parse(
    [&](auto &r, auto &c) { return (
    // auto cli =  (
    //=======================================================================//
    c(p(        "h", "help"), "Print help information and exit."),
    c(p(          "version"), "Print version information and exit."),
    //=======================================================================//
    r( (        "config_file"), "The path of input config file.",
                              opt_strs("file")),
    //=======================================================================//
           "common options"  % g(
    c(p(   "l", "log_level"), "Set the log level.",
                              default_level_name, list(level_names)),
    r(p(             "plot"), "Make diagnostic plot."),
    r(p(     "plot_backend"), "Matplotlib backend to use",
                              "default", str()),
    r(p(          "grppiex"), "GRPPI execution policy",
                              ex_config::default_mode(),
                              list(ex_config::mode_names_supported())))
    //=======================================================================//
    );}, screen, argc, argv);
    // screen.parse(cli, argc, argv);
    // auto cc = std::move(c).config();
    // auto rc = std::move(r).config();
    // clang-format on
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
    return std::move(rc);
}

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
int main(int argc, char *argv[]) {
    tula::logging::init(spdlog::level::trace);
    try {
        auto rc = parse_args(argc, argv);
        SPDLOG_INFO("rc: {}", rc.pformat());
        // run a simple loop
        /*
        {
            tula::logging::scoped_timeit TULA_X{"Some progress"};
            constexpr auto pbw = 60;
            tula::logging::progressbar pb0{
                [](auto &&msg) {
                    SPDLOG_INFO("{}", std::forward<decltype(msg)>(msg));
                },
                pbw, "Some progres: "};
            constexpr auto total = 100;
            constexpr auto pb_stride = 100 / 50;
            constexpr auto sleep_ms = 10;
            for (auto i = 0; i < total; ++i) {
                pb0.count(total, pb_stride);
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(sleep_ms));
            }
        }
        */
        return EXIT_SUCCESS;
    } catch (std::exception const &e) {
        SPDLOG_ERROR("abort: {}", e.what());
        return EXIT_FAILURE;
    }
}
