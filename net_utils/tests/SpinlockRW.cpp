#include <gtest/gtest.h>

#include <net_utils/aux/SpinlockRWFixture.hpp>


namespace {


using namespace nut;


class SpinlockRWTest
    : public ::testing::Test
    , public aux::SpinlockRWFixture {
protected:
  static auto constexpr r_iters = 1000;
  static auto constexpr w_iters = 25;

protected:
  void SetUp() override {
    up(9. / 10, r_iters, w_iters);
  }
};


TEST_F(SpinlockRWTest, smoke) {
  start();
  down();
  ASSERT_EQ(data, 0);
  ASSERT_EQ(iter_counter(), r_iters * readers() + w_iters * writers());
}


}// namespace