#pragma once

#include <cassert>
#include <thread>
#include <vector>

#include <net_utils/ConcurrencyUtils.hpp>
#include <net_utils/utils.hpp>


namespace nut::aux {

template<typename Payload>
class MultiThreadedFixture;


template<typename Derived_>
class MultiThreadedFixturePayloadMixin {
public:
  std::size_t   iterations;
  IterationRate rate;

  void pre_run() noexcept {
    rate.reset();
  }

  void iteration_post() noexcept {
    ++rate;
  }


private:
  Derived_& cast() noexcept {
    return static_cast<Derived_&>(*this);
  }
};


template<typename Payload_>
class MultiThreadedFixture {
public:
  using PayloadPtr = std::shared_ptr<Payload_ const>;

  struct Node final {
    std::thread thread;
    PayloadPtr  payload;
  };

  using NodesArray = std::vector<Node>;

public:
  virtual ~MultiThreadedFixture() {
    stop();
  }


  void bind() {
    stop();
  }


  template<typename Fn, typename... PArgs>
  PayloadPtr emplace_worker(Fn&& fn, PArgs&&... pargs) {
    assert(!pc_.producer.is_running());
    auto pp = std::make_shared<Payload_>(std::forward<PArgs>(pargs)...);
    nodes_.emplace_back(
      Node{
        .thread{std::thread{INVOKER_AS_LAMBDA(runner, this), std::forward<Fn>(fn), pp}},
        .payload = pp,
      });
    return pp;
  }


  void iteration() {
    pre_start();
    pc_.producer.run(nodes_.size());
    pc_.producer.wait();
  }


  [[nodiscard]] NodesArray const& nodes() const noexcept {
    return nodes_;
  }

private:
  template<typename Fn, typename PayloadPtr>
  void runner(Fn&& fn, PayloadPtr payload) {
    pc_.consumer.execute([this, &fn, payload]() {
      for (std::size_t i = 0; i < payload->iterations; ++i) {
        fn(i);
        payload->post();
      }
    });
  }


  void end_iteration() {
    post_stop();
  }


  virtual void post_stop() {
  }


  virtual void pre_start() {
  }


  void stop() {
    pc_.producer.quit();
    for (auto& [thread, payload] : nodes_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    nodes_.clear();
  }

private:
  NodesArray     nodes_;
  ProduceConsume pc_;
};


}// namespace nut::aux