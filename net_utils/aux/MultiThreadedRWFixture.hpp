#pragma once

#include <cmath>

#include <net_utils/utils.hpp>

#include <net_utils/aux/MultiThreadedFixture.hpp>


namespace nut::aux {


class MultiThreadedRWFixture : public MultiThreadedFixture {
public:
  template<typename RGen, typename WGen>
  void up(float rw_relation, std::size_t r_iters, std::size_t w_iters, RGen&& rgen, WGen&& wgen) {
    MultiThreadedFixture::up();

    assert(rw_relation >= 0);
    assert(rw_relation <= 1);
    readers_ = std::round(NUT_CPU_COUNT * rw_relation);
    writers_ = NUT_CPU_COUNT - readers_;

    for (std::size_t i = 0; i < writers_; ++i) {
      emplace_worker(w_iters, carry(wgen, i, writers_));
    }

    for (std::size_t i = 0; i < readers_; ++i) {
      emplace_worker(r_iters, carry(rgen, i, readers_));
    }
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
};


}// namespace nut