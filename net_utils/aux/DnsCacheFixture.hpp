#pragma once

#include <net_utils/DnsCache.hpp>

#include <net_utils/aux/MultiThreadedFixture.hpp>


namespace nut {


class DnsCacheFixture : public MultiThreadedFixture {
public:
  void compute() {
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


  void up(std::size_t cache_size) {
    DnsCache::init(cache_size);
  }


  void down() override {
    MultiThreadedFixture::down();
    DnsCache::destroy();
  }
};


}// namespace nut