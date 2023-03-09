#include "./cli.hpp"

auto dldi::DldiCli::help_query() -> void {
  help_query_terms();
  help_query_triples();
}

auto dldi::DldiCli::query(int argc, char** argv) -> int {
  if (argc < 3) {
    help_query();
    return EXIT_FAILURE;
  }

  const std::string argv2{argv[2]};
  if (argv2 == "terms") {
    return query_terms(argc, argv);
  } else if (argv2 == "triples") {
    return query_triples(argc, argv);
  }
  help_query();
  return EXIT_FAILURE;
}