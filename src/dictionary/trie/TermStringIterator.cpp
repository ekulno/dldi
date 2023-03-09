#include <cstddef>
#include <tuple>
#include <vector>

#include <dictionary/trie/TermStringIterator.hpp>

#include "TrieAlgorithm/TrieAlgorithm.hpp"

namespace csd {

  TermStringIterator::TermStringIterator(const DataManager* const data, const std::string& prefix)
    : m_data{data},
      m_termiterator{data, prefix} {
    m_has_next = m_termiterator.has_next();
    if (m_has_next) {
      const auto next_id{m_termiterator.read()};
      const auto* const leaf{m_data->get_leafNode(next_id)};
      m_next = std::pair<std::string, std::size_t>{TrieAlgorithm::id_to_string(m_data, next_id), leaf->occurences};
    }
  }
  auto TermStringIterator::inner_proceed() -> void {
    m_termiterator.proceed();
    m_has_next = m_termiterator.has_next();
    if (m_has_next) {
      const auto next_id{m_termiterator.read()};
      const auto* const leaf{m_data->get_leafNode(next_id)};
      m_next = std::pair<std::string, std::size_t>{TrieAlgorithm::id_to_string(m_data, next_id), leaf->occurences};
    }
  }
}
