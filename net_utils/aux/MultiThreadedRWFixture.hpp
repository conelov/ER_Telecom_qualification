#pragma once

#include <cmath>

#include <net_utils/utils.hpp>

#include <net_utils/aux/IterationRate.hpp>
#include <net_utils/aux/MultiThreadedFixture.hpp>


namespace nut::aux {


struct MultiThreadedRWFixturePayloadMixin : public MultiThreadedFixturePayloadMixin {
  enum Type : std::uint8_t {
    reader,
    writer,
  } type;
};


class MultiThreadedRWFixture
    : public MultiThreadedFixture<MultiThreadedRWFixturePayloadMixin> {
public:
  using Payload = MultiThreadedRWFixturePayloadMixin;

public:
  std::size_t readers     = 0;
  std::size_t writers     = 0;
  std::size_t read_iters  = 0;
  std::size_t write_iters = 0;

public:
  void set_rw_relation(float rw_relation, std::size_t ths = NET_UTILS_CPU_COUNT) {
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
      // ReSharper disable once CppSomeObjectMembersMightNotBeInitialized
      Payload payload;
      payload.rate.set_resolution(read_iters / 100);
      payload.type       = Payload::reader;
      payload.iterations = read_iters;
      emplace_worker(bind_front(r, i), std::move(payload));
    }

    for (std::size_t i = 0; i < writers; ++i) {
      // ReSharper disable once CppSomeObjectMembersMightNotBeInitialized
      Payload payload;
      payload.rate.set_resolution(write_iters / 100);
      payload.type       = Payload::writer;
      payload.iterations = write_iters;
      emplace_worker(bind_front(w, i), std::move(payload));
    }
  }
};


}// namespace nut::aux