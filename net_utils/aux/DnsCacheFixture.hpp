#pragma once

#include <cmath>
#include <optional>

#include <net_utils/DnsCacheImpl.hpp>

#include <net_utils/aux/MultiThreadedFixture.hpp>


namespace nut {


class DnsCacheFixture : public MultiThreadedFixture {
public:
  float rw_relation = 1. / 2;

public:
  void compute() {
    auto const str_gen = [](std::size_t start) {
      return [i = start]() mutable {
        return std::to_string(i++);
      };
    };

    for (std::size_t i = 0; i < writers_; ++i) {
      emplace_worker([this, gen = str_gen(iterations * i), idx = std::to_string(i)]() mutable {
        cache_->update(gen(), idx);
      });
    }

    for (std::size_t i = 0; i < readers_; ++i) {
      emplace_worker([this, gen = str_gen(0)]() mutable {
        [[maybe_unused]] auto const dummy = cache_->resolve(gen());
        std::this_thread::yield();
      });
    }
  }


  template<typename... Args>
  void up(Args&&... args) {
    cache_.emplace(std::forward<Args>(args)...);
    assert(rw_relation >= 0);
    assert(rw_relation <= 1);
    readers_ = std::round(NUT_CPU_COUNT * rw_relation);
    writers_ = NUT_CPU_COUNT - readers_;
  }


  void down() override {
    MultiThreadedFixture::down();
    cache_.reset();
  }


  [[nodiscard]] std::size_t readers() const {
    return readers_;
  }


  [[nodiscard]] std::size_t writers() const {
    return writers_;
  }

private:
  std::size_t readers_;
  std::size_t writers_;

private:
  std::optional<DnsCacheImpl> cache_;
};


}// namespace nut