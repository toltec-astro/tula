#include <tula/config.h>
#include <tula/grppi.h>

#include <algorithm>
#include <string>
#include <vector>

int main()
{
    const auto supported = tula::grppi_utils::ex_config::mode_names_supported();
    const auto has_openmp =
        std::ranges::find(supported, std::string{"omp"}) != supported.end();
    if (has_openmp != static_cast<bool>(TULA_HAS_OPENMP)) {
        return 1;
    }

    const std::vector input{1, 2, 3};
    std::vector<int> output(input.size());
    auto execution = tula::grppi_utils::dyn_ex("seq");
    execution.map(
        std::make_tuple(input.begin()),
        output.begin(),
        input.size(),
        [](int value) { return value * 2; });
    if (output != std::vector{2, 4, 6}) {
        return 2;
    }

#if TULA_HAS_OPENMP
    output.assign(input.size(), 0);
    auto openmp = tula::grppi_utils::dyn_ex("omp");
    openmp.map(
        std::make_tuple(input.begin()),
        output.begin(),
        input.size(),
        [](int value) { return value * 3; });
    return output == std::vector{3, 6, 9} ? 0 : 3;
#else
    try {
        static_cast<void>(tula::grppi_utils::dyn_ex("omp"));
    } catch (const std::runtime_error &) {
        return 0;
    }
    return 4;
#endif
}
