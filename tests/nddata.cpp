#include <gtest/gtest.h>

#include "common.h"
#include <stdexcept>
#include <tula/formatter/matrix.h>
#include <tula/logging.h>
#include <tula/nddata/cacheddata.h>
#include <tula/nddata/eigen.h>

namespace {

using namespace tula::testing;

TEST(nddata, type_traits) {

    using namespace tula::nddata;
    static_assert(std::is_same_v<type_traits<int>::index_t, std::size_t>);
    static_assert(
        std::is_same_v<type_traits<EigenData<Eigen::MatrixXd>>::index_t,
                       Eigen::Index>);
}

// NOLINTNEXTLINE
TEST(nddata, eigen_data) {
    using namespace tula::nddata;
    Eigen::MatrixXd m{{0, 1}, {2, 3}, {4, 5}};
    auto mm = EigenData{m};
    fmtlog("mm{}", mm());
    EXPECT_EQ(mm.data, m);
    EXPECT_EQ(mm(), m);

    auto nn = EigenDataRef(m);
    // static_assert(std::is_same_v<decltype(EigenDataRef(m))::ref_t, void>);
    fmtlog("nn{}", nn());
    EXPECT_EQ(nn.data, m);
    EXPECT_EQ(nn(), m);
    m.coeffRef(0, 0) = 9;
    fmtlog("nn{}", nn());
    EXPECT_EQ(nn()(0, 0), 9.);

    auto pp = EigenDataRef(mm().block(0, 0, 2, 2));
    // static_assert(std::is_same_v<decltype(EigenDataRef(m))::ref_t, void>);
    fmtlog("pp{}", pp());
    EXPECT_EQ(pp.data, mm().block(0, 0, 2, 2));
    EXPECT_EQ(pp(), mm().block(0, 0, 2, 2));
    mm().coeffRef(0, 0) = 8;
    fmtlog("pp{}", pp());
    EXPECT_EQ(pp()(0, 0), 8.);
}

struct TestCachedData {

    struct some_value_evaluator {
        template <typename T>
        static auto evaluate(T & /*unused*/) {
            return TestCachedData::get_value();
        }
    };

    struct some_other_value_evaluator {
        template <typename T>
        static auto evaluate(T &self) -> int {
            if constexpr (std::is_const_v<T>) {
                throw std::runtime_error("This property modifies the object.");
            } else {
                auto v = self.get_other_value();
                self.set_other_value(v * 2);
                return v;
            }
        }
    };

    TULA_CACHED_GETTER_DECL(some_value, int);
    TULA_CACHED_GETTER_DECL(some_other_value, int);

public:
    // every time this is called, x incremets one
    static int get_value() {
        static int x = 0;
        return ++x;
    }

    int get_other_value() const { return m_other_value; }
    void set_other_value(int v) { m_other_value = v; }

private:
    int m_other_value{1};
};

// NOLINTNEXTLINE
TEST(nddata, cached_data) {

    auto td = TestCachedData{};
    EXPECT_EQ(td.some_value(), 1);
    EXPECT_EQ(td.some_value(), 1);
    EXPECT_EQ(td.some_value(), 1);
    EXPECT_EQ(td.some_value_invalidate().some_value(), 2);
    EXPECT_EQ(td.some_value(), 2);

    EXPECT_EQ(td.get_other_value(), 1);
    EXPECT_EQ(td.some_other_value(), 1);
    EXPECT_EQ(td.get_other_value(), 2);
    EXPECT_EQ(td.some_other_value_invalidate().some_other_value(), 2);
    EXPECT_EQ(td.get_other_value(), 4);
    EXPECT_EQ(td.some_other_value(), 2);

    // assignment
    auto td2 = td;
    EXPECT_EQ(td2.some_value(), 2);
    EXPECT_EQ(td2.some_value_invalidate().some_value(), 3);
    EXPECT_EQ(td2.some_other_value(), 2);
    EXPECT_EQ(td2.some_other_value_invalidate().some_other_value(), 4);

    // move
    auto td3 = std::move(td);
    EXPECT_EQ(td3.some_value(), 2);
    EXPECT_EQ(td3.some_value_invalidate().some_value(), 4);
    EXPECT_EQ(td3.some_other_value(), 2);
    EXPECT_EQ(td3.some_other_value_invalidate().some_other_value(), 4);
}

} // namespace
