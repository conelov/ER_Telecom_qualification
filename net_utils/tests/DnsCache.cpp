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
  iterations = 10'000;

  auto constexpr cache_size = 2'500;
  SetUp(cache_size);

  auto const str_gen = [](std::size_t start) {
    return [i = start]() mutable {
      return std::to_string(i++);
    };
  };

  for (std::size_t i = 0; i < 23; ++i) {
    emplace_worker([gen = str_gen(iterations * i), idx = std::to_string(i)]() mutable {
      DnsCache::update(gen(), idx);
    });
  }

  for (std::size_t i = 0; i < 1; ++i) {
    emplace_worker([gen = str_gen(0)]() mutable {
      [[maybe_unused]] auto const dummy = DnsCache::resolve(gen());
      std::this_thread::yield();
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    });
  }
}


}// namespace
