#pragma once

#include <memory>

#include <net_utils/LruStorage.hpp>
#include <net_utils/RcuStorage.hpp>
#include <net_utils/utils.hpp>


namespace nut {


template<typename Lru_, typename Lock_>
class RcuLruStorage final : public RcuStorage<Lru_, Lock_> {
public:
  using Lru      = Lru_;
  using RcuUnder = RcuStorage<Lru, Lock_>;
  using ReadPtr  = std::shared_ptr<typename Lru::value_type const>;

public:
  RcuLruStorage(std::size_t cache_size)
      : RcuUnder{std::make_shared<Lru>(cache_size)} {
  }


  template<typename Key>
  [[nodiscard]] ReadPtr get(Key&& key) noexcept {
    ReadPtr out;
    this->modify([&out, &key](auto lru_ptr) {
      out = ReadPtr{lru_ptr, lru_ptr->get(std::forward<Key>(key))};
      return std::move(lru_ptr);
    });
    return out;
  }


  template<typename Key, typename... VArgs>
  void put(Key&& key, VArgs&&... vargs) {
    this->modify(bind_back([&key](auto lru_ptr, auto&&... vargs) {
      lru_ptr->put(std::forward<Key>(key), std::move(vargs)...);
      return std::move(lru_ptr);
    },
      std::forward<VArgs>(vargs)...));
  }
};


}// namespace nut