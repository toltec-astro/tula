#include "common.h"
#include <gtest/gtest.h>
#include <tula/grppiex.h>

namespace {

using namespace tula::testing;

TEST(grppiex, modes) {

    using namespace tula::grppiex;

    EXPECT_NO_THROW(fmtlog("using default modes: {:s}", modes::enabled()));
    EXPECT_NO_THROW(fmtlog("default mode: {}", default_mode()));
    EXPECT_EQ(default_mode_name(Mode::seq | Mode::omp),
              default_mode_name(Mode::omp));
    EXPECT_EQ(default_mode_name(Mode::par), default_mode_name(Mode::omp));
    // custom modes
    using ms = Modes<Mode::seq, Mode::omp>;
    SPDLOG_TRACE("using modes: {:s}", ms::enabled());
    EXPECT_EQ(ms::default_name(Mode::seq | Mode::omp),
              ms::default_name(Mode::seq));
    EXPECT_EQ(ms::default_name(Mode::par), ms::default_name(Mode::omp));
    // dyn_ex
    EXPECT_NO_THROW(dyn_ex("omp"));
    EXPECT_NO_THROW(ms::dyn_ex(Mode::par));
    EXPECT_NO_THROW(ms::dyn_ex(Mode::seq));
}

} // namespace