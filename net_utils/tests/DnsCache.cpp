#include <gtest/gtest.h>

#include <net_utils/aux/DnsCacheFixture.hpp>


namespace {


using namespace nut;


TEST(DnsCacheTest, smoke) {
  DnsCacheImpl<DnsCacheImplType::lru_std_mx> c{1};
  ASSERT_EQ(c.resolve("www.google.com"), "");
  ASSERT_NO_THROW(c.update("www.google.com", "0.0.0.0"));
  ASSERT_EQ(c.resolve("www.google.com"), "0.0.0.0");
}

//
// class DnsCacheTestF
//     : public ::testing::Test
//     , public DnsCacheFixture {
// protected:
//   void SetUp() override {
//     up(1. / 2, 10'000, 10'000, 100, 125);
//   }
//
//
//   void TearDown() override {
//     down();
//   }
// };
//
//
// TEST_F(DnsCacheTestF, high_load) {
//   ASSERT_NO_THROW(start());
//   SUCCEED();
// }


}// namespace
