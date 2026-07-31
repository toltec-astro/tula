#include <tula/config.h>
#include <tula/grppi.h>

#include <algorithm>
#include <string>

int main()
{
    static_assert(TULA_HAS_GRPPI == 1);
    static_assert(TULA_HAS_OPENMP == EXPECT_OPENMP);
    const auto names = tula::grppi_utils::ex_config::mode_names_supported();
    const auto has_openmp =
        std::ranges::find(names, std::string{"omp"}) != names.end();
    return has_openmp == static_cast<bool>(EXPECT_OPENMP) ? 0 : 1;
}
