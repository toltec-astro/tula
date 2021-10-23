#include <gtest/gtest.h>
#include "../test_common.h"

namespace {

using namespace tula::testing;

struct A {
    A () = default;
    A (int v): m_value{v} {}
    template <typename OStream>
    friend auto operator<<(OStream &os, const A &a )
        -> decltype(auto) {
        return os << fmt::format("A(value={})", a.value());
    }

    int value() const {return m_value;}

private:
    int m_value{0};

};

TEST(formatter, ostream) {
  fmtlog("{}\n", A{});
  fmtlog("{}\n", A{1});
}

}  // namespace
