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
#include "./triples/TriplesWriter.hpp"
#include "./triples/TriplesReader.hpp"
#include <dictionary/Dictionary.hpp>

#include "./Composer.hpp"

namespace dldi {

  auto DLDI::compose(const std::vector<dldi::SourceInfo>& additions,
                     const std::vector<dldi::SourceInfo>& subtractions,
                     const std::filesystem::path& output_path) -> void {
    if (additions.empty()) {
      throw std::runtime_error("Need at least one additive source");
    }

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
      triples.sort(subjects, predicates, objects, order);;
      triples.save(dldi::TriplesReader::triples_file_path(output_path, order));
    }

    subjects.save(Dictionary::dictionary_file_path(output_path, dldi::TripleTermPosition::subject));
    predicates.save(Dictionary::dictionary_file_path(output_path, dldi::TripleTermPosition::predicate));
    objects.save(Dictionary::dictionary_file_path(output_path, dldi::TripleTermPosition::object));
  }
}
