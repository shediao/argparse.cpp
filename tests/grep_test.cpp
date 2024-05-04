#include <gtest/gtest.h>
#include "argparse.hpp"

/* ****************************************************************************
 * Usage: grep [OPTION]... PATTERNS [FILE]...
 * Search for PATTERNS in each FILE.
 * Example: grep -i 'hello world' menu.h main.c
 * PATTERNS can contain multiple patterns separated by newlines.
 *
 * Pattern selection and interpretation:
 *   -E, --extended-regexp     PATTERNS are extended regular expressions
 *   -F, --fixed-strings       PATTERNS are strings
 *   -G, --basic-regexp        PATTERNS are basic regular expressions
 *   -P, --perl-regexp         PATTERNS are Perl regular expressions
 *   -e, --regexp=PATTERNS     use PATTERNS for matching
 *   -f, --file=FILE           take PATTERNS from FILE
 *   -i, --ignore-case         ignore case distinctions in patterns and data
 *       --no-ignore-case      do not ignore case distinctions (default)
 *   -w, --word-regexp         match only whole words
 *   -x, --line-regexp         match only whole lines
 *   -z, --null-data           a data line ends in 0 byte, not newline
 *
 * Miscellaneous:
 *   -s, --no-messages         suppress error messages
 *   -v, --invert-match        select non-matching lines
 *   -V, --version             display version information and exit
 *       --help                display this help text and exit
 *
 * Output control:
 *   -m, --max-count=NUM       stop after NUM selected lines
 *   -b, --byte-offset         print the byte offset with output lines
 *   -n, --line-number         print line number with output lines
 *       --line-buffered       flush output on every line
 *   -H, --with-filename       print file name with output lines
 *   -h, --no-filename         suppress the file name prefix on output
 *       --label=LABEL         use LABEL as the standard input file name prefix
 *   -o, --only-matching       show only nonempty parts of lines that match
 *   -q, --quiet, --silent     suppress all normal output
 *       --binary-files=TYPE   assume that binary files are TYPE;
 *                             TYPE is 'binary', 'text', or 'without-match'
 *   -a, --text                equivalent to --binary-files=text
 *   -I                        equivalent to --binary-files=without-match
 *   -d, --directories=ACTION  how to handle directories;
 *                             ACTION is 'read', 'recurse', or 'skip'
 *   -D, --devices=ACTION      how to handle devices, FIFOs and sockets;
 *                             ACTION is 'read' or 'skip'
 *   -r, --recursive           like --directories=recurse
 *   -R, --dereference-recursive  likewise, but follow all symlinks
 *       --include=GLOB        search only files that match GLOB (a file
 * pattern)
 *       --exclude=GLOB        skip files that match GLOB
 *       --exclude-from=FILE   skip files that match any file pattern from FILE
 *       --exclude-dir=GLOB    skip directories that match GLOB
 *   -L, --files-without-match  print only names of FILEs with no selected lines
 *   -l, --files-with-matches  print only names of FILEs with selected lines
 *   -c, --count               print only a count of selected lines per FILE
 *   -T, --initial-tab         make tabs line up (if needed)
 *   -Z, --null                print 0 byte after FILE name
 *
 * Context control:
 *   -B, --before-context=NUM  print NUM lines of leading context
 *   -A, --after-context=NUM   print NUM lines of trailing context
 *   -C, --context=NUM         print NUM lines of output context
 *   -NUM                      same as --context=NUM
 *       --group-separator=SEP  print SEP on line between matches with context
 *       --no-group-separator  do not print separator for matches with context
 *       --color[=WHEN],
 *       --colour[=WHEN]       use markers to highlight the matching strings;
 *                             WHEN is 'always', 'never', or 'auto'
 *   -U, --binary              do not strip CR characters at EOL (MSDOS/Windows)
 *
 * When FILE is '-', read standard input.  With no FILE, read '.' if
 * recursive, '-' otherwise.  With fewer than two FILEs, assume -h.
 * Exit status is 0 if any line is selected, 1 otherwise;
 * if any error occurs and -q is not given, the exit status is 2.
 * ***************************************************************************/

argparse::ArgParser make_grep_parser() {
  argparse::ArgParser parser;
  parser.set_program_name("grep");
  parser.add_flag("E,extended-regexp")
      .help("PATTERNS are extended regular expressions");
  parser.add_flag("F,fixed-strings").help("PATTERNS are strings");
  parser.add_flag("G,basic-regexp")
      .help("PATTERNS are basic regular expressions");
  parser.add_flag("P,perl-regexp")
      .help("PATTERNS are Perl regular expressions");
  parser.add_option("e,regexp").help("use PATTERNS for matching").value_help("PATTERNS");
  parser.add_option("f,file").help("take PATTERNS from FILE").value_help("FILE");
  parser.add_option("i,ignore-case,!no-ignore-case")
      .help("ignore case distinctions in patterns and data");
  parser.add_option("w,word-regexp").help("match only whole words");
  parser.add_option("x,line-regexp").help("match only whole lines");
  parser.add_option("z,null-data")
      .help("a data line ends in 0 byte, not newline");
  parser.add_flag("s,no-messages").help("suppress error messages");
  parser.add_flag("v,invert-match").help("select non-matching lines");
  parser.add_flag("V,version").help("display version information and exit");
  parser.add_flag("help").help("display this help text and exit");

  parser.add_option<int>("m,max-count").help("stop after NUM selected lines").value_help("NUM");
  parser.add_flag("b,byte-offset")
      .help("print the byte offset with output lines");
  parser.add_flag("n,line-number").help("print line number with output lines");
  parser.add_flag("line-buffered").help("flush output on every line");
  parser.add_flag("H,with-filename").help("print file name with output lines");
  parser.add_flag("h,no-filename")
      .help("suppress the file name prefix on output");
  parser.add_option("label").help(
      "use LABEL as the standard input file name prefix").value_help("LABEL");
  parser.add_flag("o,only-matching")
      .help("show only nonempty parts of lines that match");
  parser.add_flag("q,quiet,").help("--silent     suppress all normal output");
  parser.add_option("binary-files")
      .help(
          "assume that binary files are TYPE; TYPE is 'binary', 'text', or "
          "'without-match'").value_help("TYPE");
  parser.add_alias_flag("a,text", "binary-files", "text")
      .help("equivalent to --binary-files=text");
  parser.add_alias_flag("-I", "binary-files", "without-match")
      .help("equivalent to --binary-files=without-match");
  parser.add_option("d,directories")
      .help(
          "how to handle directories; ACTION is 'read', 'recurse', or 'skip'").value_help("ACTION");
  parser.add_option("D,devices")
      .help(
          "how to handle devices, FIFOs and sockets; ACTION is 'read' or "
          "'skip'").value_help("ACTION");
  parser.add_flag("r,recursive").help("like --directories=recurse");
  parser.add_flag("R,dereference-recursive")
      .help("likewise, but follow all symlinks");
  parser.add_option("include").help(
      "search only files that match GLOB (a file pattern)").value_help("GLOB");
  parser.add_option("exclude").help("skip files that match GLOB").value_help("GLOB");
  parser.add_option("exclude-from")
      .help("skip files that match any file pattern from FILE").value_help("FILE");
  parser.add_option("exclude-dir").help("skip directories that match GLOB").value_help("GLOB");
  parser.add_flag("L,files-without-match")
      .help("print only names of FILEs with no selected lines");
  parser.add_flag("l,files-with-matches")
      .help("print only names of FILEs with selected lines");
  parser.add_flag("c,count").help(
      "print only a count of selected lines per FILE");
  parser.add_flag("T,initial-tab").help("make tabs line up (if needed)");
  parser.add_flag("Z,null").help("print 0 byte after FILE name");

  parser.add_option<int>("B,before-context")
      .help("print NUM lines of leading context").value_help("NUM");
  parser.add_option<int>("A,after-context")
      .help("print NUM lines of trailing context").value_help("NUM");
  parser.add_option<int>("C,context").help("print NUM lines of output context").value_help("NUM");
  // TODO:
  // -NUM                      same as --context=NUM
  parser.add_option("group-separator")
      .help("print SEP on line between matches with context").value_help("SEP");
  parser.add_flag("no-group-separator")
      .help("do not print separator for matches with context");
  parser.add_option("color,colour")
      .help(
          "use markers to highlight the matching strings; WHEN is 'always', "
          "'never', or 'auto'").value_help("WHEN");
  parser.add_flag("U,binary")
      .help("do not strip CR characters at EOL (MSDOS/Windows)");

  parser.add_positional("pattern_and_files").value_help("FILES");

  return parser;
}
TEST(grep, add_flag) {
  auto parser = make_grep_parser();
  std::cout << parser.usage() << std::endl;
}
