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
  void SetUp() override {
    read_iters  = 100'000;
    write_iters = 10'000;
    this->set_rw_relation(1. / 2);
    this->readers *= 2;
    this->writers = this->writers * 2;
    bind(1000);
  }
};


TEST_F(DnsCacheTest, high_load) {
  ASSERT_NO_THROW(this->start());
}


}// namespace
