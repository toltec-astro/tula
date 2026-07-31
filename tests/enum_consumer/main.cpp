#include <tula/config.h>
#include <tula/enum.h>

TULA_ENUM(Mode, int, first, second);

int main()
{
    static_assert(TULA_HAS_ENUM == 1);
    return tula::enum_utils::name(Mode::second) == "second" ? 0 : 1;
}
