#include "./cli.hpp"

auto dldi::DldiCli::help_query_terms() -> void {
  std::cout << "$ dldi query terms [--position <position>] [--prefix <string>] [--limit <number>] [--offset <number>] <dldi path>" << std::endl
            << "        -h, --help                  This help" << std::endl
            << "        -p, --position <position>   The position to search for matches in; subject, predicate, or object." << std::endl
            << "        -r, --prefix <prefix>       Prefix to match terms against." << std::endl
            << "        -l, --limit <number>        The max number of matches to return." << std::endl
            << "        -o, --offset <number>       The number of initial matches to skip." << std::endl;
}

auto dldi::DldiCli::query_terms(int argc, char** argv) -> int {
  bool subjects{false};
  bool predicates{false};
  bool objects{false};
  std::string prefix{};

  int flag{0};
  while ((flag = getopt(argc, argv, "spor:h")) != -1) {
    switch (flag) {
    case 's':
      subjects = true;
      break;
    case 'p':
      predicates = true;
      break;
    case 'o':
      objects = true;
      break;
    case 'r':
      prefix = std::string{optarg};
      break;
    case 'h':
      help_query_terms();
      return EXIT_SUCCESS;
    default:
      std::cerr << "Unrecognized option\n";
      help_query_terms();
      return EXIT_FAILURE;
    }
  }

  if (argc - optind < 1) {
    std::cerr << "Need an input path\n";
    help_query_terms();
    return EXIT_FAILURE;
  }
  if (!subjects && !predicates && !objects) {
    std::cerr << "Must query in at least one triple-term position\n";
    help_query_terms();
    return EXIT_FAILURE;
  }
  const auto dldi_path{std::filesystem::path{argv[argc - 1]}};

  dldi::DLDI dldi{dldi_path};

  if (subjects)
    dldi.ensure_loaded(dldi::TripleTermPosition::subject);
  if (predicates)
    dldi.ensure_loaded(dldi::TripleTermPosition::predicate);
  if (objects)
    dldi.ensure_loaded(dldi::TripleTermPosition::object);

  auto it{dldi.terms(prefix, subjects, predicates, objects)};

  while (it->has_next()) {
    const auto next{it->next()};
    std::cout << next << std::endl;
  }

  return EXIT_SUCCESS;
}