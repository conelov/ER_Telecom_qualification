#include <gtest/gtest.h>

#include <net_utils/IterativeAverage.hpp>


TEST(IterativeAverage, smoke) {
  nut::IterativeAverage average;
  ASSERT_EQ(average, 0);
  ASSERT_EQ(average.count(), 0);
}
