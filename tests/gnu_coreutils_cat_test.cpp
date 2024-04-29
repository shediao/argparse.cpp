
#include <argparse.hpp>
#include <gtest/gtest.h>

/* ****************************************************************************
 * NAME
 *        cat - concatenate files and print on the standard output
 *
 * SYNOPSIS
 *        cat [OPTION]... [FILE]...
 *
 * DESCRIPTION
 *        Concatenate FILE(s) to standard output.
 *
 *        With no FILE, or when FILE is -, read standard input.
 *
 *        -A, --show-all
 *               equivalent to -vET
 *
 *        -b, --number-nonblank
 *               number nonempty output lines, overrides -n
 *
 *        -e     equivalent to -vE
 *
 *        -E, --show-ends
 *               display $ at end of each line
 *
 *        -n, --number
 *               number all output lines
 *
 *        -s, --squeeze-blank
 *               suppress repeated empty output lines
 *
 *        -t     equivalent to -vT
 *
 *        -T, --show-tabs
 *               display TAB characters as ^I
 *
 *        -u     (ignored)
 *
 *        -v, --show-nonprinting
 *               use ^ and M- notation, except for LFD and TAB
 *
 *        --help display this help and exit
 *
 *        --version
 *               output version information and exit
 *
 * EXAMPLES
 *        cat f - g
 *               Output f's contents, then standard input, then g's contents.
 *
 *        cat    Copy standard input to standard output.
 * ****************************************************************************/

struct CatFlags {
  bool show_all{};
  bool number_nonblank{};
  bool e{};
  bool show_ends{};
  bool number{};
  bool squeeze_blank{};
  bool t{};
  bool show_tabs{};
  bool u{};
  bool show_nonprinting{};
  bool help{};
  bool version{};
  std::vector<std::string> files{};
};
static CatFlags cat_flags;
void reset_cat_flags() {
  cat_flags = CatFlags{};
}

argparse::ArgParser bind_value() {
  argparse::ArgParser parser{
      "concatenate files and print on the standard output"};
  parser.set_program_name("cat");
  parser.add_flag("-A,--show-all", cat_flags.show_all)
      .help("equivalent to -vET");
  parser.add_flag("-b,--number-nonblank", cat_flags.number_nonblank)
      .help("number nonempty output lines, overrides -n");
  parser.add_flag("-e", cat_flags.e).help("equivalent to -vE");
  parser.add_flag("-E,--show-ends", cat_flags.show_ends)
      .help("display $ at end of each line");
  parser.add_flag("-n,--number", cat_flags.number)
      .help("number all output lines");
  parser.add_flag("-s,--squeeze-blank", cat_flags.squeeze_blank)
      .help("suppress repeated empty output lines");
  parser.add_flag("-t", cat_flags.t).help("equivalent to -vT");
  parser.add_flag("-T,--show-tabs", cat_flags.show_tabs)
      .help("display TAB characters as ^I");
  parser.add_flag("-u", cat_flags.u).help("(ignored)");
  parser.add_flag("-v,--show-nonprinting", cat_flags.show_nonprinting)
      .help("use ^ and M- notation, except for LFD and TAB");
  parser.add_flag("--help", cat_flags.help).help("display this help and exit");
  parser.add_flag("--version", cat_flags.version)
      .help("output version information and exit");
  parser.add_position_arg(cat_flags.files);

  return parser;
}

TEST(Cat, help) {
  reset_cat_flags();
  auto parser = bind_value();

  std::vector<const char*> cmds = {"cat", "--help"};
  auto [ret, msg] = parser.parse(cmds.size(), cmds.data());

  EXPECT_EQ(0, ret);
  EXPECT_TRUE(msg.empty());

  EXPECT_FALSE(cat_flags.show_all);
  EXPECT_FALSE(cat_flags.number_nonblank);
  EXPECT_FALSE(cat_flags.e);
  EXPECT_FALSE(cat_flags.show_ends);
  EXPECT_FALSE(cat_flags.number);
  EXPECT_FALSE(cat_flags.squeeze_blank);
  EXPECT_FALSE(cat_flags.t);
  EXPECT_FALSE(cat_flags.show_tabs);
  EXPECT_FALSE(cat_flags.u);
  EXPECT_FALSE(cat_flags.show_nonprinting);
  EXPECT_TRUE(cat_flags.help);
  EXPECT_FALSE(cat_flags.version);
  EXPECT_TRUE(cat_flags.files.empty());


  using namespace std::literals::string_literals;
  EXPECT_EQ(cat_flags.show_all, parser.flag("show-all").as<bool>());
  EXPECT_EQ(cat_flags.number_nonblank, parser.flag("number-nonblank").as<bool>());
  EXPECT_EQ(cat_flags.e, parser.flag('e').as<bool>());
  EXPECT_EQ(cat_flags.show_ends, parser.flag("show-ends").as<bool>());
  EXPECT_EQ(cat_flags.number, parser.flag("number").as<bool>());
  EXPECT_EQ(cat_flags.squeeze_blank, parser.flag("squeeze-blank").as<bool>());
  EXPECT_EQ(cat_flags.t, parser.flag('t').as<bool>());
  EXPECT_EQ(cat_flags.show_tabs, parser.flag("show-tabs").as<bool>());
  EXPECT_EQ(cat_flags.u, parser.flag('u').as<bool>());
  EXPECT_EQ(cat_flags.show_nonprinting, parser.flag("show-nonprinting").as<bool>());
  EXPECT_EQ(cat_flags.help, parser.flag("help").as<bool>());
  EXPECT_EQ(cat_flags.version, parser.flag("version").as<bool>());
}

TEST(Cat, version) {
  reset_cat_flags();
  auto parser = bind_value();

  std::vector<const char*> cmds = {"cat", "--version"};
  auto [ret, msg] = parser.parse(cmds.size(), cmds.data());

  EXPECT_EQ(0, ret);
  EXPECT_TRUE(msg.empty());

  EXPECT_FALSE(cat_flags.show_all);
  EXPECT_FALSE(cat_flags.number_nonblank);
  EXPECT_FALSE(cat_flags.e);
  EXPECT_FALSE(cat_flags.show_ends);
  EXPECT_FALSE(cat_flags.number);
  EXPECT_FALSE(cat_flags.squeeze_blank);
  EXPECT_FALSE(cat_flags.t);
  EXPECT_FALSE(cat_flags.show_tabs);
  EXPECT_FALSE(cat_flags.u);
  EXPECT_FALSE(cat_flags.show_nonprinting);
  EXPECT_FALSE(cat_flags.help);
  EXPECT_TRUE(cat_flags.version);
  EXPECT_TRUE(cat_flags.files.empty());

  using namespace std::literals::string_literals;
  EXPECT_EQ(cat_flags.show_all, parser.flag("show-all").as<bool>());
  EXPECT_EQ(cat_flags.number_nonblank, parser.flag("number-nonblank").as<bool>());
  EXPECT_EQ(cat_flags.e, parser.flag("e").as<bool>());
  EXPECT_EQ(cat_flags.show_ends, parser.flag("show-ends").as<bool>());
  EXPECT_EQ(cat_flags.number, parser.flag("number").as<bool>());
  EXPECT_EQ(cat_flags.squeeze_blank, parser.flag("squeeze-blank").as<bool>());
  EXPECT_EQ(cat_flags.t, parser.flag('t').as<bool>());
  EXPECT_EQ(cat_flags.show_tabs, parser.flag("show-tabs").as<bool>());
  EXPECT_EQ(cat_flags.u, parser.flag('u').as<bool>());
  EXPECT_EQ(cat_flags.show_nonprinting, parser.flag("show-nonprinting").as<bool>());
  EXPECT_EQ(cat_flags.help, parser.flag("help").as<bool>());
  EXPECT_EQ(cat_flags.version, parser.flag("version").as<bool>());
}

TEST(Cat, files) {
  reset_cat_flags();
  auto parser = bind_value();

  std::vector<const char*> cmds = {"cat", "--", "-", "/path/to/file"};
  auto [ret, msg] = parser.parse(cmds.size(), cmds.data());

  EXPECT_EQ(0, ret);
  EXPECT_TRUE(msg.empty());

  EXPECT_FALSE(cat_flags.show_all);
  EXPECT_FALSE(cat_flags.number_nonblank);
  EXPECT_FALSE(cat_flags.e);
  EXPECT_FALSE(cat_flags.show_ends);
  EXPECT_FALSE(cat_flags.number);
  EXPECT_FALSE(cat_flags.squeeze_blank);
  EXPECT_FALSE(cat_flags.t);
  EXPECT_FALSE(cat_flags.show_tabs);
  EXPECT_FALSE(cat_flags.u);
  EXPECT_FALSE(cat_flags.show_nonprinting);
  EXPECT_FALSE(cat_flags.help);
  EXPECT_FALSE(cat_flags.version);
  EXPECT_FALSE(cat_flags.files.empty());

  EXPECT_EQ(cat_flags.files.size(), 2);

  EXPECT_EQ(cat_flags.files[0], "-");
  EXPECT_EQ(cat_flags.files[1], "/path/to/file");
}

TEST(Cat, no_args) {
  reset_cat_flags();
  auto parser = bind_value();

  std::vector<const char*> cmds = {"cat"};
  auto [ret, msg] = parser.parse(cmds.size(), cmds.data());

  EXPECT_EQ(0, ret);
  EXPECT_TRUE(msg.empty());

  EXPECT_FALSE(cat_flags.show_all);
  EXPECT_FALSE(cat_flags.number_nonblank);
  EXPECT_FALSE(cat_flags.e);
  EXPECT_FALSE(cat_flags.show_ends);
  EXPECT_FALSE(cat_flags.number);
  EXPECT_FALSE(cat_flags.squeeze_blank);
  EXPECT_FALSE(cat_flags.t);
  EXPECT_FALSE(cat_flags.show_tabs);
  EXPECT_FALSE(cat_flags.u);
  EXPECT_FALSE(cat_flags.show_nonprinting);
  EXPECT_FALSE(cat_flags.help);
  EXPECT_FALSE(cat_flags.version);
  EXPECT_TRUE(cat_flags.files.empty());
}

TEST(Cat, other_flag) {
  reset_cat_flags();
  auto parser = bind_value();

  std::vector<const char*> cmds = {"cat", "-n", "--", "-"};
  auto [ret, msg] = parser.parse(cmds.size(), cmds.data());

  EXPECT_EQ(0, ret);
  EXPECT_TRUE(msg.empty());

  EXPECT_FALSE(cat_flags.show_all);
  EXPECT_FALSE(cat_flags.number_nonblank);
  EXPECT_FALSE(cat_flags.e);
  EXPECT_FALSE(cat_flags.show_ends);
  EXPECT_TRUE(cat_flags.number);
  EXPECT_FALSE(cat_flags.squeeze_blank);
  EXPECT_FALSE(cat_flags.t);
  EXPECT_FALSE(cat_flags.show_tabs);
  EXPECT_FALSE(cat_flags.u);
  EXPECT_FALSE(cat_flags.show_nonprinting);
  EXPECT_FALSE(cat_flags.help);
  EXPECT_FALSE(cat_flags.version);
  EXPECT_FALSE(cat_flags.files.empty());

  EXPECT_EQ(cat_flags.files.size(), 1);

  EXPECT_EQ(cat_flags.files[0], "-");
}
