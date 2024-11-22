#pragma once

#include <thread>
#include <vector>


namespace nut {


class MultiThreadedFixture {
public:
  std::size_t iterations = iterations_default;

public:
  virtual ~MultiThreadedFixture() {
    down();
  }


  virtual void down() {
    for (auto& thread : threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    threads_.clear();
    iterations = iterations_default;
  }


  template<typename Fn>
  void emplace_worker(Fn&& fn) {
    threads_.emplace_back(
      [this](auto fn) {
        for (std::size_t i = 0; i < iterations; ++i) {
          fn();
        }
      },
      std::forward<Fn>(fn));
  }

private:
  static std::size_t constexpr iterations_default = 50'000;

private:
  std::vector<std::thread> threads_;
};


}// namespace nut