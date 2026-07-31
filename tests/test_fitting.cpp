#include <tula/algorithm/ei_ceresfitter.h>

int main()
{
    const auto fixed =
        tula::alg::ceresfit::ParamSetting<double>::getfixed(2.5);
    if (!fixed.fixed.has_value() || fixed.fixed.value() != 2.5) {
        return 1;
    }

    const auto bounded =
        tula::alg::ceresfit::ParamSetting<double>::getbounded(-1.0, 1.0);
    return bounded.lower_bound == -1.0 && bounded.upper_bound == 1.0 ? 0 : 2;
}
