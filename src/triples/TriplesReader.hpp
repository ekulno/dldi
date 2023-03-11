#ifndef DLDI_TRIPLES_READER_HPP
#define DLDI_TRIPLES_READER_HPP

#include <filesystem>
#include <string>
#include <vector>
#include <memory>

#include <DLDI_enums.hpp>
#include <dictionary/Dictionary.hpp>
#include <QuantifiedTriple.hpp>
#include <TriplesIterator.hpp>

namespace dldi {

  class TriplesReader {
  public:
    TriplesReader(const std::filesystem::path& path);
    ~TriplesReader();
    auto query(const dldi::TriplePattern& pattern, const Dictionary& subjects, const Dictionary& predicates, const Dictionary& objects) -> dldi::TriplesIterator;
    auto query_ptr(const dldi::TriplePattern& pattern, const Dictionary& subjects, const Dictionary& predicates, const Dictionary& objects) const -> std::shared_ptr<dldi::TriplesIterator>;
    auto num_triples() -> std::size_t;

    static auto triples_file_path(const std::filesystem::path& dldi_dir, const dldi::TripleOrder& order) -> std::filesystem::path {
      return dldi_dir.string() + "/" + EnumMapping::order_to_string(order) + ".triples";
    }

    static auto validate_dir(const std::filesystem::path& dldi_dir) -> void {
      for ( auto order: EnumMapping::TRIPLE_ORDERS) {
        const auto filepath{triples_file_path(dldi_dir, order)};
        if (!std::filesystem::exists(filepath)) {
          throw std::runtime_error("Missing file " + filepath.string());
        }
      }
    }
  private:
    const dldi::QuantifiedTriple* m_triples;
    int fd;
    std::size_t m_num_triples;
  };
}

#endif