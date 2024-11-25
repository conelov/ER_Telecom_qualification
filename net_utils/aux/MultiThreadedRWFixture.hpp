#pragma once

#include <cmath>

#include <net_utils/utils.hpp>

#include <net_utils/aux/MultiThreadedFixture.hpp>


namespace nut::aux {


class MultiThreadedRWFixture : public MultiThreadedFixture {
public:
  std::size_t readers;
  std::size_t writers;
  std::size_t read_iters;
  std::size_t write_iters;

public:
  template<typename R, typename W>
  void bind(R&& r, W&& w) {
    for (std::size_t i = 0; i < writers; ++i) {
      emplace_worker(write_iters, carry(w, i));
    }

    for (std::size_t i = 0; i < readers; ++i) {
      emplace_worker(read_iters, carry(r, i));
    }
  }


  void set_rw_relation(float rw_relation, std::size_t ths = NUT_CPU_COUNT) {
    assert(rw_relation >= 0);
    assert(rw_relation <= 1);
    readers = std::round(ths * rw_relation);
    writers = ths - readers;
  }
};


}// namespace nut::aux