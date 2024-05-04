#include <gtest/gtest.h>
#include "argparse.hpp"

argparse::ArgParser make_top_parser() {
  argparse::ArgParser parser;
  return parser;
}
TEST(top, add_flag) {}
