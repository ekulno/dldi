#ifndef DLDI_TRIPLES_WRITER_HPP
#define DLDI_TRIPLES_WRITER_HPP

#include <filesystem>
#include <string>
#include <vector>

#include <DLDI.hpp>

#include <dictionary/Dictionary.hpp>

namespace dldi {

  class TriplesWriter {
  public:
    TriplesWriter() = default;
    TriplesWriter(const std::filesystem::path& outpath);

    auto add(const std::size_t& subject, const std::size_t& predicate, const std::size_t& object) -> void;
    auto sort(const dldi::Dictionary& subjects, const dldi::Dictionary& predicates, const dldi::Dictionary& objects, const dldi::TripleOrder& order) -> void;
    auto save(const std::filesystem::path& path) -> void;

  private:
    std::vector<QuantifiedTriple> m_triples;
  };
}

#endif