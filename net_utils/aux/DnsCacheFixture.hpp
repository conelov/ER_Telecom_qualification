#pragma once

#include <cmath>
#include <optional>

#include <net_utils/DnsCacheImpl.hpp>
#include <net_utils/utils.hpp>

#include <net_utils/aux/MultiThreadedRWFixture.hpp>


namespace nut::aux {


template<DnsCacheImplType type>
class DnsCacheFixture : public MultiThreadedRWFixture {
public:
  template<typename... Args>
  void up(float rw_relation, std::size_t r_iters, std::size_t w_iters, Args&&... args) {
    cache_.emplace(std::forward<Args>(args)...);

    auto const str_gen = [](std::size_t start) {
      return [i = start]() mutable {
        return std::to_string(i++);
      };
    };

    MultiThreadedRWFixture::up(
      rw_relation,
      r_iters,
      w_iters,
      [this, str_gen](std::size_t idx, auto...) {
        return [this, gen = str_gen(idx)]() mutable {
          [[maybe_unused]] auto const volatile dummy = cache_->resolve(gen());
        };
      },
      [this, str_gen](auto idx, auto count) {
        return [this, gen = str_gen(idx * count), idx = std::to_string(idx)]() mutable {
          cache_->update(gen(), idx);
        };
      });
  }


  void down() override {
    MultiThreadedFixture::down();
    cache_.reset();
  }

private:
  std::optional<DnsCacheImpl> cache_;
};


}// namespace nut