
#include "./cli.hpp"

auto dldi::DldiCli::help_query_triples() -> void {
  std::cout << "$ dldi query triples [--subject <term>] [--predicate <term>] [--object <term>] [--limit <number>] [--offset <number>] <dldi path>" << std::endl
            << "        -s, --subject <term>        Match triples with the given subject." << std::endl
            << "        -s, --predicate <term>      Match triples with the given predicate." << std::endl
            << "        -s, --object <term>         Match triples with the given object." << std::endl
            << "        -l, --limit <number>        The max number of matches to return." << std::endl
            << "        -l, --offset <number>       The number of initial matches to skip." << std::endl
            << "        -h, --help                  This help" << std::endl;
  return;
}
auto dldi::DldiCli::query_triples(int argc, char** argv) -> int {
  std::string subject{};
  std::string predicate{};
  std::string object{};

  int flag{0};
  while ((flag = getopt(argc, argv, "s:p:o:h")) != -1) {
    switch (flag) {
    case 's':
      subject = std::string{optarg};
      break;
    case 'p':
      predicate = std::string{optarg};
      break;
    case 'o':
      object = std::string{optarg};
      break;
    case 'h':
      help_query_triples();
      return EXIT_SUCCESS;
    default:
      std::cerr << "Unrecognized option\n";
      help_query_triples();
      return EXIT_FAILURE;
    }
  }

  if (argc - optind < 1) {
    std::cerr << "Need an input path\n";
    help_query_terms();
    return EXIT_FAILURE;
  }

  const auto dldi_path{std::filesystem::path{argv[argc - 1]}};

  dldi::DLDI dldi{dldi_path};

  auto subject_id{0};
  auto predicate_id{0};
  auto object_id{0};

  dldi.ensure_loaded(dldi::TripleTermPosition::subject);
  if (!subject.empty()) {
    subject_id = dldi.string_to_id(subject, dldi::TripleTermPosition::subject);
  }
  dldi.ensure_loaded(dldi::TripleTermPosition::predicate);
  if (!predicate.empty()) {
    predicate_id = dldi.string_to_id(predicate, dldi::TripleTermPosition::predicate);
  }
  dldi.ensure_loaded(dldi::TripleTermPosition::object);
  if (!object.empty()) {
    object_id = dldi.string_to_id(object, dldi::TripleTermPosition::object);
  }

  const dldi::TriplePattern pattern{subject_id, predicate_id, object_id};
  dldi.prepare_for_query(pattern);
  auto triple_iterator{dldi.query_ptr(pattern)};
  while (triple_iterator->has_next()) {
    const auto triple{triple_iterator->read()};
    triple_iterator->proceed();

    const auto subject_iri{(subject.empty() ? dldi.id_to_string(triple.subject(), dldi::TripleTermPosition::subject) : subject)};
    const auto predicate_iri{(predicate.empty() ? dldi.id_to_string(triple.predicate(), dldi::TripleTermPosition::predicate) : predicate)};
    const auto object_base{(object.empty() ? dldi.id_to_string(triple.object(), dldi::TripleTermPosition::object) : object)};
    const auto object_term{(object_base.at(0) == '"') ? object_base : '<' + object_base + '>'};
    std::cout << "<" << subject_iri << "> <" << predicate_iri << "> " << object_term << " ." << std::endl;
  }
  return EXIT_SUCCESS;
}
