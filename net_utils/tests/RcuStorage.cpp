#include <mutex>

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <net_utils/PriorityMutex.hpp>
#include <net_utils/RcuStorage.hpp>

#include <net_utils/aux/RcuStorageFixture.hpp>


namespace {


using namespace nut;


template<typename Mx_>
class RcuStorageTest
    : public ::testing::Test
    , public aux::RcuStorageFixture<Mx_> {
protected:
  void SetUp() override {
    this->read_iters  = 1'000'000;
    this->write_iters = 100'000;
    this->set_rw_relation(1. / 2);
    this->readers *= 2;
    this->writers = this->writers * 2;
    this->bind();
  }
};


using Storage = ::testing::Types<std::shared_mutex, PriorityMutex<>>;
TYPED_TEST_SUITE(RcuStorageTest, Storage);


TYPED_TEST(RcuStorageTest, high_load) {
  ASSERT_NO_THROW(this->start());

  EXPECT_EQ(this->iter_counter(), this->writers * this->write_iters + this->readers * this->read_iters);
  auto const array_ptr = this->value->load();
  EXPECT_THAT(*array_ptr, testing::ElementsAreArray(std::vector(this->writers, this->write_iters)));
}


}// namespace