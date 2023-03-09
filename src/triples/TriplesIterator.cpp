#include <iostream>

#include <DLDI.hpp>

#include "./TriplesReader.hpp"

namespace dldi {

  inline auto find_first_result_index(
    const QuantifiedTriple* triples,
    const dldi::TriplePattern& pattern,
    const std::size_t& num_triples,
    const Dictionary& subjects,
    const Dictionary& predicates,
    const Dictionary& objects) -> std::size_t {
    const auto order{dldi::DLDI::decide_order_from_triple_pattern(pattern)};
    if (order == dldi::TripleOrder::SPO)
      return 0; // match everything

    std::size_t lower_bound{0};
    std::size_t upper_bound{num_triples - 1};
    while (true) {
      const std::size_t i{((upper_bound - lower_bound) / 2) + lower_bound};
      if (triples[i].matched_by(pattern)) {
        upper_bound = i;
        if (upper_bound == lower_bound)
          return i;
      } else if (triples[i].precedes(pattern, order, subjects, predicates, objects)) {
        lower_bound = i + 1;
      } else {
        upper_bound = i - 1;
      }
    }

    return 0;
  }

  TriplesIterator::TriplesIterator(
    const QuantifiedTriple* triples,
    const dldi::TriplePattern& pattern,
    const std::size_t& num_triples,
    const Dictionary& subjects,
    const Dictionary& predicates,
    const Dictionary& objects)
    : m_triples{triples}, m_pattern{pattern}, m_index{0}, m_num_triples{num_triples} {
    assert(subjects != nullptr && predicates != nullptr && objects != nullptr);

    const auto first_result_index{find_first_result_index(triples, pattern, num_triples, subjects, predicates, objects)};
    // std::cout << "first_result_index : " << first_result_index << std::endl;
    m_index = first_result_index;
    m_has_next = m_index < num_triples && m_triples[m_index].matched_by(pattern);
    if (m_has_next) {
      m_next = m_triples[m_index];
      // m_triples[m_index].print();
      // std::cout << std::endl;
    }
  }

  auto TriplesIterator::inner_proceed() -> void {
    ++m_index;
    m_has_next = m_index < m_num_triples && m_triples[m_index].matched_by(m_pattern);
    if (m_has_next) {
      m_next = m_triples[m_index];
    }
  }
}
