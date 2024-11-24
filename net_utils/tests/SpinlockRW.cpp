#include <gtest/gtest.h>

#include <net_utils/aux/SpinlockRWFixture.hpp>


using namespace nut;


namespace {


template<typename P_>
class SpinlockRWTest
    : public ::testing::Test
    , public aux::SpinlockRWFixture<P_> {
protected:
  static auto constexpr r_iters = 100'000;
  static auto constexpr w_iters = 10'000;

protected:
  void SetUp() override {
    this->set_rw_relation(1. / 2);
    this->readers *= 2;
    this->writers *= 2;
    this->up(r_iters, w_iters);
  }


  void TearDown() override {
    this->down();
  }
};


using Storage = ::testing::Types<std::shared_mutex, SpinlockRW<>>;
TYPED_TEST_SUITE(SpinlockRWTest, Storage);


TYPED_TEST(SpinlockRWTest, smoke) {
  this->start();
  this->down();
  ASSERT_EQ(this->data, 0);
  ASSERT_EQ(this->iter_counter(), this->r_iters * this->readers + this->w_iters * this->writers);
}


}// namespace