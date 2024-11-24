#pragma once

#include <optional>

#include <net_utils/DnsCacheImpl.hpp>

#include <net_utils/aux/MultiThreadedRWFixture.hpp>


namespace nut::aux {


class DnsCacheFixture : public MultiThreadedRWFixture {
public:
  template<typename... Args>
  void up(std::size_t r_iters, std::size_t w_iters, Args&&... args) {
    cache_.emplace(std::forward<Args>(args)...);

    auto str_gen = [this, i = std::size_t{0}](std::size_t idx) mutable {
      return std::to_string(idx * writers + i++);
    };

    MultiThreadedRWFixture::up(
      r_iters,
      w_iters,

      [this, str_gen](std::size_t idx) mutable {
        auto const volatile dummy = cache_->resolve(str_gen(idx));
        // [[maybe_unused]] auto const volatile c = dummy.front();
      },

      [this, str_gen, idx_str = std::string{}](auto idx) mutable {
        if (idx_str.empty()) {
          idx_str = std::to_string(idx);
        }
        cache_->update(str_gen(idx), idx_str);
      });
  }


  void down() override {
    MultiThreadedRWFixture::down();
    cache_.reset();
  }

private:
  std::optional<DnsCacheImplLRU> cache_;
};


}// namespace nut::aux