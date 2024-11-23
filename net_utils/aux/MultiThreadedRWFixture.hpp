#pragma once

#include <cmath>

#include <net_utils/utils.hpp>

#include <net_utils/aux/MultiThreadedFixture.hpp>


namespace nut::aux {


class MultiThreadedRWFixture : public MultiThreadedFixture {
public:
  std::size_t readers;
  std::size_t writers;

public:
  template<typename RGen, typename WGen>
  void up(std::size_t r_iters, std::size_t w_iters, RGen&& rgen, WGen&& wgen) {
    MultiThreadedFixture::up();

    for (std::size_t i = 0; i < writers; ++i) {
      emplace_worker(w_iters, carry(wgen, i, writers));
    }

    for (std::size_t i = 0; i < readers; ++i) {
      emplace_worker(r_iters, carry(rgen, i, readers));
    }
  }


  void down() override {
    MultiThreadedFixture::down();
  }


  void set_rw_relation(float rw_relation) {
    assert(rw_relation >= 0);
    assert(rw_relation <= 1);
    readers = std::round(NUT_CPU_COUNT * rw_relation);
    writers = NUT_CPU_COUNT - readers;
  }
};


}// namespace nut::aux