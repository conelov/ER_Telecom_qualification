#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <net_utils/PriorityMutex.hpp>
#include <net_utils/RcuStorage.hpp>

#include <net_utils/aux/RcuStorageFixture.hpp>


namespace {


using namespace nut;


template<typename Fix>
class RcuStorageTest
    : public ::testing::Test
    , public Fix {
protected:
  void SetUp() override {
    this->read_iters  = 1'000'000;
    this->write_iters = 100'000;
    this->set_rw_relation(1. / 2);
    this->readers *= 2;
    this->writers = this->writers * 2;
    if constexpr (std::is_base_of_v<aux::RcuLruStorageFixtureBaseTag, std::remove_pointer_t<decltype(this)>>) {
      this->cache_size = this->write_iters;
    }
    this->bind();
  }
};


using Storage = ::testing::Types<
  aux::RcuLruStorageFixture<std::shared_mutex>,
  aux::RcuLruStorageFixture<PriorityMutex<>>,
  aux::RcuStorageFixture<std::shared_mutex>,
  aux::RcuStorageFixture<PriorityMutex<>>>;
TYPED_TEST_SUITE(RcuStorageTest, Storage);


TYPED_TEST(RcuStorageTest, high_load) {
  ASSERT_NO_THROW(this->start());

  EXPECT_EQ(this->iter_counter(), this->writers * this->write_iters + this->readers * this->read_iters);

  if constexpr (std::is_base_of_v<aux::RcuStorageFixtureBaseTag, std::remove_pointer_t<decltype(this)>>) {
    auto const array_ptr = this->value->load();
    EXPECT_THAT(*array_ptr, testing::ElementsAreArray(std::vector(this->writers, this->write_iters)));
  }
}


}// namespace