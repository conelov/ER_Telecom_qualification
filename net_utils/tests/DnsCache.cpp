#include <gtest/gtest.h>

#include <net_utils/DnsCache.hpp>


using namespace nut;


namespace {


TEST(DnsCache, smoke) {
  ASSERT_EQ(DnsCache::resolve("www.google.com"), "");
  ASSERT_NO_THROW(DnsCache::update("www.google.com", "0.0.0.0"));
  ASSERT_EQ(DnsCache::resolve("www.google.com"), "0.0.0.0");
}


}// namespace


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  DnsCache::init(std::size_t{CACHE_LIMIT});

  return RUN_ALL_TESTS();
}
