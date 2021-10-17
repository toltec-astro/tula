
#include "test_common.h"
#include <gtest/gtest.h>
#include <tula/nc.h>

namespace {

using namespace tula::testing;

TEST(nc, type) {

    using namespace tula::nc_utils;

    static_assert(
        std::is_same_v<internal::type_t<netCDF::NcType::ncType::nc_CHAR>,
                       char>);
}

} // namespace
