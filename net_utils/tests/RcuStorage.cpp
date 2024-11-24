#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <net_utils/RcuStorage.hpp>
#include <net_utils/SpinlockRW.hpp>

#include <net_utils/aux/RcuStorageFixture.hpp>


namespace {


using namespace nut;


template<typename Mx_>
class RcuStorageTest
    : public ::testing::Test
    , public aux::RcuStorageFixture<Mx_> {
protected:
  static auto constexpr r_iters              = 100'000;
  static auto constexpr w_iters              = 10'000;
  static auto constexpr w_exclusive_relation = 5. / 10;
  static_assert(w_exclusive_relation >= 0);
  static_assert(w_exclusive_relation <= 1);

protected:
  void SetUp() override {
    this->set_rw_relation(1. / 2);
    this->readers *= 2;
    this->writers = this->writers * 2;
    this->up(r_iters, w_iters, this->writers * w_exclusive_relation);
  }


  void TearDown() override {
    this->down();
  }
};


using Storage = ::testing::Types<std::shared_mutex, SpinlockRW<>>;
TYPED_TEST_SUITE(RcuStorageTest, Storage);


TYPED_TEST(RcuStorageTest, smoke) {
  this->start();
  this->down();
  EXPECT_THAT(*this->storage().load(), testing::ElementsAreArray(std::vector(this->writers, this->w_iters)));
}


}// namespace