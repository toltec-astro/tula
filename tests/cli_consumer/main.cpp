#include <tula/cli.h>
#include <tula/config.h>

int main()
{
    static_assert(TULA_HAS_CLI == 1);
    auto value = 0;
    auto cli = clipp::option("--value") & clipp::value("value", value);
    return clipp::parse({"--value", "7"}, cli) && value == 7 ? 0 : 1;
}
