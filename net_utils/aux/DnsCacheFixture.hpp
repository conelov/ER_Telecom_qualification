#pragma once

#include <optional>

#include <net_utils/DnsCacheImpl.hpp>

#include <net_utils/aux/MultiThreadedRWValuedFixture.hpp>


namespace nut::aux {


template<DnsCacheImplType type>
class DnsCacheFixture : public MultiThreadedRWValuedFixture<DnsCacheImpl<type>> {
public:
  void bind(std::size_t cache_size) {
    using Base       = MultiThreadedRWValuedFixture<DnsCacheImpl<type>>;
    this->value_ctor = [cache_size](typename Base::ValueOpt& opt) {
      opt.emplace(cache_size);
    };

    auto str_gen = [this](std::size_t idx, std::size_t i) {
      return std::to_string(idx * this->writers + i);
    };

    Base::bind(
      [this, str_gen](auto... args) mutable {
        auto const volatile dummy = this->value->resolve(str_gen(args...));
        // [[maybe_unused]] auto const volatile c = dummy.front();
      },

      [this, str_gen, idx_str = std::string{}](auto idx, std::size_t i) mutable {
        if (idx_str.empty()) {
          idx_str = std::to_string(idx);
        }
        this->value->update(str_gen(idx, i), idx_str);
      });
  }
};


}// namespace nut::aux