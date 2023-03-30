#ifndef DLDI_TRIPLES_WRITER_HPP
#define DLDI_TRIPLES_WRITER_HPP

#include <filesystem>
#include <string>
#include <vector>

#include <QuantifiedTriple.hpp>
#include <dictionary/DictionariesHandle.hpp>

namespace dldi {

  class TriplesWriter {
  public:
    TriplesWriter() = default;
    TriplesWriter(const std::filesystem::path& outpath);

    auto add(const std::size_t& subject, const std::size_t& predicate, const std::size_t& object) -> void;
    auto save(const std::filesystem::path& path, const dldi::DictionariesHandle& dicts) -> void;

  private:
    std::vector<QuantifiedTriple> m_triples;
    auto sort(const dldi::DictionariesHandle& dicts, const dldi::TripleOrder& order, bool ignore_primary) -> void;
  };
}

#endif