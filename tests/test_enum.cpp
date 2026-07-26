#include <tula/enum.h>
#include <tula/formatter/enum.h>

#include <fmt/format.h>

#include <array>
#include <string_view>

namespace {

TULA_ENUM(Type, int, first, second, third);
TULA_ENUM(
    Flag,
    int,
    first = 1 << 0,
    second = 1 << 1,
    third = 1 << 2,
    all = first | second | third);
TULA_BITFLAG_MAX_ELEMENT(Flag, third);

struct Nested {
    TULA_ENUM_DECL(Value, int, one, two);
};

TULA_ENUM_REGISTER(Nested::Value);

} // namespace

int main()
{
    using namespace tula::enum_utils;

    static_assert(EnumWithMeta<Type>);
    static_assert(BitFlagWithMeta<Flag>);
    static_assert(EnumWithMeta<Nested::Value>);
    static_assert(names<Type>() == std::array<std::string_view, 3>{
                                       "first", "second", "third"});
    static_assert(values<Type>() == std::array{Type::first, Type::second, Type::third});
    static_assert(bitmask_v<Flag> == 7);
    static_assert(bitwidth_v<Flag> == 3);
    static_assert(is_compound_v<Flag::all>);

    if (name(Type::second) != "second" || name(static_cast<Type>(-1)) != "(undef)") {
        return 1;
    }
    if (fmt::format("{}", Type::second) != "second") {
        return 2;
    }
    if (fmt::format("{:d}", Type::third) != "b10") {
        return 3;
    }

    const auto flags = Flag::first | Flag::third;
    if (flags.bits() != 5 || fmt::format("{:s}", flags) != "(third|first)") {
        return 4;
    }
    if (fmt::format("{}", flags) != "(third|first,b101)") {
        return 5;
    }
    return fmt::format("{}", Nested::Value::two) == "two" ? 0 : 6;
}
