#include <random>

#include <gtest/gtest.h>

#include <net_utils/IterativeAverage.hpp>


namespace {


using namespace nut;


TEST(IterativeAverage, smoke) {
  IterativeAverage average;
  average += 10;
  ASSERT_EQ(average, 10);
  ASSERT_EQ(average.count(), 1);
  average += 0;
  ASSERT_EQ(average, 5);
  ASSERT_EQ(average.count(), 2);

  average.reset();
  std::random_device                     rd{};
  std::minstd_rand                       re{rd()};
  std::vector<decltype(re)::result_type> generic_values;
  for (std::size_t i = 0; i < 1'000; ++i) {
    auto const val = re();
    generic_values.push_back(val);
    average += val;
  }
  ASSERT_DOUBLE_EQ(average, std::accumulate(generic_values.cbegin(), generic_values.cend(), 0.) / generic_values.size());
}


}// namespace
