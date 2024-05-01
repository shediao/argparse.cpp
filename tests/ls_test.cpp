
/* ****************************************************************************
 *
 * NAME
 *     ls – list directory contents
 *
 *SYNOPSIS
 *     ls [-@ABCFGHILOPRSTUWabcdefghiklmnopqrstuvwxy1%,] [--color=when] [-D format] [file ...]
 *
 *DESCRIPTION
 *     For each operand that names a file of a type other than directory, ls displays its name as well as any requested, associated information.  For each operand that names a file of type directory, ls displays the names of files contained within that
 *     directory, as well as any requested, associated information.
 *
 *     If no operands are given, the contents of the current directory are displayed.  If more than one operand is given, non-directory operands are displayed first; directory and non-directory operands are sorted separately and in lexicographical
 *     order.
 *
 *     The following options are available:
 *
 *     -@      Display extended attribute keys and sizes in long (-l) output.
 *
 *     -A      Include directory entries whose names begin with a dot (‘.’) except for . and ...  Automatically set for the super-user unless -I is specified.
 *
 *     -B      Force printing of non-printable characters (as defined by ctype(3) and current locale settings) in file names as \xxx, where xxx is the numeric value of the character in octal.  This option is not defined in IEEE Std 1003.1-2008
 *             (“POSIX.1”).
 *
 *     -C      Force multi-column output; this is the default when output is to a terminal.
 *
 *     -D format
 *             When printing in the long (-l) format, use format to format the date and time output.  The argument format is a string used by strftime(3).  Depending on the choice of format string, this may result in a different number of columns in the
 *             output.  This option overrides the -T option.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).
 *
 *     -F      Display a slash (‘/’) immediately after each pathname that is a directory, an asterisk (‘*’) after each that is executable, an at sign (‘@’) after each symbolic link, an equals sign (‘=’) after each socket, a percent sign (‘%’) after each
 *             whiteout, and a vertical bar (‘|’) after each that is a FIFO.
 *
 *     -G      Enable colorized output.  This option is equivalent to defining CLICOLOR or COLORTERM in the environment and setting --color=auto.  (See below.)  This functionality can be compiled out by removing the definition of COLORLS.  This option
 *             is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).
 *
 *     -H      Symbolic links on the command line are followed.  This option is assumed if none of the -F, -d, or -l options are specified.
 *
 *     -I      Prevent -A from being automatically set for the super-user.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).
 *
 *     -L      Follow all symbolic links to final target and list the file or directory the link references rather than the link itself.  This option cancels the -P option.
 *
 *     -O      Include the file flags in a long (-l) output.  This option is incompatible with IEEE Std 1003.1-2008 (“POSIX.1”).  See chflags(1) for a list of file flags and their meanings.
 *
 *     -P      If argument is a symbolic link, list the link itself rather than the object the link references.  This option cancels the -H and -L options.
 *
 *     -R      Recursively list subdirectories encountered.
 *
 *     -S      Sort by size (largest file first) before sorting the operands in lexicographical order.
 *
 *     -T      When printing in the long (-l) format, display complete time information for the file, including month, day, hour, minute, second, and year.  The -D option gives even more control over the output format.  This option is not defined in
 *             IEEE Std 1003.1-2008 (“POSIX.1”).
 *
 *     -U      Use time when file was created for sorting or printing.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).
 *
 *     -W      Display whiteouts when scanning directories.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).
 *
 *     -a      Include directory entries whose names begin with a dot (‘.’).
 *     -b      As -B, but use C escape codes whenever possible.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).
 *
 *     -c      Use time when file status was last changed for sorting or printing.
 *
 *     --color=when
 *             Output colored escape sequences based on when, which may be set to either always, auto, or never.
 *
 *             always will make ls always output color.  If TERM is unset or set to an invalid terminal, then ls will fall back to explicit ANSI escape sequences without the help of termcap(5).  always is the default if --color is specified without an
 *             argument.
 *
 *             auto will make ls output escape sequences based on termcap(5), but only if stdout is a tty and either the -G flag is specified or the COLORTERM environment variable is set and not empty.
 *
 *             never will disable color regardless of environment variables.  never is the default when neither --color nor -G is specified.
*
*             For compatibility with GNU coreutils, ls supports yes or force as equivalent to always, no or none as equivalent to never, and tty or if-tty as equivalent to auto.
*
*     -d      Directories are listed as plain files (not searched recursively).
*
*     -e      Print the Access Control List (ACL) associated with the file, if present, in long (-l) output.
*
*     -f      Output is not sorted.  This option turns on -a.  It also negates the effect of the -r, -S and -t options.  As allowed by IEEE Std 1003.1-2008 (“POSIX.1”), this option has no effect on the -d, -l, -R and -s options.
*
*     -g      This option has no effect.  It is only available for compatibility with 4.3BSD, where it was used to display the group name in the long (-l) format output.  This option is incompatible with IEEE Std 1003.1-2008 (“POSIX.1”).
*
*     -h      When used with the -l option, use unit suffixes: Byte, Kilobyte, Megabyte, Gigabyte, Terabyte and Petabyte in order to reduce the number of digits to four or fewer using base 2 for sizes.  This option is not defined in IEEE Std
*             1003.1-2008 (“POSIX.1”).
*
*     -i      For each file, print the file's file serial number (inode number).
*
*     -k      This has the same effect as setting environment variable BLOCKSIZE to 1024, except that it also nullifies any -h options to its left.
*
*     -l      (The lowercase letter “ell”.) List files in the long format, as described in the The Long Format subsection below.
*
*     -m      Stream output format; list files across the page, separated by commas.
*
*     -n      Display user and group IDs numerically rather than converting to a user or group name in a long (-l) output.  This option turns on the -l option.
*
*     -o      List in long format, but omit the group id.
*
*     -p      Write a slash (‘/’) after each filename if that file is a directory.
*
*     -q      Force printing of non-graphic characters in file names as the character ‘?’; this is the default when output is to a terminal.
*
*     -r      Reverse the order of the sort.
*
*     -s      Display the number of blocks used in the file system by each file.  Block sizes and directory totals are handled as described in The Long Format subsection below, except (if the long format is not also requested) the directory totals are
*             not output when the output is in a single column, even if multi-column output is requested.  (-l) format, display complete time information for the file, including month, day, hour, minute, second, and year.  The -D option gives even more
*             control over the output format.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).
*
*     -t      Sort by descending time modified (most recently modified first).  If two files have the same modification timestamp, sort their names in ascending lexicographical order.  The -r option reverses both of these sort orders.
*
*             Note that these sort orders are contradictory: the time sequence is in descending order, the lexicographical sort is in ascending order.  This behavior is mandated by IEEE Std 1003.2 (“POSIX.2”).  This feature can cause problems listing
*             files stored with sequential names on FAT file systems, such as from digital cameras, where it is possible to have more than one image with the same timestamp.  In such a case, the photos cannot be listed in the sequence in which they
*             were taken.  To ensure the same sort order for time and for lexicographical sorting, set the environment variable LS_SAMESORT or use the -y option.  This causes ls to reverse the lexicographical sort order when sorting files with the same
*             modification timestamp.
*
*     -u      Use time of last access, instead of time of last modification of the file for sorting (-t) or long printing (-l).
*
*     -v      Force unedited printing of non-graphic characters; this is the default when output is not to a terminal.
*
*     -w      Force raw printing of non-printable characters.  This is the default when output is not to a terminal.  This option is not defined in IEEE Std 1003.1-2001 (“POSIX.1”).
*
*     -x      The same as -C, except that the multi-column output is produced with entries sorted across, rather than down, the columns.
*
*     -y      When the -t option is set, sort the alphabetical output in the same order as the time output.  This has the same effect as setting LS_SAMESORT.  See the description of the -t option for more details.  This option is not defined in IEEE
*             Std 1003.1-2001 (“POSIX.1”).
*
*     -%      Distinguish dataless files and directories with a '%' character in long (-l) output, and don't materialize dataless directories when listing them.
*
*     -1      (The numeric digit “one”.) Force output to be one entry per line.  This is the default when output is not to a terminal.
*
*     -,      (Comma) When the -l option is set, print file sizes grouped and separated by thousands using the non-monetary separator returned by localeconv(3), typically a comma or period.  If no locale is set, or the locale does not have a non-
*             monetary separator, this option has no effect.  This option is not defined in IEEE Std 1003.1-2001 (“POSIX.1”).
*
* ****************************************************************************/


#include "argparse.hpp"
#include <gtest/gtest.h>


argparse::ArgParser make_ls_parser() {
    argparse::ArgParser parser;
    parser.add_flag("-@")
        .help(R"(Display extended attribute keys and sizes in long (-l) output.)");

    parser.add_flag("-A")
        .help(R"(Include directory entries whose names begin with a dot (‘.’) except for . and ...  Automatically set for the super-user unless -I is specified.)");

    parser.add_flag("-B")
        .help(R"(Force printing of non-printable characters (as defined by ctype(3) and current locale settings) in file names as \xxx, where xxx is the numeric value of the character in octal.  This option is not defined in IEEE Std 1003.1-2008
        (“POSIX.1”).)");

    parser.add_flag("-C")
        .help(R"(Force multi-column output; this is the default when output is to a terminal.)");

    parser.add_option<std::string>("D")
        .help(R"(When printing in the long (-l) format, use format to format the date and time output.  The argument format is a string used by strftime(3).  Depending on the choice of format string, this may result in a different number of columns in the output.  This option overrides the -T option.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).)");


    parser.add_flag("-F")
        .help(R"(Display a slash (‘/’) immediately after each pathname that is a directory, an asterisk (‘*’) after each that is executable, an at sign (‘@’) after each symbolic link, an equals sign (‘=’) after each socket, a percent sign (‘%’) after each whiteout, and a vertical bar (‘|’) after each that is a FIFO.)");

    parser.add_flag("-G")
        .help(R"(Enable colorized output.  This option is equivalent to defining CLICOLOR or COLORTERM in the environment and setting --color=auto.  (See below.)  This functionality can be compiled out by removing the definition of COLORLS.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).)");

    parser.add_flag("-H")
        .help(R"(Symbolic links on the command line are followed.  This option is assumed if none of the -F, -d, or -l options are specified.)");

    parser.add_flag("-I")
        .help(R"(Prevent -A from being automatically set for the super-user.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).)");

    parser.add_flag("-L")
        .help(R"(Follow all symbolic links to final target and list the file or directory the link references rather than the link itself.  This option cancels the -P option.)");

    parser.add_flag("-O")
        .help(R"(Include the file flags in a long (-l) output.  This option is incompatible with IEEE Std 1003.1-2008 (“POSIX.1”).  See chflags(1) for a list of file flags and their meanings.)");

    parser.add_flag("-P")
        .help(R"(If argument is a symbolic link, list the link itself rather than the object the link references.  This option cancels the -H and -L options.)");

    parser.add_flag("-R")
        .help(R"(Recursively list subdirectories encountered.)");

    parser.add_flag("-S")
        .help(R"(Sort by size (largest file first) before sorting the operands in lexicographical order.)");

    parser.add_flag("-T")
        .help(R"(When printing in the long (-l) format, display complete time information for the file, including month, day, hour, minute, second, and year.  The -D option gives even more control over the output format.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).)");

    parser.add_flag("-U")
        .help(R"(Use time when file was created for sorting or printing.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).)");

    parser.add_flag("-W")
        .help(R"(Display whiteouts when scanning directories.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).)");

    parser.add_flag("-a")
        .help(R"(Include directory entries whose names begin with a dot (‘.’).)");
    parser.add_flag("-b")
        .help(R"(As -B, but use C escape codes whenever possible.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).)");

    parser.add_flag("-c")
        .help(R"(Use time when file status was last changed for sorting or printing.)");

    parser.add_option<std::string>("color")
        .help(R"(Output colored escape sequences based on when, which may be set to either always, auto, or never.
        always will make ls always output color.  If TERM is unset or set to an invalid terminal, then ls will fall back to explicit ANSI escape sequences without the help of termcap(5).  always is the default if --color is specified without an
        argument.

        auto will make ls output escape sequences based on termcap(5), but only if stdout is a tty and either the -G flag is specified or the COLORTERM environment variable is set and not empty.

        never will disable color regardless of environment variables.  never is the default when neither --color nor -G is specified.

        For compatibility with GNU coreutils, ls supports yes or force as equivalent to always, no or none as equivalent to never, and tty or if-tty as equivalent to auto.)");


    parser.add_flag("-d")
        .help(R"(Directories are listed as plain files (not searched recursively).)");

    parser.add_flag("-e")
        .help(R"(Print the Access Control List (ACL) associated with the file, if present, in long (-l) output.)");

    parser.add_flag("-f")
        .help(R"(Output is not sorted.  This option turns on -a.  It also negates the effect of the -r, -S and -t options.  As allowed by IEEE Std 1003.1-2008 (“POSIX.1”), this option has no effect on the -d, -l, -R and -s options.)");

    parser.add_flag("-g")
        .help(R"(This option has no effect.  It is only available for compatibility with 4.3BSD, where it was used to display the group name in the long (-l) format output.  This option is incompatible with IEEE Std 1003.1-2008 (“POSIX.1”).)");

    parser.add_flag("-h")
        .help(R"(When used with the -l option, use unit suffixes: Byte, Kilobyte, Megabyte, Gigabyte, Terabyte and Petabyte in order to reduce the number of digits to four or fewer using base 2 for sizes.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).)");

    parser.add_flag("-i")
        .help(R"(For each file, print the file's file serial number (inode number).)");

    parser.add_flag("-k")
        .help(R"(This has the same effect as setting environment variable BLOCKSIZE to 1024, except that it also nullifies any -h options to its left.)");

    parser.add_flag("-l")
        .help(R"((The lowercase letter “ell”.) List files in the long format, as described in the The Long Format subsection below.)");

    parser.add_flag("-m")
        .help(R"(Stream output format; list files across the page, separated by commas.)");

    parser.add_flag("-n")
        .help(R"(Display user and group IDs numerically rather than converting to a user or group name in a long (-l) output.  This option turns on the -l option.)");

    parser.add_flag("-o")
        .help(R"(List in long format, but omit the group id.)");

    parser.add_flag("-p")
        .help(R"(Write a slash (‘/’) after each filename if that file is a directory.)");

    parser.add_flag("-q")
        .help(R"(Force printing of non-graphic characters in file names as the character ‘?’; this is the default when output is to a terminal.)");

    parser.add_flag("-r")
        .help(R"(Reverse the order of the sort.)");

    parser.add_flag("-s")
        .help(R"(Display the number of blocks used in the file system by each file.

Block sizes and directory totals are handled as described in The Long Format subsection below, except (if the long format is not also requested) the directory totals are not output when the output is in a single column, even if multi-column output is requested.  (-l) format, display complete time information for the file, including month, day, hour, minute, second, and year.  The -D option gives even more control over the output format.  This option is not defined in IEEE Std 1003.1-2008 (“POSIX.1”).)");

        parser.add_flag("-t")
        .help(R"(Sort by descending time modified (most recently modified first).  If two files have the same modification timestamp, sort their names in ascending lexicographical order.  The -r option reverses both of these sort orders.

        Note that these sort orders are contradictory: the time sequence is in descending order, the lexicographical sort is in ascending order.  This behavior is mandated by IEEE Std 1003.2 (“POSIX.2”).  This feature can cause problems listing files stored with sequential names on FAT file systems, such as from digital cameras, where it is possible to have more than one image with the same timestamp.  In such a case, the photos cannot be listed in the sequence in which they were taken.  To ensure the same sort order for time and for lexicographical sorting, set the environment variable LS_SAMESORT or use the -y option.  This causes ls to reverse the lexicographical sort order when sorting files with the same modification timestamp.)");

        parser.add_flag("-u")
        .help(R"(Use time of last access, instead of time of last modification of the file for sorting (-t) or long printing (-l).)");

    parser.add_flag("-v")
        .help(R"(Force unedited printing of non-graphic characters; this is the default when output is not to a terminal.)");

    parser.add_flag("-w")
        .help(R"(Force raw printing of non-printable characters.  This is the default when output is not to a terminal.  This option is not defined in IEEE Std 1003.1-2001 (“POSIX.1”).)");

    parser.add_flag("-x")
        .help(R"(The same as -C, except that the multi-column output is produced with entries sorted across, rather than down, the columns.)");

    parser.add_flag("-y")
        .help(R"(When the -t option is set, sort the alphabetical output in the same order as the time output.  This has the same effect as setting LS_SAMESORT.  See the description of the -t option for more details.  This option is not defined in IEEE Std 1003.1-2001 (“POSIX.1”).)");

    parser.add_flag("-%")
        .help(R"(Distinguish dataless files and directories with a '%' character in long (-l) output, and don't materialize dataless directories when listing them.)");

    parser.add_flag("-1")
        .help(R"((The numeric digit “one”.) Force output to be one entry per line.  This is the default when output is not to a terminal.)");

    //parser.add_flag("-,")
    //    .help(R"((Comma) When the -l option is set, print file sizes grouped and separated by thousands using the non-monetary separator returned by localeconv(3), typically a comma or period.  If no locale is set, or the locale does not have a non-monetary separator, this option has no effect.  This option is not defined in IEEE Std 1003.1-2001 (“POSIX.1”).)");

    parser.add_positional<std::vector<std::string>>("files_or_directories");
    return parser;
}


TEST(ArgParse, lscmd) {
    auto parser = make_ls_parser();
    ASSERT_TRUE(parser.get("@").has_value());
    ASSERT_TRUE(parser.get("A").has_value());
    ASSERT_TRUE(parser.get("B").has_value());
    ASSERT_TRUE(parser.get("C").has_value());
    ASSERT_TRUE(parser.get("F").has_value());
    ASSERT_TRUE(parser.get("G").has_value());
    ASSERT_TRUE(parser.get("H").has_value());
    ASSERT_TRUE(parser.get("I").has_value());
    ASSERT_TRUE(parser.get("L").has_value());
    ASSERT_TRUE(parser.get("O").has_value());
    ASSERT_TRUE(parser.get("P").has_value());
    ASSERT_TRUE(parser.get("R").has_value());
    ASSERT_TRUE(parser.get("S").has_value());
    ASSERT_TRUE(parser.get("T").has_value());
    ASSERT_TRUE(parser.get("U").has_value());
    ASSERT_TRUE(parser.get("W").has_value());
    ASSERT_TRUE(parser.get("a").has_value());
    ASSERT_TRUE(parser.get("b").has_value());
    ASSERT_TRUE(parser.get("c").has_value());
    ASSERT_TRUE(parser.get("d").has_value());
    ASSERT_TRUE(parser.get("e").has_value());
    ASSERT_TRUE(parser.get("f").has_value());
    ASSERT_TRUE(parser.get("g").has_value());
    ASSERT_TRUE(parser.get("h").has_value());
    ASSERT_TRUE(parser.get("i").has_value());
    ASSERT_TRUE(parser.get("k").has_value());
    ASSERT_TRUE(parser.get("l").has_value());
    ASSERT_TRUE(parser.get("m").has_value());
    ASSERT_TRUE(parser.get("n").has_value());
    ASSERT_TRUE(parser.get("o").has_value());
    ASSERT_TRUE(parser.get("p").has_value());
    ASSERT_TRUE(parser.get("q").has_value());
    ASSERT_TRUE(parser.get("r").has_value());
    ASSERT_TRUE(parser.get("s").has_value());
    ASSERT_TRUE(parser.get("t").has_value());
    ASSERT_TRUE(parser.get("u").has_value());
    ASSERT_TRUE(parser.get("v").has_value());
    ASSERT_TRUE(parser.get("w").has_value());
    ASSERT_TRUE(parser.get("x").has_value());
    ASSERT_TRUE(parser.get("y").has_value());
    ASSERT_TRUE(parser.get("1").has_value());
    ASSERT_TRUE(parser.get("%").has_value());
    // ASSERT_TRUE(parser.get(",").has_value());
    ASSERT_TRUE(parser.get("D").has_value());
    ASSERT_TRUE(parser.get("color").has_value());
    ASSERT_TRUE(parser.get("files_or_directories").has_value());

    ASSERT_TRUE(parser.get("@").value()->is_flag());
    ASSERT_TRUE(parser.get("A").value()->is_flag());
    ASSERT_TRUE(parser.get("B").value()->is_flag());
    ASSERT_TRUE(parser.get("C").value()->is_flag());
    ASSERT_TRUE(parser.get("F").value()->is_flag());
    ASSERT_TRUE(parser.get("G").value()->is_flag());
    ASSERT_TRUE(parser.get("H").value()->is_flag());
    ASSERT_TRUE(parser.get("I").value()->is_flag());
    ASSERT_TRUE(parser.get("L").value()->is_flag());
    ASSERT_TRUE(parser.get("O").value()->is_flag());
    ASSERT_TRUE(parser.get("P").value()->is_flag());
    ASSERT_TRUE(parser.get("R").value()->is_flag());
    ASSERT_TRUE(parser.get("S").value()->is_flag());
    ASSERT_TRUE(parser.get("T").value()->is_flag());
    ASSERT_TRUE(parser.get("U").value()->is_flag());
    ASSERT_TRUE(parser.get("W").value()->is_flag());
    ASSERT_TRUE(parser.get("a").value()->is_flag());
    ASSERT_TRUE(parser.get("b").value()->is_flag());
    ASSERT_TRUE(parser.get("c").value()->is_flag());
    ASSERT_TRUE(parser.get("d").value()->is_flag());
    ASSERT_TRUE(parser.get("e").value()->is_flag());
    ASSERT_TRUE(parser.get("f").value()->is_flag());
    ASSERT_TRUE(parser.get("g").value()->is_flag());
    ASSERT_TRUE(parser.get("h").value()->is_flag());
    ASSERT_TRUE(parser.get("i").value()->is_flag());
    ASSERT_TRUE(parser.get("k").value()->is_flag());
    ASSERT_TRUE(parser.get("l").value()->is_flag());
    ASSERT_TRUE(parser.get("m").value()->is_flag());
    ASSERT_TRUE(parser.get("n").value()->is_flag());
    ASSERT_TRUE(parser.get("o").value()->is_flag());
    ASSERT_TRUE(parser.get("p").value()->is_flag());
    ASSERT_TRUE(parser.get("q").value()->is_flag());
    ASSERT_TRUE(parser.get("r").value()->is_flag());
    ASSERT_TRUE(parser.get("s").value()->is_flag());
    ASSERT_TRUE(parser.get("t").value()->is_flag());
    ASSERT_TRUE(parser.get("u").value()->is_flag());
    ASSERT_TRUE(parser.get("v").value()->is_flag());
    ASSERT_TRUE(parser.get("w").value()->is_flag());
    ASSERT_TRUE(parser.get("x").value()->is_flag());
    ASSERT_TRUE(parser.get("y").value()->is_flag());
    ASSERT_TRUE(parser.get("1").value()->is_flag());
    ASSERT_TRUE(parser.get("%").value()->is_flag());

    ASSERT_TRUE(parser.get("D").value()->is_option());
    ASSERT_TRUE(parser.get("color").value()->is_option());
    ASSERT_TRUE(parser.get("files_or_directories").value()->is_positional());
}
