#include "common.h"
#include <gtest/gtest.h>
#include <tula/grppi.h>

namespace {

using namespace tula::testing;

// NOLINTNEXTLINE
TEST(grppi_utils, exconfig) {

    using namespace tula::grppi_utils;

    EXPECT_NO_THROW(fmtlog("enabled modes: {:s}", ex_config::modes_enabled()));
    EXPECT_NO_THROW(fmtlog("default mode: {}", default_mode()));
    // omp is higher priority in the default order
    EXPECT_EQ(default_mode_name(ExMode::seq | ExMode::omp),
              default_mode_name(ExMode::omp));
    // omp is higher priority in the par, which is omp|tbb|ff|trh
    EXPECT_EQ(default_mode_name(ExMode::par), default_mode_name(ExMode::omp));
    // custom config
    using cfg = ExConfig<ExMode::seq, ExMode::omp>;
    SPDLOG_TRACE("enabled modes: {:s}", cfg::modes_enabled());
    EXPECT_EQ(cfg::default_mode_name(ExMode::seq | ExMode::omp),
              cfg::default_mode_name(ExMode::seq));
    EXPECT_EQ(cfg::default_mode_name(ExMode::par),
              cfg::default_mode_name(ExMode::omp));
    // dyn_ex
    EXPECT_NO_THROW(dyn_ex("omp"));
    EXPECT_NO_THROW(cfg::dyn_ex(ExMode::par));
    EXPECT_NO_THROW(cfg::dyn_ex(ExMode::seq));
}

} // namespace
