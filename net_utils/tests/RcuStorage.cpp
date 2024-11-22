#include <array>

#include <gtest/gtest.h>

#include <net_utils/RcuStorage.hpp>

#include <net_utils/aux/MultiThreadedFixture.hpp>


using namespace nut;


namespace {


std::size_t constexpr num_readers = 10;
std::size_t constexpr num_writers = 3;
using Counts                      = std::array<std::uintmax_t, num_writers>;


template<typename T_>
class RcuStorageTest
    : public ::testing::Test
    , public MultiThreadedFixture {
protected:
  T_ shared_data;

protected:
  void TearDown() override {
    shared_data = {};
  }
};


using Storage = ::testing::Types<RcuStorage<Counts>>;
TYPED_TEST_SUITE(RcuStorageTest, Storage);


TYPED_TEST(RcuStorageTest, smoke) {
  Counts counts{};

  for (std::size_t i = 0; i < num_readers; ++i) {
    this->emplace_worker([this] {
      if (!*this->shared_data) {
        return;
      }
      [[maybe_unused]] auto v = **this->shared_data;
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    });
  }


  for (std::size_t i = 0; i < num_writers; ++i) {
    this->emplace_worker([this, &c = counts[i], i] {
      this->shared_data.modify([&c, i](auto ptr) {
        if (!ptr) {
          ptr = std::make_shared<typename decltype(this->shared_data)::value_type>();
        }
        ptr->at(i) = ++c;
        return std::move(ptr);
      });
    });
  }

  this->down();
  ASSERT_EQ(**this->shared_data, counts);
}


}// namespace