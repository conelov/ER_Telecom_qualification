#include <cassert>
#include <numeric>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <net_utils/RcuStorage.hpp>


namespace {


std::size_t constexpr num_readers = 10;
std::size_t constexpr num_writers = 3;
std::size_t constexpr iterations  = 100'000;
using Counts                      = std::array<std::uintmax_t, num_writers>;


template<typename T_>
class MultiThreadedTest : public ::testing::Test {
private:
  std::vector<std::thread> threads_;

protected:
  T_ shared_data;

protected:
  void SetUp() override {
    threads_.clear();
    shared_data = {};
  }


  void TearDown() override {
    for (auto& thread : threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  }


  template<typename Fn>
  void emplace_worker(Fn&& fn) {
    threads_.emplace_back(std::forward<Fn>(fn));
  }
};


using Storage = ::testing::Types<nut::RcuStorage<Counts>>;
TYPED_TEST_SUITE(MultiThreadedTest, Storage);


TYPED_TEST(MultiThreadedTest, smoke) {
  Counts counts{};

  for (std::size_t i = 0; i < num_readers; ++i) {
    this->emplace_worker([this] {
      for (std::size_t i = 0; i < iterations; ++i) {
        if (!*this->shared_data) {
          return;
        }
        [[maybe_unused]] auto v = **this->shared_data;
      }
    });
  }


  for (std::size_t i = 0; i < num_writers; ++i) {
    this->emplace_worker([this, &c = counts[i], i] {
      for (std::size_t j = 0; j < iterations; ++j) {
        this->shared_data.modify([&c, i](auto ptr) {
          if (!ptr) {
            ptr = std::make_shared<typename decltype(this->shared_data)::value_type>();
          }
          ptr->at(i) = ++c;
          return std::move(ptr);
        });
      }
    });
  }

  this->TearDown();
  ASSERT_EQ(**this->shared_data, counts);
}


}// namespace