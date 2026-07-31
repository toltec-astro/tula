#include <tula/config.h>

#include <tula/bits.h>
#include <tula/concepts.h>
#include <tula/meta.h>
#include <tula/nddata/core.h>
#include <tula/switch_invoke.h>
#include <tula/traits.h>

#if TULA_HAS_LOGGING
#include <tula/formatter/container.h>
#include <tula/formatter/duration.h>
#include <tula/logging.h>
#endif
#if TULA_HAS_PERFLIBS
#include <tula_perflibs/config.h>
#endif

#include <iostream>
#include <string_view>
#include <vector>

int main()
{
    static_assert(std::string_view{TULA_VERSION} == "3.1.0");
    std::cout << "logging=" << TULA_HAS_LOGGING << '\n';
    std::cout << "perflibs.openmp=" << TULA_HAS_OPENMP << '\n';

#if TULA_HAS_LOGGING
    const std::vector<int> values{1, 2, 3};
    std::cout << fmt::format("values={}", values) << '\n';
    spdlog::info("Tula logging header compiled and executed");
#endif

#if TULA_HAS_PERFLIBS
    static_assert(TULA_PERFLIBS_HAS_THREADS == 1);
    static_assert(TULA_HAS_OPENMP == TULA_PERFLIBS_HAS_OPENMP);
#endif
    return 0;
}
