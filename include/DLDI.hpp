#ifndef DLDI_HPP
#define DLDI_HPP

#include <cassert>
#include <dictionary/trie/Trie.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include <memory>

#include <Iterator.hpp>
#include <dictionary/Dictionary.hpp>

namespace dldi {
  /**
   * Subject, predicate, object
  */
  using TriplePattern = std::tuple<std::size_t, std::size_t, std::size_t>;

  enum class TripleOrder {
    SPO,
    SOP,
    PSO,
    POS,
    OPS,
    OSP
  };

  class QuantifiedTriple {
  public:
    QuantifiedTriple(const std::size_t& subject, const std::size_t& predicate, const std::size_t& object, const std::size_t& quantity)
      : m_subject{subject},
        m_predicate{predicate},
        m_object{object},
        m_quantity{quantity} {
    }
    QuantifiedTriple(const QuantifiedTriple& other)
      : m_subject{other.subject()},
        m_predicate{other.predicate()},
        m_object{other.object()},
        m_quantity{other.quantity()} {
    }
    QuantifiedTriple() = default;
    auto operator=(const QuantifiedTriple& other) -> QuantifiedTriple& {
      if (this != &other) {
        m_subject = other.subject();
        m_predicate = other.predicate();
        m_object = other.object();
        m_quantity = other.quantity();
      }
      return *this;
    }
    auto print() const -> void {
      std::cout << subject() << " " << predicate() << " " << object();
    }

    auto subject() const -> std::size_t {
      return m_subject;
    }
    auto predicate() const -> std::size_t {
      return m_predicate;
    }
    auto object() const -> std::size_t {
      return m_object;
    }
    auto quantity() const -> std::size_t {
      return m_quantity;
    }
    auto equals(const QuantifiedTriple& other) const -> bool {
      return subject() == other.subject() && predicate() == other.predicate() && object() == other.object();
    }

    auto smaller_or_equal_to(
      const dldi::QuantifiedTriple& rhs,
      const dldi::TripleOrder& order,
      const dldi::Dictionary& subjects,
      const dldi::Dictionary& predicates,
      const dldi::Dictionary& objects) const -> bool {
      if (order == dldi::TripleOrder::SPO) {
        const auto subjectComparison{subjects.compare(subject(), rhs.subject())};
        if (subjectComparison != 0)
          return subjectComparison < 0;

        const auto predicateComparison{predicates.compare(predicate(), rhs.predicate())};
        if (predicateComparison != 0)
          return predicateComparison < 0;

        const auto objectComparison{objects.compare(object(), rhs.object())};
        if (objectComparison != 0)
          return objectComparison < 0;

        return true;
      } else if (order == dldi::TripleOrder::SOP) {
        const auto subjectComparison{subjects.compare(subject(), rhs.subject())};
        if (subjectComparison != 0)
          return subjectComparison < 0;

        const auto objectComparison{objects.compare(object(), rhs.object())};
        if (objectComparison != 0)
          return objectComparison < 0;

        const auto predicateComparison{predicates.compare(predicate(), rhs.predicate())};
        if (predicateComparison != 0)
          return predicateComparison < 0;

        return true;

      } else if (order == dldi::TripleOrder::PSO) {
        const auto predicateComparison{predicates.compare(predicate(), rhs.predicate())};
        if (predicateComparison != 0)
          return predicateComparison < 0;

        const auto subjectComparison{subjects.compare(subject(), rhs.subject())};
        if (subjectComparison != 0)
          return subjectComparison < 0;

        const auto objectComparison{objects.compare(object(), rhs.object())};
        if (objectComparison != 0)
          return objectComparison < 0;

        return true;

      } else if (order == dldi::TripleOrder::POS) {
        const auto predicateComparison{predicates.compare(predicate(), rhs.predicate())};
        if (predicateComparison != 0)
          return predicateComparison < 0;

        const auto objectComparison{objects.compare(object(), rhs.object())};
        if (objectComparison != 0)
          return objectComparison < 0;

        const auto subjectComparison{subjects.compare(subject(), rhs.subject())};
        if (subjectComparison != 0)
          return subjectComparison < 0;

        return true;

      } else if (order == dldi::TripleOrder::OSP) {
        const auto objectComparison{objects.compare(object(), rhs.object())};
        if (objectComparison != 0)
          return objectComparison < 0;

        const auto subjectComparison{subjects.compare(subject(), rhs.subject())};
        if (subjectComparison != 0)
          return subjectComparison < 0;

        const auto predicateComparison{predicates.compare(predicate(), rhs.predicate())};
        if (predicateComparison != 0)
          return predicateComparison < 0;

        return true;
      } else if (order == dldi::TripleOrder::OPS) {
        const auto objectComparison{objects.compare(object(), rhs.object())};
        if (objectComparison != 0)
          return objectComparison < 0;

        const auto predicateComparison{predicates.compare(predicate(), rhs.predicate())};
        if (predicateComparison != 0)
          return predicateComparison < 0;

        const auto subjectComparison{subjects.compare(subject(), rhs.subject())};
        if (subjectComparison != 0)
          return subjectComparison < 0;

        return true;
      }
      throw std::runtime_error("Unrecognized order");
    }

    auto precedes(
      const dldi::TriplePattern& pattern,
      const dldi::TripleOrder& order,
      const dldi::Dictionary& subjects,
      const dldi::Dictionary& predicates,
      const dldi::Dictionary& objects) const -> bool {
      const auto s{std::get<0>(pattern)};
      const auto p{std::get<1>(pattern)};
      const auto o{std::get<2>(pattern)};
      if (s == 0 && p == 0 && o == 0) {
        throw std::runtime_error("All wildcards");
      }
      if (s == 0 && p == 0 && o != 0) {
        assert(order == dldi::TripleOrder::OSP);
        const auto objectComparison{objects.compare(object(), o)};
        return objectComparison < 0;
      }
      if (s == 0 && p != 0 && o == 0) {
        assert(order == dldi::TripleOrder::PSO);
        const auto predicateComparison{predicates.compare(predicate(), p)};
        return predicateComparison < 0;
      }
      if (s == 0 && p != 0 && o != 0) {
        assert(order == dldi::TripleOrder::POS);
        const auto predicateComparison{predicates.compare(predicate(), p)};
        if (predicateComparison != 0)
          return predicateComparison < 0;
        const auto objectComparison{objects.compare(object(), o)};
        return objectComparison < 0;
      }
      if (s != 0 && p == 0 && o == 0) {
        assert(order == dldi::TripleOrder::SPO);
        const auto subjectComparison{subjects.compare(subject(), s)};
        return subjectComparison < 0;
      }
      if (s != 0 && p == 0 && o != 0) {
        assert(order == dldi::TripleOrder::SOP);
        const auto subjectComparison{subjects.compare(subject(), s)};
        if (subjectComparison != 0)
          return subjectComparison < 0;
        const auto objectComparison{objects.compare(object(), o)};
        return objectComparison < 0;
      }
      if (s != 0 && p != 0 && o == 0) {
        assert(order == dldi::TripleOrder::SPO);
        const auto subjectComparison{subjects.compare(subject(), s)};
        if (subjectComparison != 0)
          return subjectComparison < 0;
        const auto predicateComparison{predicates.compare(predicate(), p)};
        return predicateComparison < 0;
      }
      if (s != 0 && p != 0 && o != 0) {
        assert(order == dldi::TripleOrder::SPO);
        const auto subjectComparison{subjects.compare(subject(), s)};
        if (subjectComparison != 0)
          return subjectComparison < 0;
        const auto predicateComparison{predicates.compare(predicate(), p)};
        if (predicateComparison != 0)
          return predicateComparison < 0;
        const auto objectComparison{objects.compare(object(), o)};
        return objectComparison < 0;
      }
      throw std::runtime_error("surprise");
    }

    auto set_quantity(const std::size_t& quantity) {
      m_quantity = quantity;
    }
    auto matched_by(const dldi::TriplePattern& pattern) const {
      return (std::get<0>(pattern) == 0 || subject() == std::get<0>(pattern)) &&
             (std::get<1>(pattern) == 0 || predicate() == std::get<1>(pattern)) &&
             (std::get<2>(pattern) == 0 || object() == std::get<2>(pattern));
    }

  private:
    std::size_t m_subject{0};
    std::size_t m_predicate{0};
    std::size_t m_object{0};
    std::size_t m_quantity;
  };

  class TriplesReader;

  enum class SourceType {
    PlainTextLinkedData_Sorted,
    PlainTextLinkedData_Unsorted,
    DynamicLinkedDataIndex
  };
  struct SourceInfo {
    SourceType type;
    /**
     * An indicator of the size of the source data. 
     * Sizes between DLDIs and PTLDs are not comparable. 
    */
    std::size_t size;
    std::filesystem::path path;
  };

  enum class TripleTermPosition {
    subject,
    predicate,
    object
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

  class TriplesIterator : public Iterator<QuantifiedTriple> {
  public:
    TriplesIterator(const QuantifiedTriple* const triples, const TriplePattern& m_pattern, const std::size_t& num_triples, const Dictionary& subjects, const Dictionary& predicates, const Dictionary& objects);
    auto inner_proceed() -> void override;

  private:
    const QuantifiedTriple* const m_triples;
    const TriplePattern m_pattern;
    std::size_t m_index;
    std::size_t m_num_triples;
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

    // auto get_source_info() -> SourceInfo;
    static auto get_triples_name(const dldi::TripleOrder& order) -> std::string {
      if (order == dldi::TripleOrder::SPO)
        return "spo";
      if (order == dldi::TripleOrder::SOP)
        return "sop";
      if (order == dldi::TripleOrder::POS)
        return "pos";
      if (order == dldi::TripleOrder::PSO)
        return "pso";
      if (order == dldi::TripleOrder::OSP)
        return "osp";
      if (order == dldi::TripleOrder::OPS)
        return "ops";
      throw std::runtime_error("Unknown order");
    }

    static auto get_dict_name(const dldi::TripleTermPosition& position) -> std::string {
      if (position == dldi::TripleTermPosition::subject) {
        return "subjects";
      } else if (position == dldi::TripleTermPosition::predicate) {
        return "predicates";
      } else if (position == dldi::TripleTermPosition::object) {
        return "objects";
      }
      throw std::runtime_error("Unrecognized position");
    }

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
