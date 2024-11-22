#pragma once

#include <optional>

#include <net_utils/DnsCacheImpl.hpp>

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
      emplace_worker([this, gen = str_gen(iterations * i), idx = std::to_string(i)]() mutable {
        cache_->update(gen(), idx);
      });
    }

    for (std::size_t i = 0; i < 1; ++i) {
      emplace_worker([this, gen = str_gen(0)]() mutable {
        [[maybe_unused]] auto const dummy = cache_->resolve(gen());
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::microseconds(10));
      });
    }
  }


  void up(std::size_t cache_size) {
    cache_.emplace(cache_size);
  }


  void down() override {
    MultiThreadedFixture::down();
    cache_.reset();
  }

private:
  std::optional<DnsCacheImpl> cache_;
};


}// namespace nut