#include <gtest/gtest.h>

#include <net_utils/aux/DnsCacheFixture.hpp>


namespace {


using namespace nut;


TEST(DnsCacheTest_smoke, smoke) {
  DnsCacheImpl<DnsCacheImplType::lru_std_mx> c{1};
  ASSERT_EQ(c.resolve("www.google.com"), "");
  ASSERT_NO_THROW(c.update("www.google.com", "0.0.0.0"));
  ASSERT_EQ(c.resolve("www.google.com"), "0.0.0.0");
}

template<typename>
class DnsCacheTest;


template<DnsCacheImplType type>
class DnsCacheTest<std::integral_constant<DnsCacheImplType, type>>
    : public ::testing::Test
    , public aux::DnsCacheFixture<type> {
protected:
  static auto constexpr r_iters              = 1'000'000;
  static auto constexpr w_iters              = 10'000;
  static auto constexpr w_exclusive_relation = 5. / 10;
  static_assert(w_exclusive_relation >= 0);
  static_assert(w_exclusive_relation <= 1);

protected:
  void SetUp() override {
    this->set_rw_relation(1. / 2);
    this->readers *= 2;
    this->writers = this->writers * 2;
    if constexpr (type == DnsCacheImplType::lru_std_mx
      || type == DnsCacheImplType::lru_spinlock_rw) {
      this->up(r_iters, w_iters, 100'000);
    } else {
      this->up(r_iters, w_iters, 100'000, 200'000);
    }
  }


  void TearDown() override {
    this->down();
  }
};


using Storage = ::testing::Types<
  std::integral_constant<DnsCacheImplType, DnsCacheImplType::rcu_std_mx>,
  // std::integral_constant<DnsCacheImplType, DnsCacheImplType::rcu_spinlock_rw>,
  std::integral_constant<DnsCacheImplType, DnsCacheImplType::lru_std_mx>
  // std::integral_constant<DnsCacheImplType, DnsCacheImplType::lru_spinlock_rw>
  >;
TYPED_TEST_SUITE(DnsCacheTest, Storage);


TYPED_TEST(DnsCacheTest, smoke) {
  this->start();
  this->down();
}

}// namespace
