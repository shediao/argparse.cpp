#include <gtest/gtest.h>
#include "argparse.hpp"

/* ****************************************************************************
 * NAME
 *      uniq – report or filter out repeated lines in a file
 *
 * SYNOPSIS
 *      uniq [-c | -d | -D | -u] [-i] [-f num] [-s chars] [input_file
 * [output_file]]
 *
 * DESCRIPTION
 *      The uniq utility reads the specified input_file comparing adjacent
 * lines, and writes a copy of each unique input line to the output_file.  If
 * input_file is a single dash (‘-’) or absent, the standard input is read.  If
 * output_file is absent, standard output is used for output.  The second and
 * succeeding copies of identical adjacent input lines are not written. Repeated
 * lines in the input will not be detected if they are not adjacent, so it may
 * be necessary to sort the files first.
 *
 *      The following options are available:
 *
 *      -c, --count
 *              Precede each output line with the count of the number of times
 * the line occurred in the input, followed by a single space.
 *
 *      -d, --repeated
 *              Output a single copy of each line that is repeated in the input.
 *
 *      -D, --all-repeated [septype]
 *              Output all lines that are repeated (like -d, but each copy of
 * the repeated line is written).  The optional septype argument controls how to
 * separate groups of repeated lines in the output; it must be one of the
 * following values:
 *
 *              none      Do not separate groups of lines (this is the default).
 *              prepend   Output an empty line before each group of lines.
 *              separate  Output an empty line after each group of lines.
 *
 *      -f num, --skip-fields num
 *              Ignore the first num fields in each input line when doing
 * comparisons.  A field is a string of non-blank characters separated from
 * adjacent fields by blanks.  Field numbers are one based, i.e., the first
 * field is field one.
 *
 *      -i, --ignore-case
 *              Case insensitive comparison of lines.
 *
 *      -s chars, --skip-chars chars
 *              Ignore the first chars characters in each input line when doing
 * comparisons.  If specified in conjunction with the -f, --unique option, the
 * first chars characters after the first num fields will be ignored.  Character
 * numbers are one based, i.e., the first character is character one.
 *
 *      -u, --unique
 *              Only output lines that are not repeated in the input.
 *
 *
 *
 *
 * ****************************************************************************/

class UniqCommand : public testing::Test {
 public:
 protected:
  UniqCommand() : parser("report or filter out repeated lines in a file") {
    parser.set_program_name("uniq");

    parser.add_flag("-c,--count")
        .help(
            "Precede each output line with the count of the number of times "
            "the line occurred in the input, followed by a single space.");

    parser.add_flag("-d,--repeated")
        .help(
            "Output a single copy of each line that is repeated in the input.");

    parser.add_option("-D,--all-repeated")
        .value_help("")
        .help(
            R"(Output all lines that are repeated(like -d, but each copy of the repeated line is written).
The optional septype argument controls how to separate groups of repeated lines in the output; it must be one of the following values:
none      Do not separate groups of lines (this is the default).
prepend   Output an empty line before each group of lines.
separate  Output an empty line after each group of lines.)");

    parser.add_option<int>("-f,--skip-fields")
        .value_help("num")
        .help(
            "Ignore the first num fields in each input line when doing "
            "comparisons.  A field is a string of non-blank characters "
            "separated from adjacent fields by blanks.  Field numbers are one "
            "based, i.e., the first field is field one.");

    parser.add_flag("-i,--ignore-case")
        .help("Case insensitive comparison of lines.");

    parser.add_option<int>("-s,--skip-chars")
        .value_help("chars")
        .help(
            "Ignore the first chars characters in each input line when doing "
            "comparisons.If specified in conjunction with the - f, --unique "
            "option, the first chars characters after the first num fields "
            "will be ignored.Character numbers are one based, i.e., the first "
            "character is character one.");

    parser.add_flag("-u,--unique")
        .help("Only output lines that are not repeated in the input.");
  }

  argparse::ArgParser parser{};
};

TEST_F(UniqCommand, show_help) {
  std::cout << parser.usage();
}

TEST_F(UniqCommand, count) {
  std::vector<const char*> cmd{"uniq", "-c"};

  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_TRUE(parser["count"].get<bool>());

  ASSERT_EQ(parser["repeated"].count(), 0);
  ASSERT_EQ(parser["all-repeated"].count(), 0);
  ASSERT_EQ(parser["skip-fields"].count(), 0);
  ASSERT_EQ(parser["ignore-case"].count(), 0);
  ASSERT_EQ(parser["skip-chars"].count(), 0);
  ASSERT_EQ(parser["unique"].count(), 0);
}

TEST_F(UniqCommand, ignore_case) {
  std::vector<const char*> cmd{"uniq", "-c", "-i", "-s", "2"};

  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_TRUE(parser["count"].get<bool>());

  ASSERT_EQ(parser["repeated"].count(), 0);
  ASSERT_EQ(parser["all-repeated"].count(), 0);
  ASSERT_EQ(parser["skip-fields"].count(), 0);
  ASSERT_EQ(parser["ignore-case"].count(), 1);
  ASSERT_EQ(parser["skip-chars"].count(), 1);
  ASSERT_EQ(parser["unique"].count(), 0);

  ASSERT_EQ(parser["skip-chars"].get<int>(), 2);
  ASSERT_TRUE(parser["ignore-case"].get<bool>());
}
