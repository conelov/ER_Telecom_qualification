#pragma once

#include <vector>

#include <net_utils/RcuLruStorage.hpp>
#include <net_utils/RcuStorage.hpp>

#include <net_utils/aux/MultiThreadedRWValuedFixture.hpp>


namespace nut::aux {


struct RcuStorageFixtureBaseTag {};

template<typename Mx>
using RcuStorageFixtureType = RcuStorage<std::vector<std::uintmax_t>, Mx>;

template<typename Mx_>
class RcuStorageFixture
    : public MultiThreadedRWValuedFixture<RcuStorageFixtureType<Mx_>>
    , public RcuStorageFixtureBaseTag {
public:
  void bind() {
    using Base       = MultiThreadedRWValuedFixture<RcuStorageFixtureType<Mx_>>;
    this->value_ctor = MEM_FN_LAMBDA(.emplace(this->writers, 0u), this);

    Base::bind(
      [this](std::size_t idx, auto...) {
        auto const a_ptr    = this->value->load();
        auto volatile dummy = a_ptr->at(idx % this->writers);
        ++dummy;
      },

      [this](std::size_t idx, auto...) {
        this->value->modify([idx](auto a_ptr) {
          ++a_ptr->at(idx);
          return a_ptr;
        });
      });
  }
};


struct RcuLruStorageFixtureBaseTag {};

template<typename Mx>
using RcuLruStorageFixtureType = RcuLruStorage<LruStorage<std::uintmax_t, std::size_t>, Mx>;

template<typename Mx_>
class RcuLruStorageFixture
    : public MultiThreadedRWValuedFixture<RcuLruStorageFixtureType<Mx_>>
    , RcuLruStorageFixtureBaseTag {
protected:
  std::size_t cache_size = 0;

protected:
  void bind() {
    using Base       = MultiThreadedRWValuedFixture<RcuLruStorageFixtureType<Mx_>>;
    this->value_ctor = MEM_FN_LAMBDA(.emplace(cache_size), this);

    Base::bind(
      [this](std::size_t idx, auto...) {
        auto dummy_ptr  = this->value->get(idx);
        auto volatile i = *dummy_ptr;
        ++i;
      },

      [this](std::size_t idx, std::size_t iter) {
        this->value->put(idx, iter);
      });
  }
};


}// namespace nut::aux