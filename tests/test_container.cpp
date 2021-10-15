#include <gtest/gtest.h>

#include "test_common.h"
#include <tula/container.h>
#include <tula/eigen.h>
#include <tula/formatter/matrix.h>
#include <tula/grppi.h>
#include <tula/logging.h>

namespace {

using namespace tula::testing;

TEST(container, create) {

    using namespace tula::container_utils;

    auto modes = tula::grppi_utils::ExMode_meta::members;
    fmtlog("ex_modes: {}", modes);

    auto vm = create<std::vector<std::string>>(
        modes, [](const auto &m) { return std::string(m.name); });

    fmtlog("vector ex_modes: {}", vm);

    std::vector<int> a = {1, 2, 3};
    fmtlog("vec int a: {}", a);
    auto b = create<std::vector<double>>(a);
    fmtlog("vec double b: {}", b);

    // check move
    auto sm = create<std::set<std::string>>(std::move(vm));
    fmtlog("vector ex_modes after: {}, {}", vm, vm.size());
    fmtlog("set ex_modes: {}", sm);

    std::vector<std::string> ts = {"abc", "def", "ghi"};
    auto cs = create<std::vector<std::string>>(std::move(ts));
    fmtlog("vecstr ts after: {}", ts);
    fmtlog("vecstr cs: {}", cs);
}

} // namespace
