#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <fcntl.h>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <thread>
#include <vector>

#include <zlib.h>

#include <DLDI.hpp>

#include "./rdf/SerdParser.hpp"
#include "./triples/TriplesReader.hpp"
#include "./triples/TriplesWriter.hpp"
#include <dictionary/Dictionary.hpp>

#include "./Composer.hpp"

inline auto get_source_info(const std::filesystem::path& path) -> dldi::SourceInfo {
  dldi::SourceInfo info;
  info.path = path;
  if (std::filesystem::is_directory(path)) {
    info.size = std::filesystem::file_size(dldi::TriplesReader::triples_file_path(path, dldi::TripleOrder::SPO));
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

  auto DLDI::compose(const std::vector<std::filesystem::path>& addition_paths,
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

    if (std::filesystem::is_directory(output_path)){
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
    dldi::Dictionary subjects{};
    dldi::Dictionary predicates{};
    dldi::Dictionary objects{};
    dldi::TriplesWriter triples{};
    rdf::SerdParser parser{
      input_path,
      [&subjects, &predicates, &objects, &triples](const std::string& subject_str, const std::string& predicate_str, const std::string& object_str) -> void {
        const auto subject{subjects.add(subject_str, 1)};
        const auto predicate{predicates.add(predicate_str, 1)};
        const auto object{objects.add(object_str, 1)};
        triples.add(subject, predicate, object);
      },
      base_iri,
      parser_type(input_path.extension())};

    std::filesystem::create_directory(output_path);

    for (auto order: dldi::EnumMapping::TRIPLE_ORDERS) {
      triples.sort(subjects, predicates, objects, order);
      ;
      triples.save(dldi::TriplesReader::triples_file_path(output_path, order));
    }

    subjects.save(Dictionary::dictionary_file_path(output_path, dldi::TripleTermPosition::subject));
    predicates.save(Dictionary::dictionary_file_path(output_path, dldi::TripleTermPosition::predicate));
    objects.save(Dictionary::dictionary_file_path(output_path, dldi::TripleTermPosition::object));
  }
}
