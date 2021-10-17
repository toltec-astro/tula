#include <gtest/gtest.h>
#include <tula/filename.h>
#include "test_common.h"

namespace {


using namespace tula::testing;

TEST(filename, parse) {

    using namespace tula::filename_utils;
    auto v = parse_pattern("{stem}{index}.txt", "abc.nc", fmt::arg("index", 1));
    auto vp = std::filesystem::path(v);
    EXPECT_EQ(vp.filename().string(), "abc1.txt");

}

} // namespace
