#include <gtest/gtest.h>

#include <net_utils/DnsCache.hpp>

#include <MultiThreadedFixture.hpp>


using namespace nut;


namespace {


class DnsCacheTest : public MultiThreadedTest {
protected:
  void SetUp(std::size_t n) {
    MultiThreadedTest::SetUp();
    DnsCache::init(n);
  }


  void TearDown() override {
    MultiThreadedTest::TearDown();
    DnsCache::destroy();
  }
};


TEST_F(DnsCacheTest, smoke) {
  SetUp(1);
  ASSERT_EQ(DnsCache::resolve("www.google.com"), "");
  ASSERT_NO_THROW(DnsCache::update("www.google.com", "0.0.0.0"));
  ASSERT_EQ(DnsCache::resolve("www.google.com"), "0.0.0.0");
}


TEST_F(DnsCacheTest, high_load) {
}


}// namespace
