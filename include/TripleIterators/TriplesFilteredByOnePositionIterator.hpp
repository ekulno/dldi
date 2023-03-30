#ifndef DLDI_TRIPLES_FILTERED_BY_ONE_POSITION_ITERATOR_HPP
#define DLDI_TRIPLES_FILTERED_BY_ONE_POSITION_ITERATOR_HPP

#include <Iterator.hpp>
#include <QuantifiedTriple.hpp>
#include <TripleIterators/binary_search.hpp>

namespace dldi {
  class TriplesFilteredByOnePositionIterator : public Iterator<QuantifiedTriple> {
  public:
    TriplesFilteredByOnePositionIterator(
      const dldi::TripleOrder& order,
      const std::size_t& primary_id,
      const std::shared_ptr<const dldi::DictionariesHandle> dicts,
      const std::pair<const std::size_t*, const std::size_t>& primary_ids,
      const std::pair<const std::size_t*, const std::size_t>& secondary_ids,
      const std::pair<const std::size_t*, const std::size_t>& secondary_refs,
      const std::pair<const std::size_t*, const std::size_t>& tertiary_ids,
      const std::pair<const std::size_t*, const std::size_t>& tertiary_refs)
      :

        m_secondary_ids{secondary_ids},
        m_secondary_refs{secondary_refs},
        m_tertiary_ids{tertiary_ids},
        m_tertiary_refs{tertiary_refs},
        m_order{order}

    {
      const auto [primary_position, secondary_position, tertiary_position] = EnumMapping::order_to_positions(order);

      m_primary_ids_index = BinarySearch::binary_search(
        std::get<0>(primary_ids),
        0,
        std::get<1>(primary_ids) - 1,
        primary_id,
        dicts->get_dict(primary_position));

      m_secondary_ids_index = std::get<0>(secondary_refs)[m_primary_ids_index];
      m_tertiary_ids_index = std::get<0>(tertiary_refs)[m_secondary_ids_index];

      m_next_primary = primary_id;
      m_next_secondary = std::get<0>(secondary_ids)[m_secondary_ids_index];
      m_next_tertiary = std::get<0>(tertiary_ids)[m_tertiary_ids_index];

      m_has_next = true;
      set_next();
    }
    auto set_next() -> void {
      // a bit awkward...
      // map back to QuantifiedTriple, which requires SPO order.
      if (m_order == dldi::TripleOrder::SPO) {
        m_next = QuantifiedTriple{m_next_primary, m_next_secondary, m_next_tertiary, 1};
      } else if (m_order == dldi::TripleOrder::SOP) {
        m_next = QuantifiedTriple{m_next_primary, m_next_tertiary, m_next_secondary, 1};
      } else if (m_order == dldi::TripleOrder::PSO) {
        m_next = QuantifiedTriple{m_next_secondary, m_next_primary, m_next_tertiary, 1};
      } else if (m_order == dldi::TripleOrder::POS) {
        m_next = QuantifiedTriple{m_next_tertiary, m_next_primary, m_next_secondary, 1};
      } else if (m_order == dldi::TripleOrder::OSP) {
        m_next = QuantifiedTriple{m_next_secondary, m_next_tertiary, m_next_primary, 1};
      } else {
        throw std::runtime_error("Unknown order");
      }
    }
    auto inner_proceed() -> void override {
      // we need to determine for each position whether we need to increment the ID index since the previous triple.
      // if the primary changes, the secondary necessarily changes.
      // if the secondary changes, the tertiary necessarily changes.

      // The tertiary always increments. This is because we don't store duplicate triples.
      m_next_tertiary = std::get<0>(m_tertiary_ids)[++m_tertiary_ids_index];

      // To detect whether the primary ID index should change, we check whether
      //  the next secondary's segment of the tertiary ids array includes
      //  the new value for m_tertiary_ids_index.
      if (std::get<0>(m_tertiary_refs)[m_secondary_ids_index] == m_tertiary_ids_index) {
        // yes, we need to advance the predicate ID array index.
        m_next_secondary = std::get<0>(m_secondary_ids)[++m_secondary_ids_index];

        // finally, the same story for the primary position.
        if (std::get<0>(m_secondary_refs)[m_primary_ids_index] == m_secondary_ids_index) {
          m_has_next = false;
          return;
        }
      }

      set_next();
      m_has_next = true;
    }

  private:
    std::pair<const std::size_t*, const std::size_t> m_secondary_ids;
    std::pair<const std::size_t*, const std::size_t> m_secondary_refs;
    std::pair<const std::size_t*, const std::size_t> m_tertiary_ids;
    std::pair<const std::size_t*, const std::size_t> m_tertiary_refs;

    std::size_t m_primary_ids_index{0};
    std::size_t m_secondary_ids_index{0};
    std::size_t m_tertiary_ids_index{0};

    std::size_t m_next_primary;
    std::size_t m_next_secondary;
    std::size_t m_next_tertiary;

    const TripleOrder m_order;
  };
}
#endif