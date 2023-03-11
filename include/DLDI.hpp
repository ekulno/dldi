#ifndef DLDI_HPP
#define DLDI_HPP

#include <cassert>
#include <dictionary/trie/Trie.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <DLDI_enums.hpp>
#include <dictionary/Dictionary.hpp>
#include <QuantifiedTriple.hpp>
#include <TriplesIterator.hpp>

namespace dldi {


  class TriplesReader;

  struct SourceInfo {
    SourceType type;
    /**
     * An indicator of the size of the source data. 
     * Sizes between DLDIs and PTLDs are not comparable. 
    */
    std::size_t size;
    std::filesystem::path path;
  };

  class AnyPositionTermIterator {
  public:
    AnyPositionTermIterator(std::vector<csd::TermStringIterator>& iterators);
    auto peek() const -> std::string;
    auto next() -> std::string;
    auto has_next() const -> bool;

  private:
    std::vector<csd::TermStringIterator> m_iterators;
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
    auto query_ptr(const dldi::TriplePattern& pattern) const -> std::shared_ptr<dldi::TriplesIterator>;
    auto query_ptr(const dldi::TripleOrder& order) const -> std::shared_ptr<dldi::TriplesIterator>;

    /**
     * Query for terms matching a given prefix in a given triple-term-position. 
    */
    auto query(std::string prefix, dldi::TripleTermPosition position) const -> csd::TermStringIterator;

    /**
     * Query for terms matching a given prefix in the given triple-term-positions. 
    */
    auto query(std::string prefix, bool subjects, bool predicates, bool objects) const -> dldi::AnyPositionTermIterator;

    auto string_to_id(const std::string& term, const dldi::TripleTermPosition& position) const -> std::size_t;
    auto id_to_string(const std::size_t& id, const dldi::TripleTermPosition& position) const -> std::string;

    /**
     * Compose a DLDI instance from sets of resources which should be added and subtracted.
    */
    static auto compose(
      const std::vector<dldi::SourceInfo>& additions,
      const std::vector<dldi::SourceInfo>& subtractions,
      const std::filesystem::path& output_path) -> void;

    /**
     * Create a DLDI instance from a single plaintext linked data file. 
    */
    static auto from_ptld(const std::filesystem::path& input_path, const std::filesystem::path& output_path, const std::string& base_iri) -> void;

    auto ensure_loaded(const dldi::TripleTermPosition& position) -> void;

    auto prepare_for_query(const dldi::TriplePattern& pattern) -> void;
    auto ensure_loaded_triples(const dldi::TripleOrder& order) -> void;


    auto get_dict(const dldi::TripleTermPosition& position) const -> std::shared_ptr<dldi::Dictionary> {
      if (position == dldi::TripleTermPosition::subject) {
        return m_subjects;
      }
      if (position == dldi::TripleTermPosition::predicate) {
        return m_predicates;
      }
      return m_objects;
    }

    static auto decide_order_from_triple_pattern(const dldi::TriplePattern& pattern) -> dldi::TripleOrder {
      /**
       * Mapping: 
       * 
       * 000 // spo  
       * 001 // osp 
       * 010 // pso
       * 011 // pos
       * 100 // spo
       * 101 // sop
       * 110 // spo
       * 111 // spo
       * 
       * This ensures that for each combination of fixed terms, 
       * The sort order of non-fixed terms remains a subset of s-p-o. 
       * In turn, this ensures that results are always ordered lexically
       * by s-p-o, while all results are found in a contiguous sequence. 
       * 
       * NB: I think OPS can be dropped, due to overlapping use-cases with others. 
      */

      const auto s{std::get<0>(pattern) > 0};
      const auto p{std::get<1>(pattern) > 0};
      const auto o{std::get<2>(pattern) > 0};

      if (!s && !p && !o)
        return dldi::TripleOrder::SPO;
      if (!s && !p && o)
        return dldi::TripleOrder::OSP;
      if (!s && p && !o)
        return dldi::TripleOrder::PSO;
      if (!s && p && o)
        return dldi::TripleOrder::POS;
      if (s && !p && !o)
        return dldi::TripleOrder::SPO;
      if (s && !p && o)
        return dldi::TripleOrder::SOP;
      if (s && p && !o)
        return dldi::TripleOrder::SPO;
      if (s && p && o)
        return dldi::TripleOrder::SPO;
      throw std::runtime_error("Unaccounted-for case");
    }


  private:
    std::shared_ptr<Dictionary> m_subjects;
    std::shared_ptr<Dictionary> m_predicates;
    std::shared_ptr<Dictionary> m_objects;
    std::shared_ptr<TriplesReader> m_triples_spo;
    std::shared_ptr<TriplesReader> m_triples_sop;
    std::shared_ptr<TriplesReader> m_triples_pso;
    std::shared_ptr<TriplesReader> m_triples_pos;
    std::shared_ptr<TriplesReader> m_triples_osp;
    std::shared_ptr<TriplesReader> m_triples_ops;
    std::filesystem::path m_datadir;
    auto get_triples(const dldi::TripleOrder& order) const -> std::shared_ptr<TriplesReader>;
  };
}

#endif
