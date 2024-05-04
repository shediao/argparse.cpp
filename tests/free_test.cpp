#include <gtest/gtest.h>
#include "argparse.hpp"

argparse::ArgParser make_free_parser() {
  argparse::ArgParser parser;
  return parser;
}
TEST(free, add_flag) {}
