#ifndef DLDI_ALL_TRIPLES_ITERATOR_HPP
#define DLDI_ALL_TRIPLES_ITERATOR_HPP

#include <Iterator.hpp>
#include <QuantifiedTriple.hpp>

namespace dldi {
  class AllTriplesIterator : public Iterator<QuantifiedTriple> {
  public:
    AllTriplesIterator(const dldi::TripleOrder& order,
                       const std::shared_ptr<const dldi::DictionariesHandle> dicts,
                       std::pair<const std::size_t*, const std::size_t> primary_ids,
                       std::pair<const std::size_t*, const std::size_t> secondary_ids,
                       std::pair<const std::size_t*, const std::size_t> secondary_refs,
                       std::pair<const std::size_t*, const std::size_t> tertiary_ids,
                       std::pair<const std::size_t*, const std::size_t> tertiary_refs)
      : m_primary_ids{primary_ids},
        m_secondary_ids{secondary_ids},
        m_secondary_refs{secondary_refs},
        m_tertiary_ids{tertiary_ids},
        m_tertiary_refs{tertiary_refs},
        m_primary_ids_index{0},
        m_secondary_ids_index{0},
        m_tertiary_ids_index{0} {
      m_next = QuantifiedTriple{
        std::get<0>(primary_ids)[m_primary_ids_index],
        std::get<0>(secondary_ids)[m_secondary_ids_index],
        std::get<0>(tertiary_ids)[m_tertiary_ids_index],
        1};
      m_has_next = true;
    }

    auto inner_proceed() -> void override {
      const auto [secondary_ids_ptr, secondary_ids_len]{m_secondary_ids};
      const auto [secondary_ref_ptr, secondary_ref_len]{m_secondary_refs};
      const auto [teriary_ids_ptr, tertiary_ids_len]{m_secondary_ids};
      const auto [teriary_ref_ptr, tertiary_ref_len]{m_secondary_refs};

      if (m_tertiary_ids_index == tertiary_ids_len) {
        m_has_next = false;
        return;
      }

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
          m_next_primary = std::get<0>(m_primary_ids)[++m_primary_ids_index];
        }
      }

      m_next = QuantifiedTriple{m_next_primary, m_next_secondary, m_next_tertiary, 1};
      m_has_next = true;
    }

  private:
    std::pair<const std::size_t*, const std::size_t> m_primary_ids;
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

    const TriplePattern m_pattern;
    const DictionariesHandle m_dicts;
  };
}
#endif