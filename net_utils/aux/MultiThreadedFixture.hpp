#pragma once

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include <net_utils/utils.hpp>


namespace nut::aux {


class MultiThreadedFixture {
public:
  virtual ~MultiThreadedFixture() = default;


  auto start() {
    pre_start();
    assert(!run_);
    run_ = true;
    cv_.notify_one();
    return finally([this] {
      stop();
    });
  }


  template<typename Fn>
  void emplace_worker(std::size_t iters, Fn&& fn) {
    assert(iters > 0);
    threads_.emplace_back(
      [iters, this](auto fn) {
        {
          // wait start
          std::unique_lock lk{mx_};
          cv_.wait(lk, [this]() -> bool { return run_; });
        }
        cv_.notify_one();

        for (std::size_t i = 0; i < iters; ++i) {
          fn(i);
        }
      },
      std::forward<Fn>(fn));
  }


private:
  void stop() {
    assert(!!run_);
    for (auto& thread : threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    threads_.clear();
    run_ = false;
    post_stop();
  }


  virtual void post_stop() {
  }


  virtual void pre_start() {
  }


private:
  std::vector<std::thread> threads_;
  std::mutex               mx_;
  std::condition_variable  cv_;
  std::atomic<bool>        run_ = false;
};


}// namespace nut::aux