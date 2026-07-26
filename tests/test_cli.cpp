#include <tula/cli.h>
#include <tula/config/flatconfig.h>

#include <string>

namespace {

TULA_ENUM(Mode, int, fast, safe);

} // namespace

int main()
{
    using namespace tula::cli::clipp_builder;

    ConfigMapper<tula::config::FlatConfig> mapper;
    auto cli = (
        mapper(p("v", "verbose"), "Enable verbose output."),
        mapper(p("n", "count"), "Number of iterations.", 1, int_()),
        mapper(p("mode"), "Execution mode.", Mode::fast, list(Mode{})),
        mapper("input", "Input path.", str()));

    const auto result =
        clipp::parse({"--verbose", "--count", "3", "--mode", "safe", "data.nc"}, cli);
    if (!result) {
        return 1;
    }

    const auto &config = mapper.config();
    if (!config.get_typed<bool>("verbose")) {
        return 2;
    }
    if (config.get_typed<int>("count") != 3) {
        return 3;
    }
    if (config.get_str("mode") != "safe") {
        return 4;
    }
    if (config.get_str("input") != "data.nc") {
        return 5;
    }
    return 0;
}
