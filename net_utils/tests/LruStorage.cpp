#include <gtest/gtest.h>

#include <net_utils/LruStorage.hpp>


namespace {


using namespace nut;


TEST(LruStorage, smoke) {
  LruStorage<std::string, int, std::string_view> s{2};
  ASSERT_TRUE(s.empty());
  ASSERT_EQ(s.size(), 0);
  ASSERT_EQ(s.get("1"), nullptr);
  ASSERT_EQ(s.put("1", 1), 1);
  ASSERT_EQ(*s.get("1"), 1);

  ASSERT_EQ(s.put("2", 2), 2);
  ASSERT_EQ(*s.get("1"), 1);
  ASSERT_EQ(s.put("3", 3), 3);
  ASSERT_EQ(s.get("2"), nullptr);
}


}// namespace