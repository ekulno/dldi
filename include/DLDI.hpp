#ifndef DLDI_HPP
#define DLDI_HPP

#include <cassert>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <DLDI_enums.hpp>
#include <QuantifiedTriple.hpp>
#include <TriplesIterator.hpp>
#include <dictionary/DictionariesHandle.hpp>
#include <dictionary/Dictionary.hpp>
#include <dictionary/trie/Trie.hpp>

namespace dldi {

  class TriplesReader;

  class AnyPositionTermIterator {
  public:
    AnyPositionTermIterator(std::vector<std::shared_ptr<csd::TermStringIterator>>& iterators);
    auto peek() const -> std::string;
    auto next() -> std::string;
    auto has_next() const -> bool;

  private:
    std::vector<std::shared_ptr<csd::TermStringIterator>> m_iterators;
    auto sort_iterators() -> void;
  };

  class DLDI {
  public:
    /**
     * Open an existing DLDI instance for querying. 
    */
    DLDI(const std::filesystem::path& input_dir);

    /**
     * Query for triples which match a given pattern. 
    */
    auto search(const dldi::TriplePattern& pattern) const -> std::shared_ptr<dldi::TriplesIterator>;
    
    /**
     * Query for terms matching a given prefix in a given triple-term-position. 
    */
    auto terms(std::string prefix, dldi::TripleTermPosition position) const -> std::shared_ptr<csd::TermStringIterator>;

    /**
     * Query for terms matching a given prefix in the given triple-term-positions. 
    */
    auto terms(std::string prefix, bool subjects, bool predicates, bool objects) const -> std::shared_ptr<dldi::AnyPositionTermIterator>;

    auto string_to_id(const std::string& term, const dldi::TripleTermPosition& position) const -> std::size_t;
    auto id_to_string(const std::size_t& id, const dldi::TripleTermPosition& position) const -> std::string;

    /**
     * Create a DLDI instance from a single plaintext linked data file. 
    */
    static auto from_ptld(const std::filesystem::path& input_path, const std::filesystem::path& output_path, const std::string& base_iri) -> void;

    auto ensure_loaded(const dldi::TripleTermPosition& position) -> void;

    auto prepare_for_query(const dldi::TriplePattern& pattern) -> void;
    auto ensure_loaded_triples(const dldi::TripleOrder& order) -> void;

    auto get_dict(const dldi::TripleTermPosition& position) const -> std::shared_ptr<dldi::Dictionary> {
      return m_dicts->get_dict(position);
    }

    auto get_dicts(const dldi::TripleTermPosition& position) const -> std::shared_ptr<dldi::DictionariesHandle> {
      return m_dicts;
    }


    auto get_triples() const -> std::shared_ptr<TriplesReader> {
      return m_triples_reader;
    }

  private:
    std::shared_ptr<DictionariesHandle> m_dicts;
    std::shared_ptr<TriplesReader> m_triples_reader;
    std::filesystem::path m_datadir;
  };
}

#endif
