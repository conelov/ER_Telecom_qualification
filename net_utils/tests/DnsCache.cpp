#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include <net_utils/DnsCache.hpp>

#include <MultiThreadedFixture.hpp>


using namespace nut;


namespace {


class DnsCacheTest : public MultiThreadedTest {
protected:
  void SetUp(std::size_t n) {
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
  auto constexpr cache_size = 10;
  SetUp(cache_size);

  auto const str_gen = [str = std::string{"000"}, s = std::uint16_t{0}]() mutable {
    str.at(s) == '9' ? str[s] = '0' : ++str[s];
    if (++s > 2) {
      s = 0;
    }
    return str;
  };

  for (std::size_t i = 0; i < 5; ++i) {
    emplace_worker([gen = str_gen, idx = std::to_string(i)]() mutable {
      DnsCache::update(gen(), idx);
    });
  }

  for (std::size_t i = 0; i < 25; ++i) {
    emplace_worker([gen = str_gen]() mutable {
      [[maybe_unused]] auto const dummy = DnsCache::resolve(gen());
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    });
  }
}


}// namespace
