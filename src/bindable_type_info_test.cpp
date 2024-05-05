#include <gtest/gtest.h>
#include "argparse.hpp"

TEST(BindableTypeInfo, name) {
  using argparse::bindable_type_info;
  ASSERT_EQ(bindable_type_info<bool>::name(), "bool");
  ASSERT_EQ(bindable_type_info<int>::name(), "int");
  ASSERT_EQ(bindable_type_info<double>::name(), "double");
  ASSERT_EQ(bindable_type_info<std::string>::name(), "std::string");

  // pair
  ASSERT_EQ((bindable_type_info<std::pair<bool, bool>>::name()),
            "std::pair<bool,bool>");
  ASSERT_EQ((bindable_type_info<std::pair<int, int>>::name()),
            "std::pair<int,int>");
  ASSERT_EQ((bindable_type_info<std::pair<double, double>>::name()),
            "std::pair<double,double>");
  ASSERT_EQ((bindable_type_info<std::pair<std::string, std::string>>::name()),
            "std::pair<std::string,std::string>");
  ASSERT_EQ((bindable_type_info<std::pair<std::string, bool>>::name()),
            "std::pair<std::string,bool>");
  ASSERT_EQ((bindable_type_info<std::pair<std::string, int>>::name()),
            "std::pair<std::string,int>");

  // vector
  ASSERT_EQ((bindable_type_info<std::vector<bool>>::name()),
            "std::vector<bool>");
  ASSERT_EQ((bindable_type_info<std::vector<int>>::name()), "std::vector<int>");
  ASSERT_EQ((bindable_type_info<std::vector<double>>::name()),
            "std::vector<double>");
  ASSERT_EQ((bindable_type_info<std::vector<std::string>>::name()),
            "std::vector<std::string>");
  ASSERT_EQ((bindable_type_info<
                std::vector<std::pair<std::string, std::string>>>::name()),
            "std::vector<std::pair<std::string,std::string>>");
  ASSERT_EQ((bindable_type_info<std::vector<std::vector<int>>>::name()),
            "std::vector<std::vector<int>>");

  // map
  ASSERT_EQ((bindable_type_info<std::map<std::string, std::string>>::name()),
            "std::map<std::string,std::string>");

  ASSERT_EQ((bindable_type_info<std::map<std::string, int>>::name()),
            "std::map<std::string,int>");
}
