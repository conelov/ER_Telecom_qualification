#pragma once

#include <cmath>

#include <net_utils/utils.hpp>

#include <net_utils/aux/MultiThreadedFixture.hpp>


namespace nut::aux {


class MultiThreadedRWFixture : public MultiThreadedFixture {
public:
  std::size_t readers     = 0;
  std::size_t writers     = 0;
  std::size_t read_iters  = 0;
  std::size_t write_iters = 0;

public:
  template<typename R, typename W>
  void bind(R&& r, W&& w) {
    assert(readers != 0);
    assert(writers != 0);
    assert(read_iters != 0);
    assert(write_iters != 0);

    for (std::size_t i = 0; i < writers; ++i) {
      emplace_worker(write_iters, bind_front(w, i));
    }

    for (std::size_t i = 0; i < readers; ++i) {
      emplace_worker(read_iters, bind_front(r, i));
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