#pragma once

#include <cassert>
#include <functional>

#include <net_utils/ConcurrencyUtils.hpp>

#include <net_utils/aux/IterationRate.hpp>


namespace nut::aux {


struct MultiThreadedFixturePayloadMixin {
  IterationRate<> rate;
  std::size_t     iterations;


  void pre_run() noexcept {
    rate.reset();
  }


  void post_run() noexcept {
    rate.cut();
  }


  void iteration_post() noexcept {
    ++rate;
  }
};


template<typename Payload_ = MultiThreadedFixturePayloadMixin>
class MultiThreadedFixture {
public:
  using PayloadPtr    = std::shared_ptr<Payload_ const>;
  using PayloadsArray = std::vector<PayloadPtr>;

public:
  virtual ~MultiThreadedFixture() {
    stop();
  }


  void bind() {
    stop();
  }


  template<typename Fn, typename... PArgs>
  PayloadPtr emplace_worker(Fn&& fn, PArgs&&... pargs) {
    auto pp = std::make_shared<Payload_>(std::forward<PArgs>(pargs)...);
    // clang-format off
    broker_.bind(bind_front([pp](auto&& fn) {
      pp->pre_run();
      for (std::size_t i = 0; i < pp->iterations; ++i) {
        fn(i);
        pp->iteration_post();
      }
      pp->post_run();
    },
    std::forward<decltype(fn)>(fn)));
    // clang-format off
    payloads_array_.push_back(pp);
    return pp;
  }


  void iteration() {
    begin_iteration();
    broker_.next();
    end_iteration();
  }


  [[nodiscard]] PayloadsArray const& payloads() const noexcept {
    return payloads_array_;
  }

private:
  virtual void begin_iteration() {
  }


  virtual void end_iteration() {
  }


  virtual void pre_stop() {
  }


  void stop() {
    pre_stop();
    broker_.next(true);
    payloads_array_.clear();
  }

private:
  PayloadsArray                    payloads_array_;
  TaskPool<std::function<void()>> broker_ {NET_UTILS_CPU_COUNT};
};


}// namespace nut::aux