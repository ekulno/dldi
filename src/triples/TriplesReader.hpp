#ifndef DLDI_TRIPLES_READER_HPP
#define DLDI_TRIPLES_READER_HPP

#include <filesystem>
#include <string>
#include <vector>
#include <memory>

#include <DLDI.hpp>
#include <dictionary/Dictionary.hpp>

namespace dldi {

  class TriplesReader {
  public:
    TriplesReader(const std::filesystem::path& path);
    ~TriplesReader();
    auto query(const dldi::TriplePattern& pattern, const Dictionary& subjects, const Dictionary& predicates, const Dictionary& objects) -> dldi::TriplesIterator;
    auto query_ptr(const dldi::TriplePattern& pattern, const Dictionary& subjects, const Dictionary& predicates, const Dictionary& objects) const -> std::shared_ptr<dldi::TriplesIterator>;
    auto num_triples() -> std::size_t;

  private:
    const dldi::QuantifiedTriple* m_triples;
    int fd;
    std::size_t m_num_triples;
  };
}

#endif