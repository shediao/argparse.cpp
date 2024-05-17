#include <cstdlib>
#include <exception>
#include <iostream>
#include "argparse.hpp"

int main(int argc, char* argv[]) {
  argparse::ArgParser parser("This is an example program for argparse");

  try {
    parser.parse(argc, argv);
  } catch (std::exception const& e) {
    std::cerr << e.what();
    exit(EXIT_FAILURE);
  }

  if (parser["help"].get<bool>()) {
    std::cout << parser.usage() << std::endl;
  }

  return 0;
}
