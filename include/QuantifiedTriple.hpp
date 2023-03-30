#ifndef DLDI_QUANTIFIED_TRIPLE_HPP
#define DLDI_QUANTIFIED_TRIPLE_HPP

#include <cassert>
#include <iostream>

#include <DLDI_enums.hpp>
#include <dictionary/DictionariesHandle.hpp>
#include <dictionary/Dictionary.hpp>

namespace dldi {

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
    auto term(const dldi::TripleTermPosition position) const -> std::size_t {
      if (position == dldi::TripleTermPosition::subject)
        return subject();
      if (position == dldi::TripleTermPosition::predicate)
        return predicate();
      if (position == dldi::TripleTermPosition::object)
        return object();
      throw std::runtime_error("Unrecognized triple-term-position");
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
      const dldi::DictionariesHandle& dicts,
      const bool& ignore_primary = false) const -> bool {
      const auto [primary, secondary, tertiary]{EnumMapping::order_to_positions(order)};
      if (!ignore_primary) {
        const auto comparison{dicts.get_dict(primary)->compare(term(primary), rhs.term(primary))};
        if (comparison != 0)
          return comparison < 0;
      }
      {
        const auto comparison{dicts.get_dict(secondary)->compare(term(secondary), rhs.term(secondary))};
        if (comparison != 0)
          return comparison < 0;
      }
      {
        const auto comparison{dicts.get_dict(tertiary)->compare(term(tertiary), rhs.term(tertiary))};
        return comparison < 0;
      }
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
    std::size_t m_subject{0};
    std::size_t m_predicate{0};
    std::size_t m_object{0};
    std::size_t m_quantity;
  };
}

#endif