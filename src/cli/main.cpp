#include <getopt.h>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include <DLDI.hpp>

#include "./cli.hpp"

auto help() -> void {
  dldi::DldiCli::help_compose();
  dldi::DldiCli::help_query();
}

auto main(int argc, char** argv) -> int {
  if (argc == 1) {
    help();
    return EXIT_FAILURE;
  }
  try {
    const std::string argv1{argv[1]};
    if (argv1 == "compose") {
      return dldi::DldiCli::compose(argc, argv);
    } else if (argv1 == "query") {
      return dldi::DldiCli::query(argc, argv);
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
  help();
  return EXIT_FAILURE;
}
