#include <gtest/gtest.h>
#include "argparse.hpp"

argparse::ArgParser make_getopt_parser() {
  argparse::ArgParser parser;
  parser.add_flag("a,alternative")
      .help("Allow long options to start with a single '-'.");

  parser.add_flag("h,help").help(
      "Display help text and exit. No other output is generated.");

  parser.add_option("l,longoptions")
      .help(
          "The long (multi-character) options to be recognized. More than one "
          "option name may be specified at once, by separating the names with "
          "commas. This option may be given more than once, the longopts are "
          "cumulative. Each long option name in longopts may be followed by "
          "one colon to indicate it has a required argument, and by two colons "
          "to indicate it has an optional argument.");

  parser.add_option("n,name").help(
      "The name that will be used by the getopt(3) routines when it reports "
      "errors. Note that errors of getopt(1) are still reported as coming from "
      "getopt.");

  parser.add_option("o,options")
      .help(
          "The short (one-character) options to be recognized. If this option "
          "is not found, the first parameter of getopt that does not start "
          "with a '-' (and is not an option argument) is used as the short "
          "options string. Each short option character in shortopts may be "
          "followed by one colon to indicate it has a required argument, and "
          "by two colons to indicate it has an optional argument. The first "
          "character of shortopts may be '+' or '-' to influence the way "
          "options are parsed and output is generated (see section SCANNING "
          "MODES for details).");

  parser.add_flag("q,quiet").help("Disable error reporting by getopt(3).");

  parser.add_flag("Q,quiet-output")
      .help(
          "Do not generate normal output. Errors are still reported by "
          "getopt(3), unless you also use -q.");

  parser.add_option("s,shell").help(
      "Set quoting conventions to those of shell. If the -s option is not "
      "given, the BASH conventions are used. Valid arguments are currently "
      "'sh' 'bash', 'csh', and 'tcsh'.");

  parser.add_flag("T,test").help(
      "Test if your getopt(1) is this enhanced version or an old version. This "
      "generates no output, and sets the error status to 4. Other "
      "implementations of getopt(1), and this version if the environment "
      "variable GETOPT_COMPATIBLE is set, will return '--' and error status "
      "0.");

  parser.add_flag("u,unquoted")
      .help(
          "Do not quote the output. Note that whitespace and special "
          "(shell-dependent) characters can cause havoc in this mode (like "
          "they do with other getopt(1) implementations).");

  parser.add_flag("V,version")
      .help(
          "Display version information and exit. No other output is "
          "generated.");

  parser.add_positional("parameters");
  return parser;
}
TEST(getopt, add_flag) {
  auto parser = make_getopt_parser();
  ASSERT_TRUE(parser["a"].is_flag());
  ASSERT_TRUE(parser["alternative"].is_flag());
  ASSERT_TRUE(parser["h"].is_flag());
  ASSERT_TRUE(parser["help"].is_flag());
  ASSERT_TRUE(parser["q"].is_flag());
  ASSERT_TRUE(parser["quiet"].is_flag());
  ASSERT_TRUE(parser["Q"].is_flag());
  ASSERT_TRUE(parser["quiet-output"].is_flag());
  ASSERT_TRUE(parser["T"].is_flag());
  ASSERT_TRUE(parser["test"].is_flag());
  ASSERT_TRUE(parser["u"].is_flag());
  ASSERT_TRUE(parser["unquoted"].is_flag());
  ASSERT_TRUE(parser["V"].is_flag());
  ASSERT_TRUE(parser["version"].is_flag());

  ASSERT_TRUE(parser["l"].is_option());
  ASSERT_TRUE(parser["longoptions"].is_option());
  ASSERT_TRUE(parser["n"].is_option());
  ASSERT_TRUE(parser["name"].is_option());
  ASSERT_TRUE(parser["o"].is_option());
  ASSERT_TRUE(parser["options"].is_option());
  ASSERT_TRUE(parser["s"].is_option());
  ASSERT_TRUE(parser["shell"].is_option());

  ASSERT_TRUE(parser["parameters"].is_positional());
}
TEST(getopt, parse) {
  auto parser = make_getopt_parser();
  std::vector<const char*> cmd{
      "getopt", "-q", "-o", "hdrv", "-l", "help,debug,release,version",
      "--",     "-a", "-b", "-c",   "-d", "-e"};

  ASSERT_NO_THROW(parser.parse(cmd.size(), cmd.data()));

  ASSERT_TRUE(parser["quiet"].get<bool>());
  ASSERT_EQ("hdrv", parser["options"].get<std::string>());
  ASSERT_EQ("help,debug,release,version",
            parser["longoptions"].get<std::string>());

  ASSERT_EQ(5, parser["parameters"].get<std::vector<std::string>>().size());
  ASSERT_EQ("-a", parser["parameters"].get<std::vector<std::string>>()[0]);
  ASSERT_EQ("-b", parser["parameters"].get<std::vector<std::string>>()[1]);
  ASSERT_EQ("-c", parser["parameters"].get<std::vector<std::string>>()[2]);
  ASSERT_EQ("-d", parser["parameters"].get<std::vector<std::string>>()[3]);
  ASSERT_EQ("-e", parser["parameters"].get<std::vector<std::string>>()[4]);
}
