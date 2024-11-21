#pragma once

#include <thread>
#include <vector>

#include <gtest/gtest.h>


namespace nut {


class MultiThreadedTest : public ::testing::Test {
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
    threads_.emplace_back(std::forward<Fn>(fn));
  }
};


}// namespace nut