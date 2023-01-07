#include <tula/cli.h>
#include <tula/config/flatconfig.h>
#include <tula/config/yamlconfig.h>
#include <tula/logging.h>
#include <tula_config/config.h>
#include <tula_config/gitversion.h>
#include <tula/ecsv/table.h>
#include <tula/formatter/matrix.h>
#include <fstream>
#include <csv_parser/parser.hpp>

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
    auto parse =
        config_parser<tula::config::YamlConfig, tula::config::FlatConfig>{};
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
    r( (        "filepath"), "The path of input ecsv file.",
                              str("filepath")),
    //=======================================================================//
           "common options"  % g(
    c(p(   "l", "log_level"), "Set the log level.",
                              default_level_name, list(level_names)))
    //=======================================================================//
    );}, screen, argc, argv);
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

auto read_ecsv(std::string filepath)
{
    using namespace tula::ecsv;
    auto fmtlog = [](auto &&fmt_str, auto && ... args) {
        auto result = fmt::format(fmt::runtime(fmt_str), args...);
        SPDLOG_DEBUG(result);
    };

    std::ifstream fo(filepath);
    auto tbl = ECSVTable(ECSVHeader::read(fo));
    auto parser = aria::csv::CsvParser(fo).delimiter(tbl.header().delimiter());
    tbl.load_rows(parser);

    fmtlog("tbl: {}", tbl);
    fmtlog("tbl_info:\n{}", tbl.info());
    fmtlog("tbl header: {}", tbl.header());
    fmtlog("tbl meta:\n{}", YAML::Dump(tbl.header().meta()));
    fmtlog("tbl loader: {}", tbl.loader());
    fmtlog("tbl bool data: {}", tbl.array_data<bool>());
    fmtlog("tbl int data: {}", tbl.array_data<int>());
    fmtlog("tbl int64 data: {}", tbl.array_data<int64_t>());
    fmtlog("tbl double data: {}", tbl.array_data<double>());
    fmtlog("tbl complex data: {}", tbl.array_data<std::complex<double>>());
    fmtlog("tbl str data: {}", tbl.array_data<std::string>());
    fmtlog("col data nw{}", tbl.col<double>("nw").data);
    fmtlog("col data fg{}", tbl.col<double>("fg").data);
    fmtlog("all double data array {}", tbl.array_data<double>().array());
    return tbl;
}

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
int main(int argc, char *argv[]) {
    tula::logging::init(spdlog::level::trace);
    try {
        auto rc = parse_args(argc, argv);
        SPDLOG_INFO("rc: {}", rc.pformat());
        auto filepath = rc.get_str("filepath");
        {
        tula::logging::scoped_timeit TULA_X("read ECSV table");
        auto tbl = read_ecsv(filepath);
        SPDLOG_INFO("tbl {}\n{}", filepath, tbl.info());
        }
        return EXIT_SUCCESS;
    } catch (std::exception const &e) {
        SPDLOG_ERROR("abort: {}", e.what());
        return EXIT_FAILURE;
    }
}
