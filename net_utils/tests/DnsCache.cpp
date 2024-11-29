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
class DnsCacheTest<aux::DnsCacheImplTypeC<type>>
    : public ::testing::Test
    , public aux::DnsCacheFixture<type> {
protected:
  void SetUp() override {
    this->read_iters  = 100'000;
    this->write_iters = 10'000;
    this->set_rw_relation(1. / 2);
    this->bind(1000);
  }
};


using Storage = ::testing::Types<
  std::integral_constant<DnsCacheImplType, DnsCacheImplType::rcu_std_mx>,
  std::integral_constant<DnsCacheImplType, DnsCacheImplType::rcu_priority_mutex>,
  aux::DnsCacheImplTypeC<DnsCacheImplType::lru_std_mx>,
  aux::DnsCacheImplTypeC<DnsCacheImplType::lru_priority_mutex>>;
TYPED_TEST_SUITE(DnsCacheTest, Storage);


TYPED_TEST(DnsCacheTest, high_load) {
  ASSERT_NO_THROW(this->iteration());
}


}// namespace
