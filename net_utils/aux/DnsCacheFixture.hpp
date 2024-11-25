#pragma once

#include <optional>

#include <net_utils/DnsCacheImpl.hpp>

#include <net_utils/aux/MultiThreadedRWValuedFixture.hpp>


namespace nut::aux {


class DnsCacheFixture : public MultiThreadedRWValuedFixture<DnsCacheImplLRU> {
public:
  void bind(std::size_t cache_size) {
    value_ctor = [cache_size](ValueOpt& opt) {
      opt.emplace(cache_size);
    };

    auto str_gen = [this](std::size_t idx, std::size_t i) {
      return std::to_string(idx * writers + i);
    };

    MultiThreadedRWValuedFixture::bind(
      [this, str_gen](auto ... args) mutable {
        auto const volatile dummy = value->resolve(str_gen(args...));
        // [[maybe_unused]] auto const volatile c = dummy.front();
      },

      [this, str_gen, idx_str = std::string{}](auto idx, std::size_t i) mutable {
        if (idx_str.empty()) {
          idx_str = std::to_string(idx);
        }
        value->update(str_gen(idx, i), idx_str);
      });
  }
};


}// namespace nut::aux