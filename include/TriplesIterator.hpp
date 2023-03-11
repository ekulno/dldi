#ifndef DLDI_TRIPLES_ITERATOR_HPP
#define DLDI_TRIPLES_ITERATOR_HPP

#include <Iterator.hpp>

namespace dldi {

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
}
#endif 