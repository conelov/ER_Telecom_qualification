#include <gtest/gtest.h>

#include <net_utils/aux/DnsCacheFixture.hpp>


using namespace nut;


namespace {


TEST(DnsCacheTest, smoke) {
  DnsCacheImpl c{1};
  ASSERT_EQ(c.resolve("www.google.com"), "");
  ASSERT_NO_THROW(c.update("www.google.com", "0.0.0.0"));
  ASSERT_EQ(c.resolve("www.google.com"), "0.0.0.0");
}


class DnsCacheTestF
    : public ::testing::Test
    , public DnsCacheFixture {
private:
  void TearDown() override {
    down();
  }
};


TEST_F(DnsCacheTestF, high_load) {
  iterations = 500;

  auto constexpr cache_size = 2'500;
  up(cache_size);

  ASSERT_NO_THROW(compute());
  SUCCEED();
}


}// namespace
