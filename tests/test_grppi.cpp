#include <tula/grppi.h>

#include <vector>

int main()
{
    const std::vector input{1, 2, 3};
    std::vector<int> output(input.size());
    auto execution = tula::grppi_utils::dyn_ex("seq");
    execution.map(
        std::make_tuple(input.begin()),
        output.begin(),
        input.size(),
        [](int value) { return value * 2; });
    return output == std::vector{2, 4, 6} ? 0 : 1;
}
