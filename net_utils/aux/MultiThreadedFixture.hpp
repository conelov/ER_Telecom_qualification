#pragma once

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>


namespace nut::aux {
class MultiThreadedFixture {
public:
  virtual ~MultiThreadedFixture() {
    MultiThreadedFixture::down();
  }


  virtual void up() {
    run_.store(false, std::memory_order_release);
  }


  virtual void down() {
    for (auto& thread : threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    threads_.clear();
  }


  virtual void start() {
    assert(!run_.load(std::memory_order_acquire));
    run_.store(true, std::memory_order_release);
    cv_.notify_one();
  }


  template<typename Fn>
  void emplace_worker(std::size_t iters, Fn&& fn) {
    assert(iters > 0);
    threads_.emplace_back(
      [iters, this](auto fn) {
        {
          // wait start
          std::unique_lock lk{mx_};
          cv_.wait(lk, [this] { return run_.load(std::memory_order_acquire); });
        }
        cv_.notify_one();

        for (std::size_t i = 0; i < iters; ++i) {
          fn();
        }
      },
      std::forward<Fn>(fn));
  }


private:
  std::vector<std::thread> threads_;
  std::mutex               mx_;
  std::condition_variable  cv_;
  std::atomic<bool>        run_;
};


}// namespace nut::aux