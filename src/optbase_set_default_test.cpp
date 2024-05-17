#include <gtest/gtest.h>
#include "argparse.hpp"

TEST(OptBase, Flag_setDefaul) {
  argparse::ArgParser parser;
  auto& bool_flag = parser.add_flag("i,!I");
  ASSERT_FALSE(bool_flag.get<bool>());
  bool_flag.set_default(true);
  ASSERT_TRUE(bool_flag.get<bool>());

  ASSERT_THROW(bool_flag.set_default(""), argparse::bad_value_access);

  auto& int_flag = parser.add_flag<int>("-v,verbose");
  ASSERT_EQ(0, int_flag.get<int>());

  int_flag.set_default(1);
  ASSERT_EQ(1, int_flag.get<int>());
  ASSERT_THROW(int_flag.set_default(""), argparse::bad_value_access);

  std::vector<const char*> const cmd{"test", "-iiiiIvvv"};

  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));
  ASSERT_FALSE(bool_flag.get<bool>());
  ASSERT_EQ(3, int_flag.get<int>());

  ASSERT_FALSE(parser["i"].get<bool>());
  ASSERT_EQ(parser["v"].get<int>(), 3);
}

TEST(OptBase, Option_SetDefault) {
  argparse::ArgParser parser;

  auto& str_opt = parser.add_option("s");
  auto& int_opt = parser.add_option<int>("i");
  auto& double_opt = parser.add_option<double>("d");
  auto& pair_str_str_opt =
      parser.add_option<std::pair<std::string, std::string>>("kv");
  auto& pair_str_int_opt =
      parser.add_option<std::pair<std::string, int>>("hp", ':');

  auto& map_str_int_opt =
      parser.add_option<std::map<std::string, int>>("e", '=');

  ASSERT_EQ(str_opt.get<std::string>(), "");
  ASSERT_EQ(int_opt.get<int>(), 0);
  ASSERT_DOUBLE_EQ(double_opt.get<double>(), 0.0);
  ASSERT_EQ((pair_str_str_opt.get<std::pair<std::string, std::string>>()),
            (std::pair<std::string, std::string>{}));
  ASSERT_EQ((pair_str_int_opt.get<std::pair<std::string, int>>()),
            (std::pair<std::string, int>{}));

  ASSERT_TRUE((map_str_int_opt.get<std::map<std::string, int>>().empty()));

  str_opt.set_default("11");
  int_opt.set_default(13);
  double_opt.set_default(3.14);
  pair_str_str_opt.set_default(std::pair<std::string, std::string>{"k1", "v1"});
  pair_str_int_opt.set_default(
      std::pair<std::string, int>{"192.168.0.1", 8888});

  map_str_int_opt.set_default(
      std::map<std::string, int>{{"0000", 0}, {"1111", 1}});

  ASSERT_EQ((map_str_int_opt.get<std::map<std::string, int>>().at("0000")), 0);
  ASSERT_EQ((map_str_int_opt.get<std::map<std::string, int>>().at("1111")), 1);

  ASSERT_EQ(str_opt.get<std::string>(), "11");
  ASSERT_EQ(int_opt.get<int>(), 13);
  ASSERT_DOUBLE_EQ(double_opt.get<double>(), 3.14);
  ASSERT_EQ((pair_str_str_opt.get<std::pair<std::string, std::string>>()),
            (std::pair<std::string, std::string>{"k1", "v1"}));
  ASSERT_EQ((pair_str_int_opt.get<std::pair<std::string, int>>()),
            (std::pair<std::string, int>{"192.168.0.1", 8888}));

  std::vector<const char*> cmd{
      "test",      "-s",   "test",  "-i9",
      "-d2.71828", "--kv", "k2=v2", "--hp=192.168.1.1:9999",
      "-e",        "e0=0", "-e",    "e1=1",
      "-e",        "e2=2", "-e",    "e3=3",
      "-e",        "e4=4"};

  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_EQ(str_opt.get<std::string>(), "test");
  ASSERT_EQ(int_opt.get<int>(), 9);
  ASSERT_DOUBLE_EQ(double_opt.get<double>(), 2.71828);
  ASSERT_EQ((pair_str_str_opt.get<std::pair<std::string, std::string>>()),
            (std::pair<std::string, std::string>{"k2", "v2"}));
  ASSERT_EQ((pair_str_int_opt.get<std::pair<std::string, int>>()),
            (std::pair<std::string, int>{"192.168.1.1", 9999}));

  ASSERT_EQ((map_str_int_opt.get<std::map<std::string, int>>().at("e0")), 0);
  ASSERT_EQ((map_str_int_opt.get<std::map<std::string, int>>().at("e1")), 1);
  ASSERT_EQ((map_str_int_opt.get<std::map<std::string, int>>().at("e2")), 2);
  ASSERT_EQ((map_str_int_opt.get<std::map<std::string, int>>().at("e3")), 3);
  ASSERT_EQ((map_str_int_opt.get<std::map<std::string, int>>().at("e4")), 4);
  ASSERT_EQ((map_str_int_opt.get<std::map<std::string, int>>().size()), 5);
}

TEST(OptBase, Positional_SetDefault) {
  argparse::ArgParser parser;

  auto& str_opt = parser.add_option("s");
  auto& int_opt = parser.add_option<int>("i");
  auto& double_opt = parser.add_option<double>("d");
  auto& pair_str_str_opt =
      parser.add_option<std::pair<std::string, std::string>>("kv");
  auto& str_int_positional =
      parser.add_positional<std::vector<std::pair<std::string, int>>>(
          "host_port", ':');

  auto const& positionals =
      str_int_positional.get<std::vector<std::pair<std::string, int>>>();

  ASSERT_TRUE(positionals.empty());

  auto default_value = std::vector<std::pair<std::string, int>>{
      {"0000", 0}, {"1111", 1}, {"2222", 2}, {"3333", 3}};
  str_int_positional.set_default(default_value);
  ASSERT_EQ(default_value, positionals);

  std::vector<const char*> cmd{"test",
                               "-s",
                               "test",
                               "-i9",
                               "-d2.71828",
                               "--kv",
                               "k2=v2",
                               "192.168.0.0:1110",
                               "192.168.0.1:1111",
                               "192.168.0.2:1112",
                               "192.168.0.3:1113",
                               "192.168.0.4:1114"};

  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));
  ASSERT_EQ(str_opt.get<std::string>(), "test");
  ASSERT_EQ(int_opt.get<int>(), 9);
  ASSERT_DOUBLE_EQ(double_opt.get<double>(), 2.71828);
  ASSERT_EQ((pair_str_str_opt.get<std::pair<std::string, std::string>>()),
            (std::pair<std::string, std::string>{"k2", "v2"}));

  ASSERT_EQ((str_int_positional.get<std::vector<std::pair<std::string, int>>>()
                 .size()),
            5);

  ASSERT_EQ(positionals[0], (std::pair<std::string, int>{"192.168.0.0", 1110}));
  ASSERT_EQ(positionals[1], (std::pair<std::string, int>{"192.168.0.1", 1111}));
  ASSERT_EQ(positionals[2], (std::pair<std::string, int>{"192.168.0.2", 1112}));
  ASSERT_EQ(positionals[3], (std::pair<std::string, int>{"192.168.0.3", 1113}));
  ASSERT_EQ(positionals[4], (std::pair<std::string, int>{"192.168.0.4", 1114}));
}
