#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <net_utils/PriorityMutex.hpp>

#include <net_utils/aux/PriorityMutexFixture.hpp>


using namespace nut;


namespace {


template<typename Mx_>
class PriorityMutexTest
    : public ::testing::Test
    , public aux::PriorityMutexFixture<Mx_> {
protected:
  void SetUp() override {
    this->read_iters = 1'000'000;
    this->write_iters    = 100'000;
    this->set_rw_relation(1. / 2);
    this->bind();
  }
};


using Storage = ::testing::Types<std::shared_mutex, PriorityMutex>;
TYPED_TEST_SUITE(PriorityMutexTest, Storage);


TYPED_TEST(PriorityMutexTest, high_load) {
  ASSERT_NO_THROW(this->iteration());

  // EXPECT_EQ(this->iter_counter(), this->writers * this->write_iters + this->readers * this->read_iters);
  EXPECT_THAT(*this->value, testing::ElementsAreArray(std::vector(this->writers, this->write_iters)));
}


}// namespace