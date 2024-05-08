#include "argparse.hpp"

#include <gtest/gtest.h>
#include <exception>

TEST(PyArgParse, example0) {
  std::vector<const char*> cmd{"argparse"};
  argparse::ArgParser parser;

  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));
}

TEST(PyArgParse, example_default_help) {
  std::vector<const char*> cmd{"argparse", "-h"};
  argparse::ArgParser parser;

  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));
}

TEST(PyArgParse, example_1) {
  std::vector<const char*> cmd{"argparse", "echo"};
  argparse::ArgParser parser;

  ASSERT_THROW(parser.parse(cmd.size(), cmd.data()), std::exception);
}

TEST(PyArgParse, example_2) {
  std::vector<const char*> cmd{"argparse", "echo--xxx"};
  argparse::ArgParser parser;
  parser.add_positional("echo").help("echo the string you use here");

  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));
}

TEST(PyArgParse, example_3) {
  std::vector<const char*> cmd{"argparse", "4"};
  argparse::ArgParser parser;
  parser.add_positional<int>("square").help(
      "display a square of a given number");

  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_EQ(parser["square"].as<int>(), 4);
}

TEST(PyArgParse, example_4) {
  std::vector<const char*> cmd{"argparse", "--verbosity", "3"};
  argparse::ArgParser parser;
  parser.add_option<int>("--verbosity").help("increase output verbosity");

  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_EQ(parser["verbosity"].as<int>(), 3);

  parser = argparse::ArgParser{};

  cmd = {"argparse", "--verbosity", "N"};
  ASSERT_THROW(parser.parse(cmd.size(), cmd.data()), std::exception);

  parser = argparse::ArgParser{};
  cmd = {"argparse", "--verbosity"};
  ASSERT_THROW(parser.parse(cmd.size(), cmd.data()), std::exception);
}

TEST(PyArgParse, example_5) {
  argparse::ArgParser parser;
  parser.add_flag("-v,--verbose").help("increase output verbosity");

  std::vector<const char*> cmd{"argparse", "--verbose", "-v"};

  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));
  ASSERT_TRUE(parser["verbose"].as<bool>());

  cmd = {"argparse", "--verbose=1"};
  parser = argparse::ArgParser{};
  ASSERT_THROW(parser.parse(cmd.size(), cmd.data()), std::exception);

  cmd = {"argparse", "--verbose", "1"};
  parser = argparse::ArgParser{};
  ASSERT_THROW(parser.parse(cmd.size(), cmd.data()), std::exception);
}

TEST(PyArgParse, example_6) {
  argparse::ArgParser parser;
  parser.add_positional<int>("square").help(
      "display a square of a given number");
  parser.add_option<int>("-v,--verbosity").help("increase output verbosity");
  // .check([](const int& v) { return v > 0 && v < 4; });

  std::vector<const char*> cmd{"argparse", "4", "-v", "3"};
  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));
  ASSERT_EQ(parser["square"].as<int>(), 4);
  ASSERT_EQ(parser["verbosity"].as<int>(), 3);
}
