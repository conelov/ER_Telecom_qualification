#pragma once

#include <thread>
#include <vector>

#include <gtest/gtest.h>


namespace nut {


class MultiThreadedTest : public ::testing::Test {
protected:
  static std::size_t constexpr iterations = 50'000;

private:
  std::vector<std::thread> threads_;

protected:
  void TearDown() override {
    for (auto& thread : threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    threads_.clear();
  }


  template<typename Fn>
  void emplace_worker(Fn&& fn) {
    threads_.emplace_back(
      [](auto fn) {
        for (std::size_t i = 0; i < iterations; ++i) {
          fn();
        }
      },
      std::forward<Fn>(fn));
  }
};


}// namespace nut