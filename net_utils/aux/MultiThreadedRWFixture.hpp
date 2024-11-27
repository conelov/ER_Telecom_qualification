#pragma once

#include <cmath>

#include <net_utils/utils.hpp>

#include <net_utils/aux/IterationRate.hpp>
#include <net_utils/aux/MultiThreadedFixture.hpp>


namespace nut::aux {


struct MultiThreadedRWFixtureBase {
  struct Payload final {
    IterationRate rate;
    std::size_t   iterations;


    void post() noexcept {
      ++rate;
    }
  };
};


class MultiThreadedRWFixture
    : public MultiThreadedFixture<MultiThreadedRWFixtureBase::Payload>
    , public MultiThreadedRWFixtureBase {
public:
  std::size_t readers     = 0;
  std::size_t writers     = 0;
  std::size_t read_iters  = 0;
  std::size_t write_iters = 0;

public:
  void set_rw_relation(float rw_relation, std::size_t ths = NUT_CPU_COUNT) {
    assert(rw_relation >= 0);
    assert(rw_relation <= 1);
    readers = std::round(ths * rw_relation);
    writers = ths - readers;
  }


  template<typename R, typename W>
  void bind(R&& r, W&& w) {
    MultiThreadedFixture::bind();

    assert(readers != 0);
    assert(writers != 0);
    assert(read_iters != 0);
    assert(write_iters != 0);


    for (std::size_t i = 0; i < readers; ++i) {
      emplace_worker(bind_front(r, i), Payload{.iterations = read_iters});
    }

    for (std::size_t i = 0; i < writers; ++i) {
      emplace_worker(bind_front(w, i), Payload{.iterations = write_iters});
    }
  }
};


}// namespace nut::aux