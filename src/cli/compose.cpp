#include "./cli.hpp"

auto dldi::DldiCli::help_compose() -> void {
  std::cout << "$ dldi compose [--base-iri <base-IRI>] [--add <path>]* [--subtract <path>]* <output path>\n"
            << "        -h, --help                  This help" << std::endl
            << "        -a, --add <path>            Path to a linked-data resource to include." << std::endl
            << "        -s, --subtract <path>       Path to a linked-data resource to exclude." << std::endl
            << "        -B, --base-iri <base-IRI>   Base IRI of the dataset." << std::endl;
}

inline auto get_source_info(const std::filesystem::path& path) -> dldi::SourceInfo {
  dldi::SourceInfo info;
  info.path = path;
  if (std::filesystem::is_directory(path)) {
    info.size = std::filesystem::file_size(path.string() + "/spo.triples");
    info.type = dldi::SourceType::DynamicLinkedDataIndex;
    return info;
  }

  if (std::filesystem::is_regular_file(path)) {
    info.size = std::filesystem::file_size(path);
    if (path.string().find(".sorted") != path.string().npos) {
      info.type = dldi::SourceType::PlainTextLinkedData_Sorted;
      return info;
    }
    info.type = dldi::SourceType::PlainTextLinkedData_Unsorted;
    return info;
  }
  throw std::runtime_error("Path doesn't exist: " + path.string());
}

auto dldi::DldiCli::compose(int argc, char** argv) -> int {
  std::string base_iri;
  std::vector<std::filesystem::path> addition_paths;
  std::vector<std::filesystem::path> subtraction_paths;

  int flag{0};
  while ((flag = getopt(argc, argv, "B:a:s:h")) != -1) {
    switch (flag) {
    case 'a':
      addition_paths.push_back(std::filesystem::canonical(std::filesystem::path{optarg}));
      break;
    case 's':
      subtraction_paths.push_back(std::filesystem::canonical(std::filesystem::path{optarg}));
      break;
    case 'B':
      base_iri = optarg;
      break;
    case 'h':
      help_compose();
      return EXIT_SUCCESS;
    default:
      std::cerr << "Unrecognized option\n";
      help_compose();
      return EXIT_FAILURE;
    }
  }

  if (addition_paths.empty()) {
    std::cerr << "Need at least one additions path\n";
    help_compose();
    return EXIT_FAILURE;
  }
  if (argc - optind < 1) {
    std::cerr << "Need an output path\n";
    help_compose();
    return EXIT_FAILURE;
  }
  const auto output_path{std::filesystem::path{argv[argc - 1]}};

  std::vector<dldi::SourceInfo> addition_sources;
  for (const auto path: addition_paths) {
    addition_sources.push_back(get_source_info(path));
  }
  std::vector<dldi::SourceInfo> subtraction_sources;
  for (const auto path: subtraction_paths) {
    subtraction_sources.push_back(get_source_info(path));
  }

  dldi::DLDI::compose(addition_sources, subtraction_sources, output_path);
  return EXIT_SUCCESS;
}
