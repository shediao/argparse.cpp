#include <gtest/gtest.h>
#include "argparse.hpp"

argparse::ArgParser make_uniq_parser() {
  argparse::ArgParser parser;
  return parser;
}
TEST(uniq, add_flag) {}
