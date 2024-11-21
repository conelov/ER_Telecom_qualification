#include <cassert>
#include <numeric>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <net_utils/RcuStorage.hpp>

#include <MultiThreadedFixture.hpp>


using namespace nut;


namespace {


std::size_t constexpr num_readers = 10;
std::size_t constexpr num_writers = 3;
std::size_t constexpr iterations  = 100'000;
using Counts                      = std::array<std::uintmax_t, num_writers>;


template<typename T_>
class RcuStorageTest : public MultiThreadedTest {
protected:
  T_ shared_data;

protected:
  void SetUp() override {
    MultiThreadedTest::SetUp();
    shared_data = {};
  }

  void TearDown() override {
    MultiThreadedTest::TearDown();
  }
};


using Storage = ::testing::Types<RcuStorage<Counts>>;
TYPED_TEST_SUITE(RcuStorageTest, Storage);


TYPED_TEST(RcuStorageTest, smoke) {
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