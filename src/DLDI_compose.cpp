#include <filesystem>

#include <DLDI_Composer.hpp>
#include <DLDI_enums.hpp>

#include "./Composer.hpp"
#include "./rdf/SerdParser.hpp"
#include "./triples/TriplesReader.hpp"
#include "./triples/TriplesWriter.hpp"

inline auto get_source_info(const std::filesystem::path& path) -> dldi::SourceInfo {
  dldi::SourceInfo info;
  info.path = path;
  if (std::filesystem::is_directory(path)) {
    // todo base on triples
    info.size = std::filesystem::file_size(dldi::Dictionary::dictionary_file_path(path, dldi::TripleTermPosition::object)) 
    +std::filesystem::file_size(dldi::Dictionary::dictionary_file_path(path, dldi::TripleTermPosition::subject)) 
    +std::filesystem::file_size(dldi::Dictionary::dictionary_file_path(path, dldi::TripleTermPosition::predicate));
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
namespace dldi {

  auto DLDI_Composer::compose(const std::vector<std::filesystem::path>& addition_paths,
                              const std::vector<std::filesystem::path>& subtraction_paths,
                              const std::filesystem::path& output_path) -> void {
    std::vector<dldi::SourceInfo> additions;
    for (const auto path: addition_paths) {
      additions.push_back(get_source_info(path));
    }
    std::vector<dldi::SourceInfo> subtractions;
    for (const auto path: subtraction_paths) {
      subtractions.push_back(get_source_info(path));
    }

    if (additions.empty()) {
      throw std::runtime_error("Need at least one additive source");
    }

    if (std::filesystem::is_directory(output_path)) {
      throw std::runtime_error("There is already a directory at " + output_path.string());
    }
    std::filesystem::create_directories(output_path);

    // if there's only one positive and no negatives, then this must be a single-source ptld->dldi conversion.
    if (additions.size() == 1 && subtractions.size() == 0) {
      const auto first{additions.at(0)};
      if (first.type == dldi::SourceType::DynamicLinkedDataIndex) {
        throw std::runtime_error("Doesn't make sense, use `cp -R` instead.");
      }
      DLDI::from_ptld(first.path, output_path, "https://example.com/");
      return;
    }

    Composer composer;
    composer.zip(additions, subtractions, output_path);
  }

  inline auto parser_type(const std::string& extension) -> rdf::SerializationFormat {
    if (extension == ".nq")
      return rdf::SerializationFormat::NQuads;
    if (extension == ".nt")
      return rdf::SerializationFormat::NTriples;
    if (extension == ".trig")
      return rdf::SerializationFormat::TriG;
    if (extension == ".ttl")
      return rdf::SerializationFormat::Turtle;
    throw std::runtime_error("Serd parser only supports N-Triples, N-Quads, TriG, and Turtle.");
  }

  auto DLDI::from_ptld(const std::filesystem::path& input_path, const std::filesystem::path& output_path, const std::string& base_iri) -> void {
    dldi::DictionariesHandle dicts{};
    dldi::TriplesWriter triples{};
    rdf::SerdParser parser{
      input_path,
      [&dicts, &triples](const std::string& subject_str, const std::string& predicate_str, const std::string& object_str) -> void {
        const auto subject{dicts.subjects()->add(subject_str, 1)};
        const auto predicate{dicts.predicates()->add(predicate_str, 1)};
        const auto object{dicts.objects()->add(object_str, 1)};
        triples.add(subject, predicate, object);
      },
      base_iri,
      parser_type(input_path.extension())};
    std::filesystem::create_directory(output_path);

    triples.save(output_path, dicts);
    dicts.save(output_path);
  }
}
