#include <gtest/gtest.h>
#include "argparse.hpp"

/* ****************************************************************************
 *
 *
 * NAME
 *        echo - display a line of text
 *
 * SYNOPSIS
 *        echo [SHORT-OPTION]... [STRING]...
 *        echo LONG-OPTION
 *
 * DESCRIPTION
 *        Echo the STRING(s) to standard output.
 *
 *        -n     do not output the trailing newline
 *
 *        -e     enable interpretation of backslash escapes
 *
 *        -E     disable interpretation of backslash escapes (default)
 *
 *        --help display this help and exit
 *
 *        --version
 *               output version information and exit
 * ***************************************************************************/

class EchoCmdBind : public testing::Test {
 public:
  struct EchoOptions {
    bool flag_n;
    bool flag_E;
    bool flag_help;
    bool flag_version;
    // TODO:
    // 怎样判断后续的都是非选项参数
    std::vector<std::string> others;
  };
  void SetUp() override {
    parser = argparse::ArgParser{};
    options = EchoOptions{};
    parser.add_flag("n", options.flag_n);
    parser.add_flag("E,!e", options.flag_E).set_default(true);
    parser.add_flag("help", options.flag_help);
    parser.add_flag("version", options.flag_version);
    parser.add_positional("others", options.others);
    parser.set_unknown_option_as_start_of_positionals();
  };

  void TearDown() override {}

  argparse::ArgParser parser;
  EchoOptions options;
};

class EchoCmdUnbind : public testing::Test {
 public:
  void SetUp() override {
    parser = argparse::ArgParser{};
    parser.add_flag("n");
    parser.add_flag("E,!e").set_default(true);
    parser.add_flag("help");
    parser.add_flag("version");
    parser.add_positional("others");
    parser.set_unknown_option_as_start_of_positionals();
  }
  void TearDown() override {}
  argparse::ArgParser parser;
};

TEST_F(EchoCmdUnbind, check_is_flag) {
  ASSERT_TRUE(parser["n"].is_flag());
  ASSERT_TRUE(parser["E"].is_flag());
  ASSERT_TRUE(parser["help"].is_flag());
  ASSERT_TRUE(parser["version"].is_flag());

  ASSERT_FALSE(parser["n"].as<bool>());
  ASSERT_TRUE(parser["E"].as<bool>());
  ASSERT_FALSE(parser["help"].as<bool>());
  ASSERT_FALSE(parser["version"].as<bool>());
}

TEST_F(EchoCmdBind, check_is_flag) {
  ASSERT_FALSE(options.flag_n);
  ASSERT_TRUE(options.flag_E);
  ASSERT_FALSE(options.flag_help);
  ASSERT_FALSE(options.flag_version);
}

TEST_F(EchoCmdBind, parser_unbind) {
  std::vector<const char*> cmd{"echo", "-n", "123456"};
  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_TRUE(parser["n"].as<bool>());
  ASSERT_TRUE(parser["E"].as<bool>());
  ASSERT_FALSE(parser["help"].as<bool>());
  ASSERT_FALSE(parser["version"].as<bool>());

  ASSERT_EQ(parser["others"].as<std::vector<std::string>>().size(), 1);
  ASSERT_EQ(parser["others"].as<std::vector<std::string>>()[0], "123456");
}

TEST_F(EchoCmdBind, parser_bind) {
  ASSERT_TRUE(options.flag_E);
  std::vector<const char*> cmd{"echo", "-n", "123456"};
  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));
  ASSERT_TRUE(options.flag_n);
  ASSERT_TRUE(options.flag_E);
  ASSERT_FALSE(options.flag_help);
  ASSERT_FALSE(options.flag_version);

  ASSERT_EQ(options.others.size(), 1);
  ASSERT_EQ(options.others[0], "123456");
}

TEST_F(EchoCmdUnbind, parser1_unbind) {
  std::vector<const char*> cmd{"echo", "-e", "123456"};
  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_FALSE(parser["n"].as<bool>());
  ASSERT_FALSE(parser["E"].as<bool>());
  ASSERT_FALSE(parser["help"].as<bool>());
  ASSERT_FALSE(parser["version"].as<bool>());

  ASSERT_EQ(parser["others"].as<std::vector<std::string>>().size(), 1);
  ASSERT_EQ(parser["others"].as<std::vector<std::string>>()[0], "123456");
}

TEST_F(EchoCmdBind, parser1_bind) {
  std::vector<const char*> cmd{"echo", "-e", "123456"};
  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_FALSE(options.flag_n);
  ASSERT_FALSE(options.flag_E);
  ASSERT_FALSE(options.flag_help);
  ASSERT_FALSE(options.flag_version);

  ASSERT_EQ(options.others.size(), 1);
  ASSERT_EQ(options.others[0], "123456");
}

// linux command line
//% echo --test -n -E -n --version --help 123456
// --test -n -E -n --version --help 123456

TEST_F(EchoCmdUnbind, parser2_unbind) {
  std::vector<const char*> cmd{"echo",   "--test",    "-n",
                               "--help", "--version", "123456"};
  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_FALSE(parser["n"].as<bool>());
  ASSERT_TRUE(parser["E"].as<bool>());
  ASSERT_FALSE(parser["help"].as<bool>());
  ASSERT_FALSE(parser["version"].as<bool>());

  ASSERT_EQ(parser["others"].as<std::vector<std::string>>().size(), 5);
  ASSERT_EQ(parser["others"].as<std::vector<std::string>>()[0], "--test");
  ASSERT_EQ(parser["others"].as<std::vector<std::string>>()[1], "-n");
  ASSERT_EQ(parser["others"].as<std::vector<std::string>>()[2], "--help");
  ASSERT_EQ(parser["others"].as<std::vector<std::string>>()[3], "--version");
  ASSERT_EQ(parser["others"].as<std::vector<std::string>>()[4], "123456");
}

TEST_F(EchoCmdBind, parser2_bind) {
  std::vector<const char*> cmd{"echo",   "--test",    "-n",
                               "--help", "--version", "123456"};
  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_FALSE(options.flag_n);
  ASSERT_TRUE(options.flag_E);
  ASSERT_FALSE(options.flag_help);
  ASSERT_FALSE(options.flag_version);

  ASSERT_EQ(options.others.size(), 5);
  ASSERT_EQ(options.others[0], "--test");
  ASSERT_EQ(options.others[1], "-n");
  ASSERT_EQ(options.others[2], "--help");
  ASSERT_EQ(options.others[3], "--version");
  ASSERT_EQ(options.others[4], "123456");
}
