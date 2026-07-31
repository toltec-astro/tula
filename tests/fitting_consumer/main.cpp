#include <tula/algorithm/ei_ceresfitter.h>
#include <tula/config.h>

int main()
{
    static_assert(TULA_HAS_FITTING == 1);
    const auto setting =
        tula::alg::ceresfit::ParamSetting<double>::getbounded(-2.0, 3.0);
    return setting.lower_bound == -2.0 && setting.upper_bound == 3.0 ? 0 : 1;
}
