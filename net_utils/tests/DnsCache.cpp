#include <gtest/gtest.h>

#include <net_utils/aux/DnsCacheFixture.hpp>


using namespace nut;


namespace {


class DnsCacheTest
    : public ::testing::Test
    , public DnsCacheFixture {
private:
  void TearDown() override {
    down();
  }
};


TEST_F(DnsCacheTest, smoke) {
  up(1);
  ASSERT_EQ(DnsCache::resolve("www.google.com"), "");
  ASSERT_NO_THROW(DnsCache::update("www.google.com", "0.0.0.0"));
  ASSERT_EQ(DnsCache::resolve("www.google.com"), "0.0.0.0");
}


TEST_F(DnsCacheTest, high_load) {
  iterations = 500;

  auto constexpr cache_size = 2'500;
  up(cache_size);

  ASSERT_NO_THROW(compute());
  SUCCEED();
}


}// namespace
