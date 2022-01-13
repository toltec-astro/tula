#include <gtest/gtest.h>

#include "../test_common.h"
#include <tula/enum.h>
#include <tula/formatter/enum.h>

namespace tula::testing {

// Define some meta enums
// enum for test meta_enum
TULA_ENUM(Type, int, TypeA, TypeB, TypeC);

// enum for test meta_enum with bitmask
TULA_ENUM(Flag, int, FlagA = 1 << 0, FlagB = 1 << 1, FlagC = 1 << 2,
          FlagD = FlagA | FlagB | FlagC, FlagE = FlagB | FlagC);
TULA_BITFLAG_MAX_ELEMENT(Flag, FlagC);

// enum for test bitmask
enum class Bit : int { BitA = 1 << 0, BitB = 1 << 1, BitC = 1 << 2 };
TULA_BITFLAG_MAX_ELEMENT(Bit, BitC);

// enum in nested scope

struct A {
    TULA_ENUM_DECL(AType, int, Value1, Value2);
};

TULA_ENUM_REGISTER(A::AType);

} // namespace tula::testing

namespace {

using namespace tula::testing;

// NOLINTNEXTLINE
TEST(formatter, meta_enum_type) {
    using meta = Type_meta;
    EXPECT_NO_THROW(fmtlog("{}: members{}", meta::name, meta::members));
    EXPECT_NO_THROW(fmtlog("{}: non existing member {}", meta::name,
                           meta::to_name(static_cast<Type>(-1))));
    EXPECT_NO_THROW(fmtlog("TypeA: {:d}", meta::from_name("TypeA")));
    EXPECT_NO_THROW(fmtlog("TypeA: {:s}", meta::from_name("TypeA")));
    EXPECT_NO_THROW(fmtlog("TypeA: {:l}", meta::from_name("TypeA")));
    EXPECT_NO_THROW(fmtlog("TypeA: {:s}", Type::TypeA));
    EXPECT_NO_THROW(fmtlog("abc: {}", meta::from_name("abc")));
}

// NOLINTNEXTLINE
TEST(formatter, meta_enum_flag) {
    using meta = Flag_meta;
    EXPECT_NO_THROW(fmtlog("{}: members{}", meta::name, meta::members));
    EXPECT_NO_THROW(fmtlog("{}: non existing member {}", meta::name,
                           meta::to_name(static_cast<Flag>(-1))));
    EXPECT_NO_THROW(fmtlog("FlagA: {:d}", meta::from_name("FlagA")));
    EXPECT_NO_THROW(fmtlog("FlagA: {:s}", meta::from_name("FlagA")));
    EXPECT_NO_THROW(fmtlog("FlagA: {:l}", meta::from_name("FlagA")));
    EXPECT_NO_THROW(fmtlog("FlagA: {:s}", Flag::FlagA));

    EXPECT_NO_THROW(fmtlog("FlagC: {:d}", meta::from_name("FlagC")));
    EXPECT_NO_THROW(fmtlog("FlagC: {:s}", meta::from_name("FlagC")));
    EXPECT_NO_THROW(fmtlog("FlagC: {:l}", meta::from_name("FlagC")));
    EXPECT_NO_THROW(fmtlog("FlagC: {:s}", Flag::FlagC));

    EXPECT_NO_THROW(fmtlog("FlagD: {:d}", meta::from_name("FlagD")));
    EXPECT_NO_THROW(fmtlog("FlagD: {:s}", meta::from_name("FlagD")));
    EXPECT_NO_THROW(fmtlog("FlagD: {:l}", meta::from_name("FlagD")));
    EXPECT_NO_THROW(fmtlog("FlagD: {:s}", Flag::FlagD));
    EXPECT_NO_THROW(fmtlog("abc: {}", meta::from_name("abc")));
}

// NOLINTNEXTLINE
TEST(formatter, bitmask_bit) {
    auto bm = bitmask::bitmask<Bit>{};
    EXPECT_NO_THROW(fmtlog("BitA: {:d}", bm | Bit::BitA));
    EXPECT_NO_THROW(fmtlog("BitA: {:s}", bm | Bit::BitA));
    EXPECT_NO_THROW(fmtlog("BitA: {:l}", bm | Bit::BitA));
    EXPECT_NO_THROW(fmtlog("BitA: {:s}", bm | Bit::BitA));

    EXPECT_NO_THROW(fmtlog("BitAC: {:d}", Bit::BitC | Bit::BitA));
    EXPECT_NO_THROW(fmtlog("BitAC: {:s}", Bit::BitC | Bit::BitA));
    EXPECT_NO_THROW(fmtlog("BitAC: {:l}", Bit::BitC | Bit::BitA));
    EXPECT_NO_THROW(fmtlog("BitAC: {:s}", Bit::BitC | Bit::BitA));
}

// NOLINTNEXTLINE
TEST(formatter, scoped_enum) {
    using meta = A::AType_meta;
    EXPECT_NO_THROW(fmtlog("{}: members{}", meta::name, meta::members));
    EXPECT_NO_THROW(fmtlog("{}: non existing member {}", meta::name,
                           meta::to_name(static_cast<A::AType>(-1))));
    EXPECT_NO_THROW(fmtlog("AType::Value1: {:d}", meta::from_name("Value1")));
    EXPECT_NO_THROW(fmtlog("AType::Value1: {:s}", meta::from_name("Value1")));
    EXPECT_NO_THROW(fmtlog("AType::Value1: {:l}", meta::from_name("Value1")));
    EXPECT_NO_THROW(fmtlog("AType::Value1: {:s}", A::AType::Value1));
    EXPECT_NO_THROW(fmtlog("abc: {}", meta::from_name("abc")));
}

} // namespace
