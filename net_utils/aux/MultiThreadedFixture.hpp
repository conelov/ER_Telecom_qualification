#pragma once

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <thread>
#include <vector>

#include <net_utils/ConcurrencyUtils.hpp>
#include <net_utils/utils.hpp>


namespace nut::aux {


class MultiThreadedFixture {
public:
  virtual ~MultiThreadedFixture() {
    reset();
  }


  template<typename Payload, typename... PArgs>
  std::shared_ptr<Payload> emplace_worker(PArgs&&... pargs) {
    auto const pp = std::make_shared<Payload>(std::forward<PArgs>(pargs)...);
    nodes_.emplace_back(INVOKER_AS_LAMBDA(runner, this), pp);
    return pp;
  }


  template<typename Fn, typename PostFn>
  auto emplace_worker(std::size_t iters, Fn&& fn, PostFn&& post) {
    struct Payload final {
      Fn          fn;
      PostFn      post;
      std::size_t iterations;
    };
    return emplace_worker<Payload>(Payload{std::forward<Fn>(fn), std::forward<PostFn>(post), iters});
  }


  auto start() {
    pre_start();
    quit_flag_.store(false, std::memory_order_relaxed);
    barrier_.reset(nodes_.size());
    barrier_.consumers_run();
    return finally(WRAP_IN_LAMBDA(stop(), this));
  }

private:
  template<typename PayloadPtr>
  void runner(PayloadPtr payload) {
    auto const lk = barrier_.consumer_wait();
    for (std::size_t i = 0; i < payload->iterations; ++i) {
      payload->fn(i);
      payload->post();
    }
  }


  void stop() {
    barrier_.producer_wait();
    post_stop();
  }


  virtual void post_stop() {
  }


  virtual void pre_start() {
  }


  void reset() {
    quit_flag_.store(true, std::memory_order_relaxed);
    barrier_.producer_wait();
    for (auto& thread : nodes_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    nodes_.clear();
  }

public:
  enum class WorkerStage : std::uint8_t {
    idle,
    run,
    exit,
  };

private:
  std::vector<std::thread> nodes_;
  CrossBarrier             barrier_;
  std::atomic<bool>        quit_flag_ = true;
};


}// namespace nut::aux