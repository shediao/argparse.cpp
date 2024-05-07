
#include <gtest/gtest.h>
#include <argparse.hpp>

/* ****************************************************************************
 * NAME
 *        tac - concatenate and print files in reverse
 *
 * SYNOPSIS
 *        tac [OPTION]... [FILE]...
 *
 * DESCRIPTION
 *        Write each FILE to standard output, last line first.
 *
 *        With no FILE, or when FILE is -, read standard input.
 *
 *        Mandatory arguments to long options are mandatory for short options
 * too.
 *
 *        -b, --before
 *               attach the separator before instead of after
 *
 *        -r, --regex
 *               interpret the separator as a regular expression
 *
 *        -s, --separator=STRING
 *               use STRING as the separator instead of newline
 *
 *        --help display this help and exit
 *
 *        --version
 *               output version information and exit
 * ****************************************************************************/

struct TacFlags {
  bool before;
  bool regex;
  std::string separator;
  bool help;
  bool version;
  std::vector<std::string> files;
};
static TacFlags tac_flags;
void reset_tac_flags() {
  tac_flags = TacFlags{};
}

argparse::ArgParser bind_value() {
  argparse::ArgParser parser{
      R"(Write each FILE to standard output, last line first.

With no FILE, or when FILE is -, read standard input.)"};
  parser.set_program_name("tac");
  parser.add_flag("-b,--before", tac_flags.before)
      .help("attach the separator before instead of after");
  parser.add_flag("-r,--regex", tac_flags.regex)
      .help("interpret the separator as a regular expression");
  parser.add_flag("--help", tac_flags.help).help("display this help and exit");
  parser.add_flag("--version", tac_flags.version)
      .help("output version information and exit");
  parser.add_option("-s,--separator", tac_flags.separator)
      .value_help("STRING")
      .help("use STRING as the separator instead of newline");
  parser.add_positional("files", tac_flags.files)
      .value_help("FILE")
      .help("Write each FILE to standard output, last line first.");
  return parser;
}

TEST(Cat, help) {
  reset_tac_flags();
  auto parser = bind_value();

  std::vector<const char*> cmds = {"tac", "--help"};
  ASSERT_NO_THROW(parser.parse(cmds.size(), cmds.data()));

  EXPECT_FALSE(tac_flags.before);
  EXPECT_FALSE(tac_flags.regex);
  EXPECT_TRUE(tac_flags.help);
  EXPECT_FALSE(tac_flags.version);
  EXPECT_TRUE(tac_flags.separator.empty());
  EXPECT_TRUE(tac_flags.files.empty());
}

TEST(Cat, version) {
  reset_tac_flags();
  auto parser = bind_value();

  std::vector<const char*> cmds = {"tac", "--version"};
  ASSERT_NO_THROW(parser.parse(cmds.size(), cmds.data()));

  EXPECT_FALSE(tac_flags.before);
  EXPECT_FALSE(tac_flags.regex);
  EXPECT_FALSE(tac_flags.help);
  EXPECT_TRUE(tac_flags.version);
  EXPECT_TRUE(tac_flags.separator.empty());
  EXPECT_TRUE(tac_flags.files.empty());
}

TEST(Cat, files) {
  reset_tac_flags();
  auto parser = bind_value();

  std::vector<const char*> cmds = {"tac", "-"};
  ASSERT_NO_THROW(parser.parse(cmds.size(), cmds.data()));

  EXPECT_FALSE(tac_flags.before);
  EXPECT_FALSE(tac_flags.regex);
  EXPECT_FALSE(tac_flags.help);
  EXPECT_FALSE(tac_flags.version);
  EXPECT_TRUE(tac_flags.separator.empty());
  EXPECT_FALSE(tac_flags.files.empty());

  EXPECT_EQ(1, tac_flags.files.size());
  EXPECT_EQ("-", tac_flags.files[0]);
}

TEST(Cat, no_args) {
  reset_tac_flags();
  auto parser = bind_value();

  std::vector<const char*> cmds = {"tac"};
  ASSERT_NO_THROW(parser.parse(cmds.size(), cmds.data()));

  EXPECT_FALSE(tac_flags.before);
  EXPECT_FALSE(tac_flags.regex);
  EXPECT_FALSE(tac_flags.help);
  EXPECT_FALSE(tac_flags.version);
  EXPECT_TRUE(tac_flags.separator.empty());
  EXPECT_TRUE(tac_flags.files.empty());
}

TEST(Cat, separator) {
  reset_tac_flags();
  auto parser = bind_value();

  std::vector<const char*> cmds = {"tac", "--separator=,"};
  ASSERT_NO_THROW(parser.parse(cmds.size(), cmds.data()));

  EXPECT_FALSE(tac_flags.before);
  EXPECT_FALSE(tac_flags.regex);
  EXPECT_FALSE(tac_flags.help);
  EXPECT_FALSE(tac_flags.version);
  EXPECT_TRUE(tac_flags.files.empty());

  EXPECT_EQ(tac_flags.separator, ",");
}

TEST(Cat, separator2) {
  reset_tac_flags();
  auto parser = bind_value();

  std::vector<const char*> cmds = {"tac", R"(--separator=\n)"};
  ASSERT_NO_THROW(parser.parse(cmds.size(), cmds.data()));

  EXPECT_EQ(tac_flags.separator, "\\n");
}

TEST(Cat, separator3) {
  reset_tac_flags();
  auto parser = bind_value();

  std::vector<const char*> cmds = {"tac", "-s", "+"};
  ASSERT_NO_THROW(parser.parse(cmds.size(), cmds.data()));

  EXPECT_EQ(tac_flags.separator, "+");

  std::cout << parser.usage() << std::endl;
}
