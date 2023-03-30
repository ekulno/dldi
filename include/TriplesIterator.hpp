#ifndef DLDI_TRIPLES_ITERATOR_HPP
#define DLDI_TRIPLES_ITERATOR_HPP

#include <Iterator.hpp>
#include <QuantifiedTriple.hpp>
#include <TripleIterators/AllTriplesIterator.hpp>
#include <TripleIterators/SingleTripleIterator.hpp>
#include <TripleIterators/TriplesFilteredByOnePositionIterator.hpp>
#include <TripleIterators/TriplesFilteredByTwoPositionsIterator.hpp>

/**
     * notes
     *  - figure out what to do with quantity field.  
     *    want to drop quantity and duplicate triples instead. >1 trips will be rare.
     *    an alternative is to preface with special 0-0-q triple. 
     *    this should save sizeof(std::size_t) bytes per statement.
     *   but, this will complicate some of the iteration code. 
     *   hopefully not too much. 
     * 
    * -  When the teriary term is predicates, 
      Use a single secondary+tertiary file. 
      This is because subjects and objects rarely connect with more than one predicate. 
      This should save some space. 
   */

namespace dldi {

  class TriplesIterator : public Iterator<QuantifiedTriple> {
  public:
    TriplesIterator(
      const std::tuple<std::size_t, std::size_t, std::size_t>& filter,
      const dldi::TripleOrder& order,
      const std::shared_ptr<const dldi::DictionariesHandle> dicts,
      std::pair<const std::size_t*, const std::size_t> primary_ids,
      std::pair<const std::size_t*, const std::size_t> secondary_ids,
      std::pair<const std::size_t*, const std::size_t> secondary_refs,
      std::pair<const std::size_t*, const std::size_t> tertiary_ids,
      std::pair<const std::size_t*, const std::size_t> tertiary_refs) {
      const auto [primary_filter, secondary_filter, tertiary_filter] = filter;

      const auto [primary_position, secondary_position, tertiary_position] = EnumMapping::order_to_positions(order);

      /**
       * Todo try to generalize the iterators
      */

      if (primary_filter == 0) {
        // iterate over all statements
        m_it = std::make_shared<AllTriplesIterator>(order, dicts, primary_ids, secondary_ids, secondary_refs, tertiary_ids, tertiary_refs);
        return;
      }
      // iterate over a subset of statements
      if (secondary_filter == 0) {
        // iterate over all statements with the given primary term.
        m_it = std::make_shared<TriplesFilteredByOnePositionIterator>(order, primary_filter, dicts, primary_ids, secondary_ids, secondary_refs, tertiary_ids, tertiary_refs);
        return;
      }
      if (tertiary_filter == 0) {
        // iterate over all statements with the given primary + secondary term.
        m_it = std::make_shared<TriplesFilteredByTwoPositionsIterator>(order, primary_filter, secondary_filter, dicts, primary_ids, secondary_ids, secondary_refs, tertiary_ids, tertiary_refs);
        return;
      }

      m_it = std::make_shared<SingleTripleIterator>(order, filter, dicts);
      return;
      // yield a single statement, if present.
    }

    auto inner_proceed() -> void override {
      m_it->proceed();
      m_has_next = m_it->has_next();
      if (m_has_next) {
        m_next = m_it->read();
      }
    }

  private:
    std::shared_ptr<dldi::Iterator<QuantifiedTriple>> m_it;
  };
}
#endif