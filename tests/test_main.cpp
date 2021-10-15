#include "test_common.h"
#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <tula/formatter/ptr.h>
#include <tula/logging.h>

namespace {} // namespace

int main(int argc, char *argv[]) {
    tula::logging::init(spdlog::level::trace);
    tula::testing::logger()->set_error_handler(
        [](const std::string &msg) { throw std::runtime_error(msg); });
    testing::InitGoogleTest(&argc, argv);
    benchmark::Initialize(&argc, argv);
    fmt::print("Running tests:\n");
    int result = -1;
    {
        tula::logging::scoped_timeit TULA_X{"tests"};
        result = RUN_ALL_TESTS();
    }
    if (result == 0) {
        fmt::print("\nRunning benchmarks:\n");
        {
            tula::logging::scoped_timeit TULA_X{"benchmarks"};
            benchmark::RunSpecifiedBenchmarks();
        }
    }
    return result;
}
