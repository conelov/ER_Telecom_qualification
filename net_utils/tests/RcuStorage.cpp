#include <cassert>
#include <iostream>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <net_utils/RcuStorage.hpp>


namespace {


template<typename T_>
class MultiThreadedTest : public ::testing::Test {
protected:
  static std::size_t constexpr num_readers = 10;
  static std::size_t constexpr num_writers = 3;

private:
  static std::size_t constexpr iterations_ = 100'000;

private:
  std::vector<std::thread> threads_;
  std::shared_mutex        rw_lock_;

protected:
  T_ shared_data;

protected:
  void SetUp() override {
    assert(threads_.empty());
    shared_data = {};
  }


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


using Storage = ::testing::Types<nut::RcuStorage<int>>;
TYPED_TEST_SUITE(MultiThreadedTest, Storage);


TYPED_TEST(MultiThreadedTest, smoke) {
  for (std::size_t i = 0; i < this->num_readers; ++i) {
    this->emplace_worker([this] {
      auto const i = this->shared_data.load();
    });
  }


  for (std::size_t i = 0; i < this->num_writers; ++i) {
    this->emplace_worker([this] {
      this->shared_data.modify(
        [](auto const ptr) {

        },
        [](auto const new_p, auto const old_p) {

        });
    });
  }
}


}// namespace
