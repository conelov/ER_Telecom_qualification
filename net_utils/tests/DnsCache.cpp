#include <gtest/gtest.h>

#include <net_utils/aux/DnsCacheFixture.hpp>


namespace {


using namespace nut;


TEST(DnsCacheTest_smoke, smoke) {
  aux::DnsCacheImplLRU c{1};
  ASSERT_EQ(c.resolve("www.google.com"), "");
  ASSERT_NO_THROW(c.update("www.google.com", "0.0.0.0"));
  ASSERT_EQ(c.resolve("www.google.com"), "0.0.0.0");
}


class DnsCacheTest
    : public ::testing::Test
    , public aux::DnsCacheFixture {
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
    this->up(r_iters, w_iters, 100'000);
  }


  void TearDown() override {
    this->down();
  }
};


TEST_F(DnsCacheTest, high_load) {
  this->start();
  this->down();
}


}// namespace
